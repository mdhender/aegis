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

#include <aemakegen/target/debian.h>


void
target_debian::process3_end(void)
{
    //
    // Assume we are running in either the development directory, or the
    // integration directory, as appropriate for our state, as a result
    // of being invoked by DMT.
    //
    os_become_orig();
    if (!os_exists("debian"))
        os_mkdir("debian", 0755);
    os_become_undo();

    fill_instance_variables();

    gen_control();
    gen_rules();
    gen_compat();
    gen_copyright();
    gen_changelog();
    gen_source_format();
}


// vim: set ts=8 sw=4 et :
