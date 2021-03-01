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
#include <aeget/get/change/files.h>
#include <aeget/get/project/files.h>
#include <libaegis/project.h>
#include <common/str_list.h>


void
get_project_files(project *pp, string_ty *filename, string_list_ty *modifier)
{
    change::pointer cp;

    cp = change_bogus(pp);
    get_change_files(cp, filename, modifier);
    change_free(cp);
}


// vim: set ts=8 sw=4 et :
