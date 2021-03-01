//
//	aegis - project change supervisor
//	Copyright (C) 2002-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <aecomplete/complete/private.h>
#include <aecomplete/complete/project/name.h>
#include <libaegis/gonzo.h>
#include <libaegis/project.h>
#include <aecomplete/shell.h>
#include <common/str_list.h>


static void
destructor(complete_ty *)
{
}


static void
perform(complete_ty *, shell_ty *sh)
{
    string_list_ty  list;
    string_ty       *prefix;
    size_t          j;
    string_list_ty  list2;

    project_list_get(&list);
    gonzo_alias_list(&list2);
    list.push_back_unique(list2);
    prefix = shell_prefix_get(sh);
    for (j = 0; j < list.nstrings; ++j)
    {
	if (str_leading_prefix(list.string[j], prefix))
	    shell_emit(sh, list.string[j]);
    }
}


static complete_vtbl_ty vtbl =
{
    destructor,
    perform,
    sizeof(complete_ty),
    "project name",
};


complete_ty *
complete_project_name(void)
{
    return complete_new(&vtbl);
}
