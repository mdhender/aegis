//
// aegis - project change supervisor
// Copyright (C) 2002-2008, 2011, 2012 Peter Miller
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

#include <common/page.h>
#include <libaegis/output/bit_bucket.h>


output_bit_bucket::~output_bit_bucket()
{
    //
    // Make sure all buffered data has been passed to our write_inner
    // method.
    //
    flush();
}


output_bit_bucket::output_bit_bucket() :
    file_name("/dev/null"),
    pos(0)
{
}


output::pointer
output_bit_bucket::create(void)
{
    return pointer(new output_bit_bucket());
}


nstring
output_bit_bucket::filename(void)
    const
{
    return file_name;
}


long
output_bit_bucket::ftell_inner(void)
    const
{
    return pos;
}


void
output_bit_bucket::write_inner(const void *, size_t len)
{
    pos += len;
}


int
output_bit_bucket::page_width(void)
    const
{
    return page_width_get(DEFAULT_PRINTER_WIDTH);
}


int
output_bit_bucket::page_length(void)
    const
{
    return page_length_get(DEFAULT_PRINTER_LENGTH);
}


void
output_bit_bucket::end_of_line_inner(void)
{
}


nstring
output_bit_bucket::type_name(void)
    const
{
    return file_name;
}


// vim: set ts=8 sw=4 et :
