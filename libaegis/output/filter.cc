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
#include <libaegis/output/filter.h>


output_filter::~output_filter()
{
    trace(("~output_filter(this = %p)\n", this));
}


output_filter::output_filter(const output::pointer &a_deeper) :
    deeper(a_deeper)
{
    trace(("output_filter(this = %p)\n", this));
    assert(deeper);
}


nstring
output_filter::filename(void)
    const
{
    trace(("output_filter::filename(this = %p)\n", this));
    return deeper->filename();
}


int
output_filter::page_width(void)
    const
{
    trace(("output_filter::page_width(this = %p)\n", this));
    return deeper->page_width();
}


int
output_filter::page_length(void)
    const
{
    trace(("output_filter::page_length(this = %p)\n", this));
    return deeper->page_length();
}


long
output_filter::ftell_inner(void)
    const
{
    trace(("output_filter::ftell_inner(this = %p)\n", this));
    return deeper->ftell();
}


void
output_filter::write_inner(const void *data, size_t data_size)
{
    trace(("output_filter::write_inner(this = %p)\n{\n", this));
    deeper->write(data, data_size);
    trace(("}\n"));
}


void
output_filter::end_of_line_inner(void)
{
    trace(("output_filter::end_of_line_inner(this = %p)\n{\n", this));
    deeper->end_of_line();
    trace(("}\n"));
}


void
output_filter::flush_inner(void)
{
    trace(("output_filter::flush_inner(this = %p)\n{\n", this));
    deeper->flush();
    trace(("}\n"));
}


nstring
output_filter::type_name(void)
    const
{
    return ("filter > " + deeper->type_name());
}


// vim: set ts=8 sw=4 et :
