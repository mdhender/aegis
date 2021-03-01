//
//	aegis - project change supervisor
//	Copyright (C) 2002-2006 Peter Miller
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
// MANIFEST: functions to manipulate read_stricts
//

#include <libaegis/input.h>
#include <common/str.h>


bool
input_ty::read_strict(void *buf, size_t len)
{
    long asked = len;
    long got = 0;
    while (len > 0)
    {
	long result = read(buf, len);
	if (result <= 0)
	{
	    if (got == 0)
		return false;
	    nstring s =
		nstring::format("short read (asked %ld, got %ld)", asked, got);
	    fatal_error(s.c_str());
	}
	len -= result;
	got += result;
	buf = (char *)buf + result;
    }
    return true;
}
