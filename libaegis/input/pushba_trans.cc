//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2003-2006, 2008, 2012 Peter Miller
//      Copyright (C) 2008 Walter Franzini
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


void
input_ty::pushback_transfer(input &from)
{
    from->pullback_transfer(this);
}


void
input_ty::pullback_transfer(input_ty::pointer to)
{
    if (buffer_position < buffer_end)
    {
        to->unread(buffer_position, buffer_end - buffer_position);
        buffer_position = buffer_end;
    }
}


void
input_ty::pullback_transfer(input &to)
{
    pullback_transfer(to.ref);
}


// vim: set ts=8 sw=4 et :
