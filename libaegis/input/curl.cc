//
//	aegis - project change supervisor
//	Copyright (C) 2003-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/ctype.h>
#include <common/ac/curl/curl.h>
#include <common/ac/errno.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>
#include <common/ac/time.h>
#include <common/ac/unistd.h>

#include <common/error.h>
#include <common/format_elpsd.h>
#include <common/itab.h>
#include <common/mem.h>
#include <common/nstring.h>
#include <common/page.h>
#include <libaegis/input/curl.h>
#include <libaegis/option.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <libaegis/url.h>


#ifdef HAVE_LIBCURL

#define FATAL(function, reason) \
	fatal_raw("%s: %d: " function ": %s", __FILE__, __LINE__, reason);


//
// If there is more than one URL open at a time, all are processed
// in parallel.  The multi-handle aggregates them all.
//
static CURLM *multi_handle;
static bool call_multi_immediate;
static itab_ty *stp;


input_curl::~input_curl()
{
    //
    // Release libcurl resources.
    //
    curl_multi_remove_handle(multi_handle, handle);
    curl_easy_cleanup(handle);
    handle = 0;
    eof = true;

    if (progress_cleanup)
    {
	write(2, "\n", 1);
        progress_cleanup = 0;
    }

    //
    // Release dynamic memory resources.
    //
    delete [] curl_buffer;
    curl_buffer = 0;
    curl_buffer_position = 0;
    curl_buffer_length = 0;
    curl_buffer_maximum = 0;
}


static int
progress_callback(void *p, double dt, double dc, double, double)
{
    input_curl *icp = (input_curl *)p;
    icp->progress_callback(dt, dc);
    return 0;
}


//
// Libcurl calls this function when it receives more data.
//
static size_t
write_callback(char *data, size_t size, size_t nitems, void *p)
{
    input_curl *icp = (input_curl *)p;
    size_t nbytes = size * nitems;
    return icp->write_callback(data, nbytes);
}


