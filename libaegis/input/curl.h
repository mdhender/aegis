/*
 *	aegis - project change supervisor
 *	Copyright (C) 2003, 2005 Peter Miller;
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
 * MANIFEST: interface definition for libaegis/input/curl.c
 */

#ifndef LIBAEGIS_INPUT_CURL_H
#define LIBAEGIS_INPUT_CURL_H

#include <input.h>

/**
  * The input_curl_open function is used to open an input stream which
  * reads from a Uniform Resource Locator (URL), typically an HTTP or
  * FTP site somewhere i=on the Internet.
  */
input_ty *input_curl_open(struct string_ty *);

/**
  * The input_curl_looks_likely function is used to examine a filename
  * and see if it starts with a prototype name (e.g. http: or ftp:)
  * making it a likely bcandidate for the input_curl_open function.
  */
bool input_curl_looks_likely(struct string_ty *);

#endif /* LIBAEGIS_INPUT_CURL_H */
