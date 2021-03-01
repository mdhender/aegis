//
//	aegis - project change supervisor
//	Copyright (C) 2003, 2004 Peter Miller;
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
// MANIFEST: functions to manipulate projects
//

#include <ac/string.h>

#include <get/change/list.h>
#include <get/project.h>
#include <get/project/aedist.h>
#include <get/project/file.h>
#include <get/project/files.h>
#include <get/project/menu.h>
#include <get/project/staff.h>
#include <get/project/statistics.h>
#include <http.h>
#include <project.h>
#include <str_list.h>


struct table_ty
{
    const char *name;
    void (*action)(project_ty *, string_ty *, string_list_ty *);
};

static const table_ty table[] =
{
    { "aedist",     get_project_aedist     },
    { "changes",    get_change_list        },
    { "files",      get_project_files      },
    { "menu",       get_project_menu       },
    { "staff",      get_project_staff      },
    { "statistics", get_project_statistics },
};


void
get_project(project_ty *pp, string_ty *filename, string_list_ty *modifier)
{
    if (modifier->nstrings && 0 == strcmp(filename->str_text, "."))
    {
	const table_ty  *tp;
	string_ty       *name;

	//
	// To remove ambiguity at the root level, sometimes you have
	// to say "@@file@history" to distinguish the request from
	// "@@project@history"
	//
	name = modifier->string[0];
	if
	(
	    0 != strcasecmp(name->str_text, "file")
	&&
	    0 != strcasecmp(name->str_text, "change")
	)
	{
	    if
	    (
		modifier->nstrings >= 2
	    &&
		0 == strcasecmp(name->str_text, "project")
	    )
		name = modifier->string[1];
	    for (tp = table; tp < ENDOF(table); ++tp)
	    {
		if (0 == strcasecmp(tp->name, name->str_text))
		{
		    tp->action(pp, filename, modifier);
		    return;
		}
	    }
	}
    }
    get_project_file(pp, filename, modifier);
}
