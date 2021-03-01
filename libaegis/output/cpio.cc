//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003-2006 Peter Miller;
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
// MANIFEST: functions to output to cpio archivess
//

#include <common/error.h>
#include <libaegis/output/cpio.h>
#include <libaegis/output/cpio_child.h>
#include <libaegis/output/cpio_child2.h>
#include <common/str.h>


output_cpio_ty::~output_cpio_ty()
{
    //
    // Emit the archive trailer.
    // (An empty file with a magic name.)
    //
    nstring trailer("TRAILER!!!");
    delete new output_cpio_child_ty(deeper, trailer, false);

    //
    // Finish writing the archive file.
    //
    delete deeper;
    deeper = 0;
}


output_cpio_ty::output_cpio_ty(output_ty *arg1) :
    deeper(arg1)
{
}


string_ty *
output_cpio_ty::filename()
    const
{
    return deeper->filename();
}


long
output_cpio_ty::ftell_inner()
    const
{
    return 0;
}


void
output_cpio_ty::write_inner(const void *data, size_t len)
{
    this_is_a_bug();
}


void
output_cpio_ty::end_of_line_inner()
{
    this_is_a_bug();
}


const char *
output_cpio_ty::type_name()
    const
{
    return "cpio archive";
}


output_ty *
output_cpio_ty::child(const nstring &name, long len)
{
    if (len < 0)
    {
	//
        // With no length given, we have to use the memory caching
        // version, which stashes the data in memory until the end, and
        // then we can write it out with the length in the header.
	//
	return new output_cpio_child2_ty(deeper, name);
    }
    return new output_cpio_child_ty(deeper, name, len);
}