input_curl::input_curl(const nstring &arg) :
    fn(arg),
    pos(0),
    curl_buffer(0),
    curl_buffer_maximum(0),
    curl_buffer_position(0),
    curl_buffer_length(0),
    eof(false)
{
    handle = curl_easy_init();
    if (!handle)
	nfatal("curl_easy_init");

    CURLcode err = curl_easy_setopt(handle, CURLOPT_ERRORBUFFER, errbuf);
    if (err)
	FATAL("curl_easy_setopt", curl_easy_strerror(err));

#if (LIBCURL_VERSION_NUM < 0x070b01)
    //
    // libcurl prior to 7.11.1 has problems handling autenticated
    // proxy specified by http_proxy or HTTP_PROXY, so we set them
    // manually.
    //

    int uid;
    int gid;
    int umask;
    //
    // We need to save the user identity because the url::split method
    // call os_become_ itself and we must issue os_become_undo and
    // os_become to not raise a multiple permission error.
    //
    os_become_query(&uid, &gid, &umask);
    os_become_undo();
    url target_url(fn);
    os_become(uid, gid, umask);
    if (target_url.get_protocol() == "http")
    {
        char *http_proxy = getenv("http_proxy");
        if (!http_proxy || http_proxy[0] == '\0')
            http_proxy = getenv("HTTP_PROXY");
        if (http_proxy && http_proxy[0] != '\0')
        {
            //
            // We use the user's identity previously saved to
            // undo/restore the process identity in order to prevent a
            // multiple permission error from url::split.
            //
            os_become_undo();
            url proxy_url(http_proxy);
            os_become(uid, gid, umask);
            userpass = proxy_url.get_userpass();
            proxy = proxy_url.reassemble(true);
            if (!userpass.empty())
            {
                curl_easy_setopt
                (
                    handle,
                    CURLOPT_PROXYUSERPWD,
                    userpass.c_str()
                );
            }
            curl_easy_setopt(handle, CURLOPT_PROXY, proxy.c_str());
        }
    }
#endif
    err = curl_easy_setopt(handle, CURLOPT_URL, fn.c_str());
    if (err)
	FATAL("curl_easy_setopt", curl_easy_strerror(err));
    err = curl_easy_setopt(handle, CURLOPT_FILE, this);
    if (err)
	FATAL("curl_easy_setopt", curl_easy_strerror(err));
    err = curl_easy_setopt(handle, CURLOPT_VERBOSE, 0);
    if (err)
	FATAL("curl_easy_setopt", curl_easy_strerror(err));
    err = curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, ::write_callback);
    if (err)
	FATAL("curl_easy_setopt", curl_easy_strerror(err));
    err = curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1);
    if (err)
	FATAL("curl_easy_setopt", curl_easy_strerror(err));

    progress_start = 0;
    progress_buflen = 0;
    progress_buffer = 0;
    progress_cleanup = 0;
    if (option_verbose_get())
    {
	err = curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0);
	if (err)
	    FATAL("curl_easy_setopt", curl_easy_strerror(err));
	err =
	    curl_easy_setopt
	    (
		handle,
		CURLOPT_PROGRESSFUNCTION,
		::progress_callback
	    );
	if (err)
	    FATAL("curl_easy_setopt", curl_easy_strerror(err));
	err = curl_easy_setopt(handle, CURLOPT_PROGRESSDATA, this);
	if (err)
	    FATAL("curl_easy_setopt", curl_easy_strerror(err));
	time(&progress_start);
	progress_buflen = page_width_get(80);
	if (progress_buflen < 40)
	    progress_buflen = 40;
	progress_buffer = new char [progress_buflen];
    }

    if (!multi_handle)
    {
	multi_handle = curl_multi_init();
	if (!multi_handle)
	    nfatal("curl_multi_init");
    }
    CURLMcode merr = curl_multi_add_handle(multi_handle, handle);
    switch (merr)
    {
    case CURLM_CALL_MULTI_PERFORM:
	call_multi_immediate = true;
	break;

    case CURLM_OK:
	break;

    default:
	FATAL("curl_multi_add_handle", curl_multi_strerror(merr));
    }

    //
    // Start the fetch as soon as possible.
    //
    call_multi_immediate = true;

    //
    // Build an associate table from libcurl handles to our file pointers.
    //
    if (!stp)
	stp = itab_alloc();
    itab_assign(stp, (itab_key_ty)handle, (void *)this);
}


static void
print_byte_count(char *buf, size_t len, double number)
{
    if (number < 0)
    {
	snprintf(buf, len, "-----");
	return;
    }
    // K is Kelvin, k is kilo
    const char *units = " kMGTPEZY";
    for (;;)
    {
	if (*units != ' ')
	{
	    if (number < 10)
	    {
		snprintf(buf, len, "%4.2f%cB", number, *units);
		return;
	    }
	    if (number < 100)
	    {
		snprintf(buf, len, "%4.1f%cB", number, *units);
		return;
	    }
	}
	if (number < (1<<10))
	{
	    snprintf(buf, len, "%4d%cB", (int)number, *units);
	    return;
	}
	number /= 1024.;
	++units;
    }
}


void
input_curl::progress_callback(double down_total, double down_current)
{
    if (down_current <= 0 || down_total <= 0)
	return;
    if (down_current >= down_total && !progress_cleanup)
        return;
    time_t curtim;
    time(&curtim);
    curtim -= progress_start;
    char buf1[7];
    print_byte_count(buf1, sizeof(buf1), (long)down_current);
    char buf2[7];
    print_byte_count(buf2, sizeof(buf2), (long)down_total);
    double frac = (down_total <= 0) ? 0 : (down_current / down_total);
    time_t predict = (time_t)(frac ? (0.5 + curtim / frac) : 0);
    time_t remaining = predict - curtim;
    char buf3[7];
    format_elapsed(buf3, sizeof(buf3), remaining);

    memset(progress_buffer, ' ', progress_buflen);
    memcpy(progress_buffer +  0, buf1, 6);
    memcpy(progress_buffer +  6, " of ", 4);
    memcpy(progress_buffer + 10, buf2, 6);
    snprintf(progress_buffer + 17, 5, "%3d%%", (int)(100 * frac + 0.5));

    int lhs = 23;
    int rhs = (int)(lhs + (progress_buflen - 37) * frac);
    while (lhs < rhs)
	progress_buffer[lhs++] = '=';
    progress_buffer[lhs] = '>';

    memcpy(progress_buffer + progress_buflen - 11, "ETA", 3);
    memcpy(progress_buffer + progress_buflen - 7, buf3, 6);
    progress_buffer[progress_buflen - 1] = '\r';
    write(2, progress_buffer, progress_buflen);
    progress_cleanup = 1;

    if (down_current >= down_total)
    {
        write(2, "\n", 1);
        progress_cleanup = 0;
    }
}


