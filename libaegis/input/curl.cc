//
//	aegis - project change supervisor
//	Copyright (C) 2003-2005 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate curls
//

#include <ac/ctype.h>
#include <ac/curl/curl.h>
#include <ac/errno.h>
#include <ac/stdlib.h>
#include <ac/string.h>
#include <ac/time.h>
#include <ac/unistd.h>

#include <error.h>
#include <input/curl.h>
#include <input/private.h>
#include <mem.h>
#include <nstring.h>
#include <option.h>
#include <os.h>
#include <page.h>
#include <sub.h>
#include <itab.h>
#include <url.h>

#ifdef HAVE_LIBCURL

#define FATAL(function, reason) \
	fatal_raw("%s: %d: " function ": %s", __FILE__, __LINE__, reason);


struct input_curl_ty
{
    input_ty	    inherited;
    CURL            *handle;
    string_ty	    *fn;
    long	    pos;

    char            *buffer;        // buffer to store cached data
    size_t          buffer_maximum; // currently allocated buffers length
    size_t          buffer_postion; // start of data in buffer
    size_t          buffer_length;  // end of data in buffer
    int             eof;            // end of file has been reached

    char            errbuf[CURL_ERROR_SIZE];

    time_t          progress_start;
    char            *progress_buffer;
    int             progress_buflen;
    int             progress_cleanup;
#if (LIBCURL_VERSION_NUM < 0x070b01)
    nstring         *proxy;
    nstring         *userpass;
#endif
};


//
// If there is more than one URL open at a time, all are processed
// in parallel.  The multi-handle aggregates them all.
//
static CURLM    *multi_handle;
static int      call_multi_immediate;
static itab_ty  *stp;



