//
// aegis - project change supervisor
// Copyright (C) 2012 Peter Miller
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

#include <common/ac/assert.h>

#include <aemakegen/util.h>


nstring
make_pseudo_dir(const nstring &text)
{
    // This assumes the system has long file names.
    // alternatively, use a monotonically increasing number?!?
    return ".mkdir." + text.identifier();
}


nstring
make_pseudo_dir_for(const nstring &dst_file)
{
    nstring dir = dst_file.dirname();
    assert(dir != ".");
    assert(dir != "/");
    return make_pseudo_dir(dir);
}


// vim: set ts=8 sw=4 et :
