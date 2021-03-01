//
//	aegis - project change supervisor
//	Copyright (C) 2002-2004 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate names
//

#include <complete/private.h>
#include <complete/project/name.h>
#include <gonzo.h>
#include <project.h>
#include <shell.h>
#include <str_list.h>


static void
destructor(complete_ty *cp)
{
}


static void
perform(complete_ty *cp, shell_ty *sh)
{
    string_list_ty  list;
    string_ty       *prefix;
    size_t          j;
    string_list_ty  list2;

    project_list_get(&list);
    gonzo_alias_list(&list2);
    string_list_append_list_unique(&list, &list2);
    string_list_destructor(&list2);
    prefix = shell_prefix_get(sh);
    for (j = 0; j < list.nstrings; ++j)
    {
	if (str_leading_prefix(list.string[j], prefix))
	    shell_emit(sh, list.string[j]);
    }
    string_list_destructor(&list);
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