static void
print_byte_count(char *buf, size_t len, double number)
{
    const char      *units;

    if (number < 0)
    {
	snprintf(buf, len, "-----");
	return;
    }
    units = " KMGTPE";
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


static void
print_seconds(char *buf, size_t len, time_t secs)
{
    time_t          mins;
    time_t          hours;
    time_t          days;

    mins = secs / 60;
    secs %= 60;
    hours = mins / 60;
    mins %= 60;
    if (hours == 0)
    {
	snprintf(buf, len, "%2dm%2.2ds", (int)mins, (int)secs);
	return;
    }
    days = hours / 24;
    hours %= 24;
    if (days == 0)
    {
	snprintf(buf, len, "%2dh%2.2dm", (int)hours, (int)mins);
	return;
    }
    if (days < 100)
    {
	snprintf(buf, len, "%2dd%2.2dh", (int)days, (int)hours);
	return;
    }
    snprintf(buf, len, "%5.2fy", days / 365.25);
}


static int
progress(void *p, double down_total, double down_current, double up_total,
    double up_current)
{
    input_curl_ty   *fp;
    char            buf1[7];
    char            buf2[7];
    char            buf3[7];
    time_t          curtim;
    time_t          predict;
    time_t          remaining;
    double          frac;
    int             lhs;
    int             rhs;

    if (down_current <= 0 || down_total <= 0)
	return 0;
    fp = (input_curl_ty *)p;
    time(&curtim);
    curtim -= fp->progress_start;
    print_byte_count(buf1, sizeof(buf1), (long)down_current);
    print_byte_count(buf2, sizeof(buf2), (long)down_total);
    frac = (down_total <= 0) ? 0 : (down_current / down_total);
    predict = (time_t)(frac ? (0.5 + curtim / frac) : 0);
    remaining = predict - curtim;
    print_seconds(buf3, sizeof(buf3), remaining);

    memset(fp->progress_buffer, ' ', fp->progress_buflen);
    fp->progress_buffer[0] = '\r';
    memcpy(fp->progress_buffer +  1, buf1, 6);
    memcpy(fp->progress_buffer +  7, " of ", 4);
    memcpy(fp->progress_buffer + 11, buf2, 6);
    snprintf(fp->progress_buffer + 18, 5, "%3d%%", (int)(100 * frac + 0.5));

    lhs = 24;
    rhs = (int)(lhs + (fp->progress_buflen - 36) * frac);
    while (lhs < rhs)
	fp->progress_buffer[lhs++] = '=';
    fp->progress_buffer[lhs] = '>';

    memcpy(fp->progress_buffer + fp->progress_buflen - 10, "ETA", 3);
    memcpy(fp->progress_buffer + fp->progress_buflen - 6, buf3, 6);
    write(2, fp->progress_buffer, fp->progress_buflen);
    fp->progress_cleanup = 1;
    return 0;
}


//
// Libcurl calls this function when it receives more data.
//
static size_t
callback(char *data, size_t size, size_t nitems, void *p)
{
    input_curl_ty   *fp;
    size_t          nbytes;

    fp = (input_curl_ty *)p;
    nbytes = size * nitems;

    //
    // Grow the buffer if necessary.
    //
    // Always keep it a power of 2, because sigma(2**-n)==1, so we get
    // O(1) behaviour.  (That +32 means we are always just 32 bytes
    // short of a power of 2, leaving room for the malloc header, which
    // results in a nicer malloc fit on many systems.
    //
    while (fp->buffer_length + nbytes > fp->buffer_maximum)
    {
	fp->buffer_maximum = fp->buffer_maximum * 2 + 32;
	fp->buffer = (char *)mem_change_size(fp->buffer, fp->buffer_maximum);
    }

    //
    // Copy the data into the buffer.
    //
    memcpy(fp->buffer + fp->buffer_length, data, nbytes);
    fp->buffer_length += nbytes;

    //
    // A negative return will stop the transfer for this stream.
    //
    return nbytes;
}


static void
destruct(input_ty *p)
{
    input_curl_ty   *fp;

    //
    // Release libcurl resources.
    //
    fp = (input_curl_ty *)p;
    curl_multi_remove_handle(multi_handle, fp->handle);
    curl_easy_cleanup(fp->handle);
    fp->handle = 0;
    fp->eof = 1;

    if (fp->progress_cleanup)
	write(2, "\n", 1);

    //
    // Release dynamic memory resources.
    //
    if (fp->buffer)
        mem_free(fp->buffer);
    fp->buffer = 0;
    fp->buffer_postion = 0;
    fp->buffer_length = 0;
    fp->buffer_maximum = 0;
    str_free(fp->fn);
    fp->fn = 0;
#if (LIBCURL_VERSION_NUM < 0x070b01)
    delete fp->userpass;
    delete fp->proxy;
#endif
}


static input_curl_ty *
handle_to_fp(CURL *handle)
{
    input_curl_ty   *result;

    assert(stp);
    result = (input_curl_ty *)itab_query(stp, (itab_key_ty)handle);
    if (!result || result->handle != handle)
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
  * The perform function is a wrapper arounf the curl_multi_perform
  * function.  It checks form messages that may be waiting, waits in
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
	CURLMsg         *msg;
	int             msgs;
	input_curl_ty   *fp;

	msg = curl_multi_info_read(multi_handle, &msgs);
	if (!msg)
	    break;
	if (msg->msg == CURLMSG_NONE)
	    break;
	if (msg->msg != CURLMSG_DONE)
	    fatal_raw("curl_multi_info_read -> %d (bug)", msg->msg);
	fp = handle_to_fp(msg->easy_handle);
	if (msg->data.result == 0)
	{
	    // transfer over, no error
	    fp->eof = 1;
	}
	else
	{
	    sub_context_ty	*scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", fp->fn);
	    sub_var_set_charstar(scp, "ERRNO", fp->errbuf);
	    sub_var_override(scp, "ERRNO");
	    fatal_intl(scp, i18n("read $filename: $errno"));
	    // NOTREACHED
	}
    }

    //
    // Look for more to happen.
    //
    if (call_multi_immediate)
    {
	call_multi_immediate = 0;
	for (;;)
	{
	    CURLMcode       ret;
	    int             num_xfer;

	    ret = curl_multi_perform(multi_handle, &num_xfer);
	    switch (ret)
	    {
	    case CURLM_CALL_MULTI_PERFORM:
		call_multi_immediate = 1;
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
	CURLcode        err;
	fd_set          fdread;
	fd_set          fdwrite;
	fd_set          fdexcep;
	int             maxfd;

	FD_ZERO(&fdread);
	FD_ZERO(&fdwrite);
	FD_ZERO(&fdexcep);

	// get file descriptors from the transfers
	err =
	    (CURLcode)
	    curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);
	if (err)
	    FATAL("curl_multi_fdset", curl_easy_strerror(err));

	if (maxfd >= 0)
	{
	    struct timeval  timeout;
	    int             rc;

	    // set a suitable timeout to fail on
	    timeout.tv_sec = 60; // 1 minute
	    timeout.tv_usec = 0;

	rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
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
	    call_multi_immediate = 1;
	}
        }
    }
}


