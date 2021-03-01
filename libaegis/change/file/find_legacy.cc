//
// aegis - project change supervisor
// Copyright (C) 2005-2008, 2012 Peter Miller
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

#include <libaegis/change/file.h>


fstate_src_ty *
change::file_find(cstate_src_ty *src, view_path_ty vp)
{
    if (!src->uuid)
        return file_find(nstring(src->file_name), vp);
    return file_find_uuid(src->uuid, vp);
}


// vim: set ts=8 sw=4 et :
