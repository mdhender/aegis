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
 * MANIFEST: interface definition for common/ac/curl/curl.c
 */

#ifndef COMMON_AC_CURL_CURL_H
#define COMMON_AC_CURL_CURL_H

#include <config.h>

#ifdef HAVE_CURL_CURL_H
#include <curl/curl.h>

#ifndef HAVE_CURL_EASY_STRERROR

/**
  * The curl_easy_strerror function may be used to turn a CURLcode value
  * into the equivalent human readable error string.  This is useful for
  * printing meaningful error messages.
  */
const char *curl_easy_strerror(CURLcode);

#endif /* HAVE_CURL_EASY_STRERROR */

#ifndef HAVE_CURL_MULTI_STRERROR

/**
  * The curl_multi_strerror function may be used to turn a CURLMcode
  * value into the equivalent human readable error string.  This is
  * useful for printing meaningful error messages.
  */
const char *curl_multi_strerror(CURLMcode);

#endif /* HAVE_CURL_MULTI_STRERROR */
#endif /* HAVE_CURL_CURL_H */

#endif /* COMMON_AC_CURL_CURL_H */
