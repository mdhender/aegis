/*
 *	aegis - project change supervisor
 *	Copyright (C) 2003, 2004 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate curls
 */

#include <ac/ctype.h>
#include <ac/curl/curl.h>
#include <ac/errno.h>
#include <ac/string.h>
#include <ac/time.h>
#include <ac/unistd.h>

#include <error.h>
#include <input/curl.h>
#include <input/private.h>
#include <mem.h>
#include <option.h>
#include <os.h>
#include <page.h>
#include <sub.h>
#include <itab.h>

#ifdef HAVE_LIBCURL

/*
 * Requires libcurl 7.9.7 or later.
 */
#if (LIBCURL_VERSION_NUM < 0x070907)
#error "your version of libcurl is too old"
#endif


#define FATAL(function, reason) \
	fatal_raw("%s: %d: " function ": %s", __FILE__, __LINE__, reason);


typedef struct input_curl_ty input_curl_ty;
struct input_curl_ty
{
    input_ty	    inherited;
    CURL            *handle;
    string_ty	    *fn;
    long	    pos;

    char            *buffer;        /* buffer to store cached data */
    size_t          buffer_maximum; /* currently allocated buffers length */
    size_t          buffer_postion; /* start of data in buffer */
    size_t          buffer_length;  /* end of data in buffer */
    int             eof;            /* end of file has been reached */

    char            errbuf[CURL_ERROR_SIZE];

    time_t          progress_start;
    char            *progress_buffer;
    int             progress_buflen;
    int             progress_cleanup;
};


/*
 * If there is more than one URL open at a time, all are processed
 * in parallel.  The multi-handle aggregates them all.
 */
static CURLM    *multi_handle;
static int      call_multi_immediate;
static itab_ty  *stp;


