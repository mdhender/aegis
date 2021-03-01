//
// aegis - project change supervisor
// Copyright (C) 2001, 2003-2006, 2008, 2011, 2012 Peter Miller
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

#include <libaegis/output/tee.h>


output_tee::~output_tee()
{
    //
    // Make sure all buffered data has been passed to our write_inner
    // method.
    //
    flush();
}


output_tee::output_tee(const output::pointer &a1, const output::pointer &a2) :
    d1(a1),
    d2(a2)
{
}


output::pointer
output_tee::create(const output::pointer &a1, const output::pointer &a2)
{
    return pointer(new output_tee(a1, a2));
}


void
output_tee::write_inner(const void *data, size_t nbytes)
{
    d1->write(data, nbytes);
    d2->write(data, nbytes);
}


void
output_tee::flush_inner()
{
    d1->flush();
    d2->flush();
}


nstring
output_tee::filename()
    const
{
    return d1->filename();
}


long
output_tee::ftell_inner()
    const
{
    long result = d1->ftell();
    if (result < 0)
        result = d2->ftell();
    return result;
}


int
output_tee::page_width()
    const
{
    return d1->page_width();
}


int
output_tee::page_length()
    const
{
    return d1->page_length();
}


void
output_tee::end_of_line_inner()
{
    d1->end_of_line();
    d2->end_of_line();
}


nstring
output_tee::type_name()
    const
{
    return ("tee(" + d1->type_name() + "," + d2->type_name() + ")");
}


// vim: set ts=8 sw=4 et :