size_t
input_curl::write_callback(char *data, size_t nbytes)
{
    //
    // Grow the buffer if necessary.
    //
    // Always keep it a power of 2, because sigma(2**-n)==1, so we get
    // O(1) behaviour.  (That +32 means we are always just 32 bytes
    // short of a power of 2, leaving room for the malloc header, which
    // results in a nicer malloc fit on many systems.
    //
    if (curl_buffer_length + nbytes > curl_buffer_maximum)
    {
	for (;;)
	{
	    curl_buffer_maximum = curl_buffer_maximum * 2 + 32;
	    if (curl_buffer_length + nbytes <= curl_buffer_maximum)
		break;
	}
	char *new_curl_buffer = new char [curl_buffer_maximum];
	memcpy(new_curl_buffer, curl_buffer, curl_buffer_length);
	delete [] curl_buffer;
	curl_buffer = new_curl_buffer;
    }

    //
    // Copy the data into the buffer.
    //
    memcpy(curl_buffer + curl_buffer_length, data, nbytes);
    curl_buffer_length += nbytes;

    //
    // A negative return will stop the transfer for this stream.
    //
    return nbytes;
}


static input_curl *
handle_to_fp(CURL *handle)
{
    assert(stp);
    input_curl *result = (input_curl *)itab_query(stp, (itab_key_ty)handle);
    if (!result || !result->verify_handle(handle))
    {
	fatal_raw
	(
	    "%s: %d: handle %p gave file %p",
	    __FILE__,
	    __LINE__,
	    (void *)handle,
	    (void *)result
	);
    }
    return result;
}


/**
  * The perform function is a wrapper around the curl_multi_perform
  * function.  It checks for messages that may be waiting, waits in
  * select if necessary, and calls curl_multi_perform eventually.
  *
  * It is expected that this function will be repeatedly called from a
  * tight loop, so it doesn't loop itself.
  */

static void
perform(void)
{
    //
    // See if there are any messages waiting.
    // These tell us about errors, and completed transfers.
    //
    for (;;)
    {
	int msgs = 0;
	CURLMsg *msg = curl_multi_info_read(multi_handle, &msgs);
	if (!msg)
	    break;
	if (msg->msg == CURLMSG_NONE)
	    break;
	if (msg->msg != CURLMSG_DONE)
	    fatal_raw("curl_multi_info_read -> %d (bug)", msg->msg);
	input_curl *fp = handle_to_fp(msg->easy_handle);
	if (msg->data.result == 0)
	{
	    // transfer over, no error
	    fp->eof_notify();
	}
	else
	{
	    fp->read_error();
	}
    }

    //
    // Look for more to happen.
    //
    if (call_multi_immediate)
    {
	call_multi_immediate = false;
	for (;;)
	{
	    int num_xfer = 0;
	    CURLMcode ret = curl_multi_perform(multi_handle, &num_xfer);
	    switch (ret)
	    {
	    case CURLM_CALL_MULTI_PERFORM:
		call_multi_immediate = true;
		return;

	    case CURLM_OK:
		return;

	    default:
		error_raw
		(
		    "%s: %d: curl_multi_perform: %s",
		    __FILE__,
		    __LINE__,
		    curl_multi_strerror(ret)
		);
	    }
	}
    }
    else
    {
	fd_set fdread;
	FD_ZERO(&fdread);
	fd_set fdwrite;
	FD_ZERO(&fdwrite);
	fd_set fdexcep;
	FD_ZERO(&fdexcep);

	// get file descriptors from the transfers
	int maxfd = 0;
	CURLcode err =
	    (CURLcode)
	    curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);
	if (err)
	    FATAL("curl_multi_fdset", curl_easy_strerror(err));

	if (maxfd >= 0)
	{
	    // set a suitable timeout to fail on
	    struct timeval timeout;
	    timeout.tv_sec = 60; // 1 minute
	    timeout.tv_usec = 0;

	    int rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
	    if (rc < 0)
	    {
		if (errno != EINTR)
		{
		    nfatal
		    (
			"%s: %d: select: %s",
			__FILE__,
			__LINE__,
			strerror(errno)
		    );
		    // NOTREACHED
		}
	    }
	    if (rc > 0)
	    {
		//
		// Some sockets are ready.
		//
		call_multi_immediate = true;
	    }
        }
    }
}