static const char *
CURLcode_strerror(CURLcode x)
{
    switch (x)
    {
    case CURLE_OK:
	return "no error";

    case CURLE_UNSUPPORTED_PROTOCOL:
	return "unsupported protocol";

    case CURLE_FAILED_INIT:
	return "failed init";

    case CURLE_URL_MALFORMAT:
	return "url malformat";

    case CURLE_URL_MALFORMAT_USER:
	return "url malformat user";

    case CURLE_COULDNT_RESOLVE_PROXY:
	return "couldnt resolve proxy";

    case CURLE_COULDNT_RESOLVE_HOST:
	return "couldnt resolve host";

    case CURLE_COULDNT_CONNECT:
	return "couldn't connect";

    case CURLE_FTP_WEIRD_SERVER_REPLY:
	return "ftp weird server reply";

    case CURLE_FTP_ACCESS_DENIED:
	return "ftp access denied";

    case CURLE_FTP_USER_PASSWORD_INCORRECT:
	return "ftp user password incorrect";

    case CURLE_FTP_WEIRD_PASS_REPLY:
	return "ftp weird pass reply";

    case CURLE_FTP_WEIRD_USER_REPLY:
	return "ftp weird user reply";

    case CURLE_FTP_WEIRD_PASV_REPLY:
	return "ftp weird pasv reply";

    case CURLE_FTP_WEIRD_227_FORMAT:
	return "ftp weird 227 format";

    case CURLE_FTP_CANT_GET_HOST:
	return "ftp cant get host";

    case CURLE_FTP_CANT_RECONNECT:
	return "ftp can't reconnect";

    case CURLE_FTP_COULDNT_SET_BINARY:
	return "ftp couldn't set binary";

    case CURLE_PARTIAL_FILE:
	return "partial file";

    case CURLE_FTP_COULDNT_RETR_FILE:
	return "ftp couldn't retr file";

    case CURLE_FTP_WRITE_ERROR:
	return "ftp write error";

    case CURLE_FTP_QUOTE_ERROR:
	return "ftp quote error";

    case CURLE_HTTP_NOT_FOUND:
	return "http not found";

    case CURLE_WRITE_ERROR:
	return "write error";

    case CURLE_MALFORMAT_USER:
	return "user name is illegally specified";

    case CURLE_FTP_COULDNT_STOR_FILE:
	return "failed FTP upload";

    case CURLE_READ_ERROR:
	return "could open/read from file";

    case CURLE_OUT_OF_MEMORY:
	return "out of memory";

    case CURLE_OPERATION_TIMEOUTED:
	return "the timeout time was reached";

    case CURLE_FTP_COULDNT_SET_ASCII:
	return "TYPE A failed";

    case CURLE_FTP_PORT_FAILED:
	return "FTP PORT operation failed";

    case CURLE_FTP_COULDNT_USE_REST:
	return "the REST command failed";

    case CURLE_FTP_COULDNT_GET_SIZE:
	return "the SIZE command failed";

    case CURLE_HTTP_RANGE_ERROR:
	return "RANGE \"command\" didn't work";

    case CURLE_HTTP_POST_ERROR:
	return "http post error";

    case CURLE_SSL_CONNECT_ERROR:
	return "wrong when connecting with SSL";

    case CURLE_FTP_BAD_DOWNLOAD_RESUME:
	return "couldn't resume download";

    case CURLE_FILE_COULDNT_READ_FILE:
	return "file couldn't read file";

    case CURLE_LDAP_CANNOT_BIND:
	return "ldap cannot bind";

    case CURLE_LDAP_SEARCH_FAILED:
	return "ldap search failed";

    case CURLE_LIBRARY_NOT_FOUND:
	return "library not found";

    case CURLE_FUNCTION_NOT_FOUND:
	return "function not found";

    case CURLE_ABORTED_BY_CALLBACK:
	return "aborted by callback";

    case CURLE_BAD_FUNCTION_ARGUMENT:
	return "bad function argument";

    case CURLE_BAD_CALLING_ORDER:
	return "bad calling order";

    case CURLE_HTTP_PORT_FAILED:
	return "HTTP Interface operation failed";

    case CURLE_BAD_PASSWORD_ENTERED:
	return "my getpass() returns fail";

    case CURLE_TOO_MANY_REDIRECTS :
	return "catch endless re-direct loops";

    case CURLE_UNKNOWN_TELNET_OPTION:
	return "User specified an unknown option";

    case CURLE_TELNET_OPTION_SYNTAX :
	return "Malformed telnet option";

    case CURLE_OBSOLETE:
	return "obsolete";

    case CURLE_SSL_PEER_CERTIFICATE:
	return "peer's certificate wasn't ok";

    case CURLE_GOT_NOTHING:
	return "when this is a specific error";

    case CURLE_SSL_ENGINE_NOTFOUND:
	return "SSL crypto engine not found";

    case CURLE_SSL_ENGINE_SETFAILED:
	return "can not set SSL crypto engine as default";

    case CURLE_SEND_ERROR:
	return "failed sending network data";

    case CURLE_RECV_ERROR:
	return "failure in receiving network data";

#if (LIBCURL_VERSION_NUM >= 0x070A04)
    case CURLE_SHARE_IN_USE:
	return "CURLE_SHARE_IN_USER";

    case CURLE_SSL_CERTPROBLEM:
	return "problem with the local certificate";

    case CURLE_SSL_CIPHER:
	return "couldn't use specified cipher";

    case CURLE_SSL_CACERT:
	return "problem with the CA cert (path? access rights?)";

    case CURLE_BAD_CONTENT_ENCODING:
	return "Unrecognized transfer encoding";
#endif

    case CURL_LAST:
	assert(0);
	break;
    }
    return "unknown CURLcode";
}


