//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
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

#include <common/main.h>
#include <common/error.h>              // for assert
#include <common/ac/curl/curl.h>


#ifdef HAVE_CURL_CURL_H
#ifndef HAVE_CURL_EASY_STRERROR

const char *
curl_easy_strerror(CURLcode x)
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

#if (LIBCURL_VERSION_NUM >= 0x070A00)
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
#endif // 0x070A00 7.10.0

#if (LIBCURL_VERSION_NUM >= 0x070A08)
    case CURLE_LDAP_INVALID_URL:
	return "Invalid LDAP URL";

    case CURLE_FILESIZE_EXCEEDED:
	return "Maximum file size exceeded";
#endif // 0x070A08 7.10.8

#if (LIBCURL_VERSION_NUM >= 0x070B00)
    case CURLE_FTP_SSL_FAILED:
        return "Requested FTP SSL level failed";
#endif // 0x070B00 7.11.0

    case CURL_LAST:
	assert(0);
	break;

#ifndef DEBUG
    default:
        return "unknown CURLcode";
#endif
    }
    return "unknown CURLcode";
}


#endif
#ifndef HAVE_CURL_MULTI_STRERROR


const char *
curl_multi_strerror(CURLMcode x)
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

#endif
#endif // HAVE_CURL_CURL_H
