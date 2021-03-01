//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2008 Peter Miller
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

#include <common/ac/string.h>

#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/ael/change/inappropriat.h>
#include <libaegis/ael/column_width.h>
#include <libaegis/ael/project/inappropriat.h>
#include <libaegis/ael/project/projects.h>
#include <libaegis/change/identifier.h>
#include <libaegis/col.h>
#include <libaegis/option.h>
#include <libaegis/output.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>


void
list_projects(change_identifier &cid, string_list_ty *)
{
    string_list_ty name;
    output::pointer name_col;
    output::pointer dir_col;
    output::pointer desc_col;
    size_t j;
    int left;
    col::pointer colp;

    trace(("list_projects()\n{\n"));
    if (cid.project_set())
	list_project_inappropriate();
    if (cid.set())
	list_change_inappropriate();

    //
    // list the projects
    //
    trace(("mark\n"));
    project_list_get(&name);
    trace(("mark\n"));

    //
    // create the columns
    //
    colp = col::open((string_ty *)0);
    colp->title("List of Projects", (char *)0);

    left = 0;
    name_col = colp->create(left, left + PROJECT_WIDTH, "Project\n---------");
    left += PROJECT_WIDTH + 1;

    if (!option_terse_get())
    {
	dir_col =
	    colp->create
	    (
	       	left,
	       	left + DIRECTORY_WIDTH,
	       	"Directory\n-----------"
	    );
	left += DIRECTORY_WIDTH + 1;

	desc_col = colp->create(left, 0, "Description\n-------------");
    }

    //
    // list each project
    //
    trace(("mark\n"));
    for (j = 0; j < name.nstrings; ++j)
    {
        trace(("j = %d/%d\n", (int)j, (int)name.nstrings));
	project_ty *pp = project_alloc(name.string[j]);
	pp->bind_existing();

        trace(("mark\n"));
	int err = project_is_readable(pp);

	name_col->fputs(project_name_get(pp));
	if (desc_col)
	{
	    if (err != 0)
		desc_col->fputs(strerror(err));
	    else
	    {
		//
		// The development directory of the
		// project change is the one which
		// contains the trunk or branch
		// baseline.
		//
		string_ty *top = project_top_path_get(pp, 0);
		dir_col->fputs(top);

		desc_col->fputs(project_description_get(pp));
	    }
	}
    trace(("mark\n"));
	project_free(pp);
    trace(("mark\n"));
	colp->eoln();
    }
    trace(("}\n"));
}
