//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2003-2006, 2008, 2009, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <libaegis/input.h>
#include <common/nstring/accumulator.h>


bool
input_ty::one_line(nstring &result)
{
    nstring_accumulator line_buffer;
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


// vim: set ts=8 sw=4 et :
