//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003-2005 Peter Miller;
//	All rights reserved.
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
// MANIFEST: functions to manipulate cpio_child2s
//

#include <output/cpio_child.h>
#include <output/cpio_child2.h>
#include <output/memory.h>


output_cpio_child2_ty::~output_cpio_child2_ty()
{
    //
    // Make sure all buffered data has been passed to our write_inner
    // method.
    //
    flush();

    //
    // Create a cpio archive member, now that the length is known.
    //
    output_ty *tmp = new output_cpio_child_ty(deeper, name, buffer->ftell());

    //
    // Forward the stashed data to the cpio archive member.
    //
    buffer->forward(tmp);

    //
    // Get rid all the machinery necessary to pull this off, now that
    // we've done it.
    //
    delete tmp;
    delete buffer;
    buffer = 0;

    //
    // DO NOT delete deeper;
    // this is output_cpio::destructor's job.
    //
}


output_cpio_child2_ty::output_cpio_child2_ty(output_ty *arg1,
	const nstring &arg2) :
    deeper(arg1),
    name(arg2),
    buffer(new output_memory_ty())
{
}


string_ty *
output_cpio_child2_ty::filename()
    const
{
    return deeper->filename();
}


long
output_cpio_child2_ty::ftell_inner()
    const
{
    return buffer->ftell();
}


void
output_cpio_child2_ty::write_inner(const void *data, size_t len)
{
    buffer->write(data, len);
}


void
output_cpio_child2_ty::flush_inner()
{
    buffer->flush();
}


void
output_cpio_child2_ty::end_of_line_inner()
{
    buffer->end_of_line();
}


const char *
output_cpio_child2_ty::type_name()
    const
{
    return "cpio buffered child";
}
