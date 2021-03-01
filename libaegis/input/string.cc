//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
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
// MANIFEST: functions to manipulate strings
//

#include <ac/string.h>

#include <input/string.h>
#include <input/private.h>
#include <str.h>

typedef struct input_string_ty input_string_ty;
struct input_string_ty
{
    input_ty        inherited;
    string_ty       *base;
    size_t          pos;
    string_ty       *name;
};


static void
input_string_destructor(input_ty *p)
{
    input_string_ty *isp;

    isp = (input_string_ty *)p;
    str_free(isp->base);
    str_free(isp->name);
}


static long
input_string_read(input_ty *p, void *data, size_t len)
{
    input_string_ty *isp;
    size_t          nbytes;

    isp = (input_string_ty *)p;
    if (isp->pos >= isp->base->str_length)
	return 0;
    nbytes = isp->base->str_length - isp->pos;
    if (nbytes > len)
	nbytes = len;
    memcpy(data, isp->base->str_text + isp->pos, nbytes);
    isp->pos += nbytes;
    return nbytes;
}


static long
input_string_ftell(input_ty *p)
{
    input_string_ty *isp;

    isp = (input_string_ty *)p;
    return isp->pos;
}


static string_ty *
input_string_name(input_ty *p)
{
    input_string_ty *isp;

    isp = (input_string_ty *)p;
    return isp->name;
}


static long
input_string_length(input_ty *p)
{
    input_string_ty *isp;

    isp = (input_string_ty *)p;
    return isp->base->str_length;
}


static input_vtbl_ty vtbl =
{
    sizeof(input_string_ty),
    input_string_destructor,
    input_string_read,
    input_string_ftell,
    input_string_name,
    input_string_length,
    0, // keepalive
};


input_ty *
input_string_new(string_ty *s)
{
    input_ty        *result;
    input_string_ty *isp;

    result = input_new(&vtbl);
    isp = (input_string_ty *)result;
    isp->base = str_copy(s);
    isp->pos = 0;
    isp->name = str_from_c("generated string");
    return result;
}
