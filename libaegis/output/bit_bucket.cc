//
//	aegis - project change supervisor
//	Copyright (C) 2002-2007 Peter Miller
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
// MANIFEST: functions to manipulate bit_buckets
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


string_ty *
output_bit_bucket::filename()
    const
{
    return file_name.get_ref();
}


long
output_bit_bucket::ftell_inner()
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
output_bit_bucket::page_width()
    const
{
    return page_width_get(DEFAULT_PRINTER_WIDTH);
}


int
output_bit_bucket::page_length()
    const
{
    return page_length_get(DEFAULT_PRINTER_LENGTH);
}


void
output_bit_bucket::end_of_line_inner()
{
}


const char *
output_bit_bucket::type_name()
    const
{
    return file_name.c_str();
}
