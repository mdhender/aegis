//
// aegis - project change supervisor
// Copyright (C) 2011, 2012 Peter Miller
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
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/trace.h>
#include <libaegis/output/filter/set_width.h>


output_filter_set_width::~output_filter_set_width()
{
    trace(("~output_filter_set_width(this = %p) {\n", this));
    flush();
    trace(("}\n"));
}


output_filter_set_width::output_filter_set_width(
    const output::pointer &a_deeper,
    int a_width
) :
    output_filter(a_deeper),
    width(a_width)
{
    trace(("output_filter_set_width(this = %p, deeper = %p, width = %d)\n",
        this, a_deeper.get(), width));
    assert(width > 0);
}


output_filter_set_width::pointer
output_filter_set_width::create(const output::pointer &a_deeper, int a_width)
{
    return pointer(new output_filter_set_width(a_deeper, a_width));
}


int
output_filter_set_width::page_width(void)
    const
{
    trace(("output_filter_set_width::page_width(this = %p) {\n", this));
    return width;
}


// vim: set ts=8 sw=4 et :