void
input_curl::read_error()
{
    sub_context_ty sc;
    sc.var_set_string("File_Name", fn);
    sc.var_set_charstar("ERRNO", errbuf);
    sc.var_override("ERRNO");
    sc.fatal_intl(i18n("read $filename: $errno"));
    // NOTREACHED
}


/**
  * The read_data function is used to read data into the data buffer provided.
  * Returns the number of bytes read.
  */

long
input_curl::read_data(void *data, size_t nbytes)
{
    //
    // attempt to fill buffer
    //
    while (!eof && curl_buffer_position + nbytes > curl_buffer_length)
	perform();

    //
    // Extract as much data as possible from the buffer.
    //
    size_t size_of_buffer = curl_buffer_length - curl_buffer_position;
    if (nbytes > size_of_buffer)
	nbytes = size_of_buffer;
    memcpy(data, curl_buffer + curl_buffer_position, nbytes);
    curl_buffer_position += nbytes;

    //
    // Rearrange the buffer so that it does not grow forever.
    //
    size_of_buffer = curl_buffer_length - curl_buffer_position;
    if (size_of_buffer == 0)
    {
	curl_buffer_position = 0;
	curl_buffer_length = 0;
    }
    else if (size_of_buffer <= curl_buffer_position)
    {
	// can shuffle the data down easily
	memcpy(curl_buffer, curl_buffer + curl_buffer_position, size_of_buffer);
	curl_buffer_position = 0;
	curl_buffer_length = size_of_buffer;
    }

    if (nbytes == 0 && progress_cleanup)
    {
	write(2, "\n", 1);
        progress_cleanup = 0;
    }

    //
    // Return the number of bytes read.
    //
    return nbytes;
}


long
input_curl::read_inner(void *data, size_t len)
{
    os_become_must_be_active();

    long result = read_data(data, len);
    assert(result >= 0);

    pos += result;
    return result;
}


long
input_curl::ftell_inner()
{
    return pos;
}


nstring
input_curl::name()
{
    return fn;
}


long
input_curl::length()
{
    // Maybe there was a Content-Length header?
    return -1;
}


#else


input_curl::~input_curl()
{
}


input_curl::input_curl(const nstring &arg) :
    fn(arg)
{
    sub_context_ty sc;
    sc.var_set_string("FileLine", fn);
    sc.fatal_intl(i18n("open $filename: no curl library"));
}


long
input_curl::read_inner(void *, size_t)
{
    return 0;
}


long
input_curl::ftell_inner()
{
    return 0;
}


nstring
input_curl::name()
{
    return fn;
}


long
input_curl::length()
{
    return -1;
}

#endif // HAVE_LIBCURL


bool
input_curl::looks_likely(const nstring &file_name)
{
    const char *cp = file_name.c_str();
    if (!isalpha((unsigned char)*cp))
	return 0;
    for (;;)
    {
	++cp;
	if (!isalpha((unsigned char)*cp))
	    break;
    }
    return (cp[0] == ':' && cp[1] != '\0');
}


bool
input_curl::is_remote()
    const
{
    return true;
}
