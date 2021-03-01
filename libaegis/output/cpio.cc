//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003-2008 Peter Miller
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

#include <common/error.h>
#include <common/mem.h>
#include <common/str.h>
#include <libaegis/output/cpio.h>
#include <libaegis/output/cpio_child.h>
#include <libaegis/output/cpio_child2.h>


output_cpio::~output_cpio()
{
    //
    // Emit the archive trailer.
    // (An empty file with a magic name.)
    //
    nstring trailer("TRAILER!!!");
    output_cpio_child::create(deeper, trailer, 0L, mtime);
}


output_cpio::output_cpio(const output::pointer &arg1, time_t a_mtime) :
    deeper(arg1),
    mtime(a_mtime)
{
}


nstring
output_cpio::filename()
    const
{
    return deeper->filename();
}


long
output_cpio::ftell_inner()
    const
{
    return 0;
}


void
output_cpio::write_inner(const void *, size_t)
{
    this_is_a_bug();
}


void
output_cpio::end_of_line_inner()
{
    this_is_a_bug();
}


const char *
output_cpio::type_name()
    const
{
    return "cpio archive";
}


output::pointer
output_cpio::child(const nstring &name, long len)
{
    if (len < 0)
    {
	//
        // With no length given, we have to use the memory caching
        // version, which stashes the data in memory until the end, and
        // then we can write it out with the length in the header.
	//
	return output_cpio_child2::create(deeper, name, mtime);
    }
    return output_cpio_child::create(deeper, name, len, mtime);
}
