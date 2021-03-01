//
//	aegis - project change supervisor
//	Copyright (C) 2003-2006, 2008 Peter Miller
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

#ifndef COMMON_AC_CURL_CURL_H
#define COMMON_AC_CURL_CURL_H

#include <common/config.h>

#ifdef HAVE_CURL_CURL_H
#include <curl/curl.h>

//
// Some older versions of libcurl (CURL_VERSION < 0x070A00) don't
// include <curl/multi.h> for you, so you have to do it yourself, but it
// has to be after <curl/curl.h> is included.  It is safe for this file
// to be included more than once.
//
#ifdef HAVE_CURL_MULTI_H
#include <curl/multi.h>
#endif

#ifndef HAVE_CURL_EASY_STRERROR

/**
  * The curl_easy_strerror function may be used to turn a CURLcode value
  * into the equivalent human readable error string.  This is useful for
  * printing meaningful error messages.
  */
const char *curl_easy_strerror(CURLcode);

#endif // HAVE_CURL_EASY_STRERROR

#ifndef HAVE_CURL_MULTI_STRERROR

/**
  * The curl_multi_strerror function may be used to turn a CURLMcode
  * value into the equivalent human readable error string.  This is
  * useful for printing meaningful error messages.
  */
const char *curl_multi_strerror(CURLMcode);

#endif // HAVE_CURL_MULTI_STRERROR
#else // !HAVE_CURL_CURL_H
struct CURL; // forward
#define CURL_ERROR_SIZE 100
#endif // HAVE_CURL_CURL_H

#endif // COMMON_AC_CURL_CURL_H
