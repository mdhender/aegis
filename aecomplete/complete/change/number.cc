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
// MANIFEST: functions to manipulate numbers
//

#include <change.h>
#include <complete/change/number.h>
#include <complete/private.h>
#include <cstate.h>
#include <project.h>
#include <project/history.h>
#include <shell.h>


typedef struct complete_change_number_ty complete_change_number_ty;
struct complete_change_number_ty
{
    complete_ty     inherited;
    project_ty      *pp;
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
	change_ty       *cp;

	if (!project_change_nth(this_thing->pp, j, &change_number))
	    break;
	cp = change_alloc(this_thing->pp, change_number);
	change_bind_existing(cp);
	cstate_data = change_cstate_get(cp);
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
complete_change_number(project_ty *pp, int mask)
{
    complete_ty     *result;
    complete_change_number_ty *this_thing;

    result = complete_new(&vtbl);
    this_thing = (complete_change_number_ty *)result;
    this_thing->pp = pp;
    this_thing->mask = mask ? mask : ~0;
    return result;
}