/**
  * The read_data function is used to read data into the data buffer provided.
  * Returns the number of bytes read.
  */

static long
read_data(input_curl_ty *fp, void *data, size_t nbytes)
{
    size_t          size_of_buffer;

    //
    // attempt to fill buffer
    //
    while (!fp->eof && fp->buffer_postion + nbytes > fp->buffer_length)
	perform();

    //
    // Extract as much data as possible from the buffer.
    //
    size_of_buffer = fp->buffer_length - fp->buffer_postion;
    if (nbytes > size_of_buffer)
	nbytes = size_of_buffer;
    memcpy(data, fp->buffer + fp->buffer_postion, nbytes);
    fp->buffer_postion += nbytes;

    //
    // Rearrange the buffer so that it does not grow forever.
    //
    size_of_buffer = fp->buffer_length - fp->buffer_postion;
    if (size_of_buffer == 0)
    {
	fp->buffer_postion = 0;
	fp->buffer_length = 0;
    }
    else if (size_of_buffer <= fp->buffer_postion)
    {
	// can shuffle the data down easily
	memcpy(fp->buffer, fp->buffer + fp->buffer_postion, size_of_buffer);
	fp->buffer_postion = 0;
	fp->buffer_length = size_of_buffer;
    }

    //
    // Return the number of bytes read.
    //
    return nbytes;
}


static long
input_curl_read(input_ty *p, void *data, size_t len)
{
    input_curl_ty   *fp;
    long	    result;

    os_become_must_be_active();
    if (len < 0)
	return 0;
    fp = (input_curl_ty *)p;

    result = read_data(fp, data, len);
    assert(result >= 0);

    fp->pos += result;
    return result;
}


static long
input_curl_ftell(input_ty *p)
{
    input_curl_ty   *fp;

    fp = (input_curl_ty *)p;
    return fp->pos;
}


static string_ty *
input_curl_name(input_ty *p)
{
    input_curl_ty   *fp;

    fp = (input_curl_ty *)p;
    return fp->fn;
}


static long
input_curl_length(input_ty *p)
{
    // Maybe there was a Content-Length header?
    return -1;
}


static input_vtbl_ty vtbl =
{
    sizeof(input_curl_ty),
    destruct,
    input_curl_read,
    input_curl_ftell,
    input_curl_name,
    input_curl_length,
    0, // keepalive
};


