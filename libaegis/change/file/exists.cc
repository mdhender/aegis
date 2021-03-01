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

#include <libaegis/change.h>
#include <libaegis/change/file.h>


bool
change::file_exists(const nstring &filename)
{
    fstate_src_ty *c_src_data = file_find(filename, view_path_first);
    return
        (
            c_src_data
        &&
            !c_src_data->deleted_by
        &&
            !c_src_data->about_to_be_created_by
        );
}


// vim: set ts=8 sw=4 et :
