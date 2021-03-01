//
//      aegis - project change supervisor
//      Copyright (C) 2002-2008, 2011, 2012 Peter Miller
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
#include <libaegis/cstate.fmtgen.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>

#include <aecomplete/complete/change/number.h>
#include <aecomplete/complete/private.h>
#include <aecomplete/shell.h>


struct complete_change_number_ty
{
    complete_ty     inherited;
    project      *pp;
    int             mask;
};


static void
destructor(complete_ty *cp)
{
    complete_change_number_ty *this_thing;

    this_thing = (complete_change_number_ty *)cp;
    project_free(this_thing->pp);
}


static void
perform(complete_ty *cop, shell_ty *sh)
{
    complete_change_number_ty *this_thing;
    string_ty       *prefix;
    size_t          j;

    this_thing = (complete_change_number_ty *)cop;
    prefix = shell_prefix_get(sh);
    for (j = 0; ; ++j)
    {
        cstate_ty       *cstate_data;
        long            change_number;
        change::pointer cp;

        if (!project_change_nth(this_thing->pp, j, &change_number))
            break;
        cp = change_alloc(this_thing->pp, change_number);
        change_bind_existing(cp);
        cstate_data = cp->cstate_get();
        if (this_thing->mask & (1 << cstate_data->state))
        {
            string_ty       *name;

            name = str_format("%ld", magic_zero_decode(change_number));
            if (str_leading_prefix(name, prefix))
                shell_emit(sh, name);
            str_free(name);
        }
        change_free(cp);
    }
}


static complete_vtbl_ty vtbl =
{
    destructor,
    perform,
    sizeof(complete_change_number_ty),
    "change number",
};


complete_ty *
complete_change_number(project *pp, int mask)
{
    complete_ty     *result;
    complete_change_number_ty *this_thing;

    result = complete_new(&vtbl);
    this_thing = (complete_change_number_ty *)result;
    this_thing->pp = pp;
    this_thing->mask = mask ? mask : ~0;
    return result;
}


// vim: set ts=8 sw=4 et :
