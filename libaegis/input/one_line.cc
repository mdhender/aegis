//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003-2005 Peter Miller;
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
// MANIFEST: functions to manipulate one_lines
//

#include <libaegis/input.h>
#include <common/nstring/accumulator.h>


bool
input_ty::one_line(nstring &result)
{
    static nstring_accumulator line_buffer;
    line_buffer.clear();
    for (;;)
    {
	int c = getch();
	if (c < 0)
	{
	    if (line_buffer.empty())
	       	return false;
	    break;
	}
	if (c == '\n')
	    break;
	line_buffer.push_back(c);
    }
    result = line_buffer.mkstr();
    return true;
}
