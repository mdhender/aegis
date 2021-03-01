//
//	aegis - project change supervisor
//	Copyright (C) 2002-2005 Peter Miller;
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
// MANIFEST: functions to manipulate read_strict2s
//

#include <input.h>


void
input_ty::read_strictest(void *buf, size_t len)
{
    long asked = len;
    long got = 0;
    while (len > 0)
    {
	long result = read(buf, len);
	if (result <= 0)
	{
	    nstring s =
		nstring::format("short read (asked %ld, got %ld)", asked, got);
	    fatal_error(s.c_str());
	}
	len -= result;
	got += result;
	buf = (char *)buf + result;
    }
}
