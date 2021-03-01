//
//	aegis - project change supervisor
//	Copyright (C) 2002-2006, 2008 Peter Miller
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

#include <libaegis/input.h>


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
