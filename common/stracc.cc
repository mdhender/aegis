//
//	aegis - project change supervisor
//	Copyright (C) 1998, 2001, 2003, 2004 Peter Miller;
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
// MANIFEST: functions to accumulate strings
//

#include <ac/string.h>

#include <mem.h>
#include <stracc.h>


void
stracc_constructor(stracc_t *sap)
{
    sap->length = 0;
    sap->maximum = 0;
    sap->buffer = 0;
}


void
stracc_destructor(stracc_t *sap)
{
    if (sap->buffer)
	mem_free(sap->buffer);
    sap->length = 0;
    sap->maximum = 0;
    sap->buffer = 0;
}


void
stracc_open(stracc_t *sap)
{
    sap->length = 0;
}


string_ty *
stracc_close(const stracc_t *sap)
{
    return str_n_from_c(sap->buffer, sap->length);
}


#undef stracc_char

void
stracc_char(stracc_t *sap, int c)
{
    if (sap->length >= sap->maximum)
    {
	sap->maximum = sap->maximum * 2 + 16;
	sap->buffer = (char *)mem_change_size(sap->buffer, sap->maximum);
    }
    sap->buffer[sap->length++] = c;
}


void
stracc_chars(stracc_t *sap, const char *cp, size_t n)
{
    if (!n)
	return;
    if (sap->length + n > sap->maximum)
    {
	for (;;)
	{
	    sap->maximum = sap->maximum * 2 + 16;
	    if (sap->length + n <= sap->maximum)
		break;
	}
	sap->buffer = (char *)mem_change_size(sap->buffer, sap->maximum);
    }
    memcpy(sap->buffer + sap->length, cp, n);
    sap->length += n;
}
