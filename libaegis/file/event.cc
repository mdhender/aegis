//
//	aegis - project change supervisor
//	Copyright (C) 2006-2008 Peter Miller
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

#include <libaegis/file/event.h>


file_event::~file_event()
{
    when = 0;
    cp = 0;
    src = 0;
}


file_event::file_event() :
    when(0),
    cp(0),
    src(0)
{
}


file_event::file_event(time_t a_when, change::pointer a_cp,
        fstate_src_ty *a_src) :
    when(a_when),
    cp(a_cp),
    src(a_src)
{
}


file_event::file_event(const file_event &arg) :
    when(arg.when),
    cp(arg.cp),
    src(arg.src)
{
}


file_event &
file_event::operator=(const file_event &arg)
{
    if (this != &arg)
    {
	when = arg.when;
	cp = arg.cp;
	src = arg.src;
    }
    return *this;
}