static const char *
CURLMcode_strerror(CURLMcode x)
{
    switch (x)
    {
    case CURLM_CALL_MULTI_PERFORM:
	return "please call curl_multi_perform() soon";

    case CURLM_OK:
	return "no error";

    case CURLM_BAD_HANDLE:
	return "CURLM not valid multi handle";

    case CURLM_BAD_EASY_HANDLE:
	return "CURLM not valid easy handle";

    case CURLM_OUT_OF_MEMORY:
	return "CURLM libcurl out of memory";

    case CURLM_INTERNAL_ERROR:
	return "CURLM libcurl internal bug";

    case CURLM_LAST:
	assert(0);
	break;
    }
    return "unknown CURLMcode";
}


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
    fp = p;
    time(&curtim);
    curtim -= fp->progress_start;
    print_byte_count(buf1, sizeof(buf1), (long)down_current);
    print_byte_count(buf2, sizeof(buf2), (long)down_total);
    frac = (down_total <= 0) ? 0 : (down_current / down_total);
    predict = frac ? (0.5 + curtim / frac) : 0;
    remaining = predict - curtim;
    print_seconds(buf3, sizeof(buf3), remaining);

    memset(fp->progress_buffer, ' ', fp->progress_buflen);
    fp->progress_buffer[0] = '\r';
    memcpy(fp->progress_buffer +  1, buf1, 6);
    memcpy(fp->progress_buffer +  7, " of ", 4);
    memcpy(fp->progress_buffer + 11, buf2, 6);
    snprintf(fp->progress_buffer + 18, 5, "%3d%%", (int)(100 * frac + 0.5));

    lhs = 24;
    rhs = lhs + (fp->progress_buflen - 36) * frac;
    while (lhs < rhs)
	fp->progress_buffer[lhs++] = '=';
    fp->progress_buffer[lhs] = '>';

    memcpy(fp->progress_buffer + fp->progress_buflen - 10, "ETA", 3);
    memcpy(fp->progress_buffer + fp->progress_buflen - 6, buf3, 6);
    write(2, fp->progress_buffer, fp->progress_buflen);
    fp->progress_cleanup = 1;
    return 0;
}


/*
 * Libcurl calls this function when it receives more data.
 */
static size_t
callback(char *data, size_t size, size_t nitems, void *p)
{
    input_curl_ty   *fp;
    size_t          nbytes;

    fp = p;
    nbytes = size * nitems;

    /*
     * Grow the buffer if necessary.
     *
     * Always keep it a power of 2, because sigma(2**-n)==1, so we get
     * O(1) behaviour.  (That +32 means we are always just 32 bytes
     * short of a power of 2, leaving room for the malloc header, which
     * results in a nicer malloc fit on many systems.
     */
    while (fp->buffer_length + nbytes > fp->buffer_maximum)
    {
	fp->buffer_maximum = fp->buffer_maximum * 2 + 32;
	fp->buffer = mem_change_size(fp->buffer, fp->buffer_maximum);
    }

    /*
     * Copy the data into the buffer.
     */
    memcpy(fp->buffer + fp->buffer_length, data, nbytes);
    fp->buffer_length += nbytes;

    /*
     * A negative return will stop the transfer for this stream.
     */
    return nbytes;
}


static void
destruct(input_ty *p)
{
    input_curl_ty   *fp;

    /*
     * Release libcurl resources.
     */
    fp = (input_curl_ty *)p;
    curl_multi_remove_handle(multi_handle, fp->handle);
    curl_easy_cleanup(fp->handle);
    fp->handle = 0;
    fp->eof = 1;

    if (fp->progress_cleanup)
	write(2, "\n", 1);

    /*
     * Release dynamic memory resources.
     */
    if (fp->buffer)
        mem_free(fp->buffer);
    fp->buffer = 0;
    fp->buffer_postion = 0;
    fp->buffer_length = 0;
    fp->buffer_maximum = 0;
    str_free(fp->fn);
    fp->fn = 0;
}


