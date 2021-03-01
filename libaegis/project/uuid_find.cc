//
//      aegis - project change supervisor
//      Copyright (C) 2004-2008, 2011, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <libaegis/change/branch.h>
#include <libaegis/change/list.h>
#include <libaegis/project.h>


change::pointer
project_uuid_find(project *pp, string_ty *uuid)
{
    if (!pp->is_a_trunk())
        return project_uuid_find(pp->trunk_get(), uuid);

    change_list_ty result;
    change_branch_uuid_find(pp->change_get(), uuid, result);
    if (result.size() == 1)
        return result[0];
    for (size_t j = 0; j < result.size(); ++j)
         change_free(result[j]);
    return 0;
}


// vim: set ts=8 sw=4 et :
