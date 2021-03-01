//
// aegis - project change supervisor
// Copyright (C) 1991-1995, 1997-1999, 2001-2006, 2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <libaegis/col.h>
#include <common/trace.h>


col::~col()
{
    trace(("col::~col(this = %p)\n{\n", this));
    trace(("}\n"));
}


col::col()
{
    trace(("col::col(this = %p)\n{\n", this));
    trace(("}\n"));
}


void
col::title(const char *first, const char *second)
{
    title(nstring(first), nstring(second));
}


// vim: set ts=8 sw=4 et :
