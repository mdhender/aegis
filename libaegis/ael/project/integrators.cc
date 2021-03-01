//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2001-2008 Peter Miller
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

#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/ael/change/inappropriat.h>
#include <libaegis/ael/column_width.h>
#include <libaegis/ael/project/integrators.h>
#include <libaegis/change/identifier.h>
#include <libaegis/col.h>
#include <libaegis/option.h>
#include <libaegis/output.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/user.h>


void
list_integrators(change_identifier &cid, string_list_ty *)
{
    output::pointer login_col;
    output::pointer name_col;
    string_ty       *line1;
    int             left;

    trace(("list_integrators()\n{\n"));
    if (cid.set())
        list_change_inappropriate();

    //
    // create the columns
    //
    col::pointer colp = col::open((string_ty *)0);
    line1 = str_format("Project \"%s\"", cid.get_pp()->name_get()->str_text);
    colp->title(line1->str_text, "List of Integrators");
    str_free(line1);

    left = 0;
    login_col = colp->create(left, left + LOGIN_WIDTH, "User\n------");
    left += LOGIN_WIDTH + 2;

    if (!option_terse_get())
    {
        name_col = colp->create(left, 0, "Full Name\n-----------");
    }

    //
    // list the project's integrators
    //
    for (int j = 0; ; ++j)
    {
        nstring logname(project_integrator_nth(cid.get_pp(), j));
        if (logname.empty())
            break;
        login_col->fputs(logname);
        if (!option_terse_get())
            name_col->fputs(user_ty::full_name(logname));
        colp->eoln();
    }
    trace(("}\n"));
}