input_ty *
input_curl_open(string_ty *fn)
{
    CURLcode        err;
    CURLMcode       merr;
    input_ty	    *result;
    input_curl_ty   *fp;

    result = input_new(&vtbl);
    fp = (input_curl_ty *)result;
    fp->fn = str_copy(fn);
    fp->pos = 0;
    fp->eof = 0;
    fp->buffer = 0;
    fp->buffer_postion = 0;
    fp->buffer_length = 0;
    fp->buffer_maximum = 0;

    fp->handle = curl_easy_init();
    if (!fp->handle)
	nfatal("curl_easy_init");

    err = curl_easy_setopt(fp->handle, CURLOPT_ERRORBUFFER, fp->errbuf);
    if (err)
	FATAL("curl_easy_setopt", curl_easy_strerror(err));

#if (LIBCURL_VERSION_NUM < 0x070b01)
    //
    // libcurl prior to 7.11.1 has problems handling autenticated
    // proxy specified by http_proxy or HTTP_PROXY, so we set them
    // manually.
    //

    int         uid;
    int         gid;
    int         umask;
    //
    // We need to save the user identity because the url::split method
    // call os_become_ itself and we must issue os_become_undo and
    // os_become to not raise a multiple permission error.
    //
    os_become_query(&uid, &gid, &umask);
    os_become_undo();
    url target_url(fn->str_text);
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
            fp->userpass = new nstring(proxy_url.get_userpass());
            fp->proxy = new nstring(proxy_url.reassemble(true));
            if (!fp->userpass->empty())
            {
                curl_easy_setopt
                (
                    fp->handle,
                    CURLOPT_PROXYUSERPWD,
                    fp->userpass->c_str()
                );
            }
            curl_easy_setopt(fp->handle, CURLOPT_PROXY, fp->proxy->c_str());
        }
    }
#endif
    err = curl_easy_setopt(fp->handle, CURLOPT_URL, fp->fn->str_text);
    if (err)
	FATAL("curl_easy_setopt", curl_easy_strerror(err));
    err = curl_easy_setopt(fp->handle, CURLOPT_FILE, fp);
    if (err)
	FATAL("curl_easy_setopt", curl_easy_strerror(err));
    err = curl_easy_setopt(fp->handle, CURLOPT_VERBOSE, 0);
    if (err)
	FATAL("curl_easy_setopt", curl_easy_strerror(err));
    err = curl_easy_setopt(fp->handle, CURLOPT_WRITEFUNCTION, callback);
    if (err)
	FATAL("curl_easy_setopt", curl_easy_strerror(err));
    err = curl_easy_setopt(fp->handle, CURLOPT_FOLLOWLOCATION, 1);
    if (err)
	FATAL("curl_easy_setopt", curl_easy_strerror(err));

    fp->progress_start = 0;
    fp->progress_buflen = 0;
    fp->progress_buffer = 0;
    fp->progress_cleanup = 0;
    if (option_verbose_get())
    {
	err = curl_easy_setopt(fp->handle, CURLOPT_NOPROGRESS, 0);
	if (err)
	    FATAL("curl_easy_setopt", curl_easy_strerror(err));
	err = curl_easy_setopt(fp->handle, CURLOPT_PROGRESSFUNCTION, progress);
	if (err)
	    FATAL("curl_easy_setopt", curl_easy_strerror(err));
	err = curl_easy_setopt(fp->handle, CURLOPT_PROGRESSDATA, fp);
	if (err)
	    FATAL("curl_easy_setopt", curl_easy_strerror(err));
	time(&fp->progress_start);
	fp->progress_buflen = page_width_get(80);
	if (fp->progress_buflen < 40)
	    fp->progress_buflen = 40;
	fp->progress_buffer = (char *)mem_alloc(fp->progress_buflen);
    }

    if (!multi_handle)
    {
	multi_handle = curl_multi_init();
	if (!multi_handle)
	    nfatal("curl_multi_init");
    }
    merr = curl_multi_add_handle(multi_handle, fp->handle);
    switch (merr)
    {
    case CURLM_CALL_MULTI_PERFORM:
	call_multi_immediate = 1;
	break;

    case CURLM_OK:
	break;

    default:
	FATAL("curl_multi_add_handle", curl_multi_strerror(merr));
    }

    //
    // Start the fetch as soon as possible.
    //
    call_multi_immediate = 1;

    //
    // Build an associate table from libcurl handles to our file pointers.
    //
    if (!stp)
	stp = itab_alloc(1);
    itab_assign(stp, (itab_key_ty)fp->handle, (void *)fp);

    //
    // Report success.
    //
    return result;
}

#endif // HAVE_LIBCURL


bool
input_curl_looks_likely(string_ty *fn)
{
    const char *cp = fn->str_text;
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