static input_curl_ty *
handle_to_fp(CURL *handle)
{
    input_curl_ty   *result;

    assert(stp);
    result = itab_query(stp, (itab_key_ty)handle);
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
    /*
     * See if there are any messages waiting.
     * These tell us about errors, and completed transfers.
     */
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
	    /* transfer over, no error */
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
	    /* NOTREACHED */
	}
    }

    /*
     * Look for more to happen.
     */
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
		    CURLMcode_strerror(ret)
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

	/* get file descriptors from the transfers */
	err =
	    curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);
	if (err)
	    FATAL("curl_multi_fdset", CURLcode_strerror(err));

	if (maxfd >= 0)
	{
	    struct timeval  timeout;
	    int             rc;

	    /* set a suitable timeout to fail on */
	    timeout.tv_sec = 60; /* 1 minute */
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
		    /* NOTREACHED */
		}
	    }
	    if (rc > 0)
	    {
		/*
		 * Some sockets are ready.
		 */
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

    /*
     * attempt to fill buffer
     */
    while (!fp->eof && fp->buffer_postion + nbytes > fp->buffer_length)
	perform();

    /*
     * Extract as much data as possible from the buffer.
     */
    size_of_buffer = fp->buffer_length - fp->buffer_postion;
    if (nbytes > size_of_buffer)
	nbytes = size_of_buffer;
    memcpy(data, fp->buffer + fp->buffer_postion, nbytes);
    fp->buffer_postion += nbytes;

    /*
     * Rearrange the buffer so that it does not grow forever.
     */
    size_of_buffer = fp->buffer_length - fp->buffer_postion;
    if (size_of_buffer == 0)
    {
	fp->buffer_postion = 0;
	fp->buffer_length = 0;
    }
    else if (size_of_buffer <= fp->buffer_postion)
    {
	/* can shuffle the data down easily */
	memcpy(fp->buffer, fp->buffer + fp->buffer_postion, size_of_buffer);
	fp->buffer_postion = 0;
	fp->buffer_length = size_of_buffer;
    }

    /*
     * Return the number of bytes read.
     */
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
    /* Maybe there was a Content-Length header? */
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
	FATAL("curl_easy_setopt", CURLcode_strerror(err));
    err = curl_easy_setopt(fp->handle, CURLOPT_URL, fp->fn->str_text);
    if (err)
	FATAL("curl_easy_setopt", CURLcode_strerror(err));
    err = curl_easy_setopt(fp->handle, CURLOPT_FILE, fp);
    if (err)
	FATAL("curl_easy_setopt", CURLcode_strerror(err));
    err = curl_easy_setopt(fp->handle, CURLOPT_VERBOSE, 0);
    if (err)
	FATAL("curl_easy_setopt", CURLcode_strerror(err));
    err = curl_easy_setopt(fp->handle, CURLOPT_WRITEFUNCTION, callback);
    if (err)
	FATAL("curl_easy_setopt", CURLcode_strerror(err));
    err = curl_easy_setopt(fp->handle, CURLOPT_FOLLOWLOCATION, 1);
    if (err)
	FATAL("curl_easy_setopt", CURLcode_strerror(err));

    fp->progress_start = 0;
    fp->progress_buflen = 0;
    fp->progress_buffer = 0;
    fp->progress_cleanup = 0;
    if (option_verbose_get())
    {
	err = curl_easy_setopt(fp->handle, CURLOPT_NOPROGRESS, 0);
	if (err)
	    FATAL("curl_easy_setopt", CURLcode_strerror(err));
	err = curl_easy_setopt(fp->handle, CURLOPT_PROGRESSFUNCTION, progress);
	if (err)
	    FATAL("curl_easy_setopt", CURLcode_strerror(err));
	err = curl_easy_setopt(fp->handle, CURLOPT_PROGRESSDATA, fp);
	if (err)
	    FATAL("curl_easy_setopt", CURLcode_strerror(err));
	time(&fp->progress_start);
	fp->progress_buflen = page_width_get(80);
	if (fp->progress_buflen < 40)
	    fp->progress_buflen = 40;
	fp->progress_buffer = mem_alloc(fp->progress_buflen);
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
	FATAL("curl_multi_add_handle", CURLMcode_strerror(merr));
    }

    /*
     * Start the fetch as soon as possible.
     */
    call_multi_immediate = 1;

    /*
     * Build an associate table from libcurl handles to our file pointers.
     */
    if (!stp)
	stp = itab_alloc(1);
    itab_assign(stp, (itab_key_ty)fp->handle, (void *)fp);

    /*
     * Report success.
     */
    return result;
}


int
input_curl_looks_likely(string_ty *fn)
{
    const char *cp;

    cp = fn->str_text;
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

#endif /* HAVE_LIBCURL */
