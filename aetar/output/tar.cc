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
// MANIFEST: functions to output to tar archivess
//

#include <common/ac/string.h>

#include <common/error.h>
#include <aetar/output/tar.h>
#include <aetar/output/tar_child.h>


output_tar_ty::~output_tar_ty()
{
    //
    // Write a 512-byte block of zero (three times) to indicate end of
    // file.  While GNU tar(1) doesn't care, BSD tar(1) and pax(1) do
    // care.
    //
    char zero[512];
    memset(zero, 0, sizeof(zero));
    deeper->write(zero, sizeof(zero));
    // Three times is weird, but it's what other tar format tools use.
    // Don't ask me why: even one is redundant.
    deeper->write(zero, sizeof(zero));
    deeper->write(zero, sizeof(zero));

    //
    // Finish writing the archive file.
    //
    delete deeper;
    deeper = 0;
}


output_tar_ty::output_tar_ty(output_ty *arg1) :
    deeper(arg1)
{
}


string_ty *
output_tar_ty::filename()
    const
{
    return deeper->filename();
}


long
output_tar_ty::ftell_inner()
    const
{
    return 0;
}


void
output_tar_ty::write_inner(const void *, size_t)
{
    this_is_a_bug();
}


void
output_tar_ty::end_of_line_inner()
{
    this_is_a_bug();
}


const char *
output_tar_ty::type_name()
    const
{
    return "tar archive";
}


output_ty *
output_tar_ty::child(const nstring &name, long len, bool executable)
{
    assert(len >= 0);
    return new output_tar_child_ty(deeper, name, len, executable);
}
