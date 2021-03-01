//
// aegis - project change supervisor
// Copyright (C) 1999, 2003-2006, 2008, 2011, 2012 Peter Miller
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

#include <libaegis/output/cpio_child.h>
#include <libaegis/output/cpio_child2.h>
#include <libaegis/output/memory.h>


output_cpio_child2::~output_cpio_child2()
{
    //
    // Make sure all buffered data has been passed to our write_inner
    // method.
    //
    flush();

    //
    // Create a cpio archive member, now that the length is known.
    //
    output::pointer tmp =
        output_cpio_child::create(deeper, name, buffer->ftell(), mtime);

    //
    // Forward the stashed data to the cpio archive member.
    //
    buffer->forward(tmp);
}


output_cpio_child2::output_cpio_child2(const output::pointer &a_deeper,
        const nstring &a_name, time_t a_mtime) :
    deeper(a_deeper),
    name(a_name),
    buffer(output_memory::create()),
    mtime(a_mtime)
{
}


output::pointer
output_cpio_child2::create(const output::pointer &a_deeper,
    const nstring &a_name, time_t a_mtime)
{
    return pointer(new output_cpio_child2(a_deeper, a_name, a_mtime));
}


nstring
output_cpio_child2::filename(void)
    const
{
    return deeper->filename();
}


long
output_cpio_child2::ftell_inner(void)
    const
{
    return buffer->ftell();
}


void
output_cpio_child2::write_inner(const void *data, size_t len)
{
    buffer->write(data, len);
}


void
output_cpio_child2::flush_inner(void)
{
    buffer->flush();
}


void
output_cpio_child2::end_of_line_inner(void)
{
    buffer->end_of_line();
}


nstring
output_cpio_child2::type_name(void)
    const
{
    return "cpio buffered child";
}


// vim: set ts=8 sw=4 et :
