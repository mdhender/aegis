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

#include <common/ac/assert.h>
#include <common/ac/string.h>

#include <common/error.h>

#include <aetar/output/tar.h>
#include <aetar/output/tar_child.h>


output_tar::~output_tar()
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
    deeper->flush();
}


output_tar::output_tar(const output::pointer &arg1) :
    deeper(arg1)
{
}


nstring
output_tar::filename(void)
    const
{
    return deeper->filename();
}


long
output_tar::ftell_inner(void)
    const
{
    return 0;
}


void
output_tar::write_inner(const void *, size_t)
{
    this_is_a_bug();
}


void
output_tar::end_of_line_inner(void)
{
    this_is_a_bug();
}


nstring
output_tar::type_name(void)
    const
{
    return ("tar archive > " + deeper->type_name());
}


output::pointer
output_tar::child(const nstring &name, long len, bool executable)
{
    assert(len >= 0);
    return output_tar_child::create(deeper, name, len, executable);
}


// vim: set ts=8 sw=4 et :
