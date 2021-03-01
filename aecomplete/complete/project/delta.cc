//
//      aegis - project change supervisor
//      Copyright (C) 2002-2006, 2008, 2011, 2012 Peter Miller
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

#include <aecomplete/complete/private.h>
#include <aecomplete/complete/project/delta.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <aecomplete/shell.h>
#include <common/str_list.h>


struct complete_project_delta_ty
{
    complete_ty     inherited;
    project      *pp;
};


static void
destructor(complete_ty *cp)
{
    complete_project_delta_ty *this_thing;

    this_thing = (complete_project_delta_ty *)cp;
    project_free(this_thing->pp);
}


static void
perform(complete_ty *cp, shell_ty *sh)
{
    complete_project_delta_ty *this_thing;
    string_ty       *prefix;
    size_t          j;

    this_thing = (complete_project_delta_ty *)cp;
    prefix = shell_prefix_get(sh);
    for (j = 0; ; ++j)
    {
        long            cn;
        long            dn;
        string_list_ty  name;
        size_t          k;
        string_ty       *s;

        if (!project_history_nth(this_thing->pp, j, &cn, &dn, &name))
            break;

        s = str_format("%ld", dn);
        if (str_leading_prefix(s, prefix))
            shell_emit(sh, s);
        str_free(s);

        for (k = 0; k < name.nstrings; ++k)
        {
            s = name.string[k];
            if (str_leading_prefix(s, prefix))
                shell_emit(sh, s);
        }
    }
}


static complete_vtbl_ty vtbl =
{
    destructor,
    perform,
    sizeof(complete_project_delta_ty),
    "project delta",
};


complete_ty *
complete_project_delta(project *pp)
{
    complete_ty     *result;
    complete_project_delta_ty *this_thing;

    result = complete_new(&vtbl);
    this_thing = (complete_project_delta_ty *)result;
    this_thing->pp = pp;
    return result;
}


// vim: set ts=8 sw=4 et :
