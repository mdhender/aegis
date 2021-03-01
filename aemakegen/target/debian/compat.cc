//
// aegis - project change supervisor
// Copyright (C) 2008, 2011, 2012 Peter Miller
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

#include <libaegis/os.h>
#include <libaegis/output/file.h>

#include <aemakegen/target/debian.h>


void
target_debian::gen_compat(void)
{
    // write debian/copyright
    os_become_orig();
    output::pointer fp = output_file::open("debian/compat");
    fp->fputs("5\n");
    fp.reset();
    os_become_undo();
}


void
target_debian::gen_source_format(void)
{
    // write debian/copyright
    os_become_orig();
    nstring filename = "debian/source/format";
    os_mkdir_between(".", filename, 0755);
    output::pointer fp = output_file::open(filename);
    fp->fputs("1.0\n");
    fp.reset();
    os_become_undo();
}


// vim: set ts=8 sw=4 et :
