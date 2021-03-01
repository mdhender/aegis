//
//      aegis - project change supervisor
//      Copyright (C) 2003-2008, 2011, 2012 Peter Miller
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

#include <libaegis/change.h>
#include <libaegis/project.h>
#include <libaegis/project/directory.h>


string_ty *
project_directory_get(project *pp, int resolve)
{
    if (pp->is_a_trunk())
        return pp->baseline_path_get(resolve);
    change::pointer cp = pp->change_get();
    if (cp->is_being_developed())
        return pp->baseline_path_get(resolve);
    return project_directory_get(pp->parent_get(), resolve);
}


// vim: set ts=8 sw=4 et :
