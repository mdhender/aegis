//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2001-2007 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate reviewerss
//

#include <libaegis/ael/change/inappropriat.h>
#include <libaegis/ael/column_width.h>
#include <libaegis/ael/project/reviewers.h>
#include <libaegis/col.h>
#include <libaegis/option.h>
#include <libaegis/output.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/user.h>


void
list_reviewers(string_ty *project_name, long change_number, string_list_ty *)
{
    project_ty      *pp;
    output_ty       *login_col = 0;
    output_ty       *name_col = 0;
    int             j;
    string_ty       *line1;
    int             left;
    col             *colp;

    trace(("list_reviewers()\n{\n"));
    if (change_number)
        list_change_inappropriate();

    //
    // locate project data
    //
    if (!project_name)
    {
        nstring n = user_ty::create()->default_project();
	project_name = str_copy(n.get_ref());
    }
    else
        project_name = str_copy(project_name);
    pp = project_alloc(project_name);
    str_free(project_name);
    pp->bind_existing();

    //
    // create the columns
    //
    colp = col::open((string_ty *) 0);
    line1 = str_format("Project \"%s\"", project_name_get(pp)->str_text);
    colp->title(line1->str_text, "List of Reviewers");
    str_free(line1);

    left = 0;
    login_col = colp->create(left, left + LOGIN_WIDTH, "User\n------");
    left += LOGIN_WIDTH + 2;

    if (!option_terse_get())
    {
        name_col = colp->create(left, 0, "Full Name\n-----------");
    }

    //
    // list the project's reviewers
    //
    for (j = 0;; ++j)
    {
        nstring logname(project_reviewer_nth(pp, j));
        if (logname.empty())
            break;
        login_col->fputs(logname);
        if (!option_terse_get())
            name_col->fputs(user_ty::full_name(logname));
        colp->eoln();
    }

    //
    // clean up and go home
    //
    delete colp;
    project_free(pp);
    trace(("}\n"));
}
