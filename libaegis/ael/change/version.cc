//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2006 Peter Miller;
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
// MANIFEST: functions to manipulate versions
//

#include <common/ac/stdio.h>

#include <libaegis/ael/change/version.h>
#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <libaegis/option.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/user.h>


void
list_version(string_ty *project_name, long change_number, string_list_ty *args)
{
    project_ty	    *pp;
    cstate_ty       *cstate_data;
    change_ty	    *cp;
    user_ty	    *up;
    string_ty	    *vs;

    //
    // locate project data
    //
    trace(("list_version()\n{\n"));
    if (!project_name)
	project_name = user_default_project();
    else
	project_name = str_copy(project_name);
    pp = project_alloc(project_name);
    str_free(project_name);
    pp->bind_existing();

    //
    // locate user data
    //
    up = user_executing(pp);

    //
    // locate change data
    //
    if (!change_number)
	change_number = user_default_change(up);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    cstate_data = change_cstate_get(cp);
    vs = project_version_short_get(pp);
    if (option_terse_get())
    {
	if (cstate_data->state == cstate_state_being_developed)
	{
	    // ...punctuation?
	    printf
	    (
		"%s.C%3.3ld\n",
		vs->str_text,
		magic_zero_decode(change_number)
	    );
	}
	else
	{
	    // ...punctuation?
	    printf
	    (
		"%s.D%3.3ld\n",
		vs->str_text,
		cstate_data->delta_number
	    );
	}
    }
    else
    {
	//
	// a century should be enough
	// for a while, at least :-)
	//
	int		cy[100];
	int		ncy;
	string_ty	*s;

	printf("version = \"%s\";\n", vs->str_text);
	if (cstate_data->state == cstate_state_being_developed)
	{
	    printf("change_number = %ld;\n", magic_zero_decode(change_number));
	}
	else
	{
	    printf("delta_number = %ld;\n", cstate_data->delta_number);
	}
	s = project_version_previous_get(pp);
	if (s)
	    printf("version_previous = \"%s\";\n", s->str_text);
	change_copyright_years_get(cp, cy, SIZEOF(cy), &ncy);
	if (ncy)
	{
	    int		    j;

	    printf("copyright_years = [");
	    for (j = 0; j < ncy; ++j)
	    {
		if (j)
		    printf(", ");
		printf("%d", cy[j]);
	    }
	    printf("];\n");
	}
    }
    change_free(cp);
    project_free(pp);
    user_free(up);
    trace(("}\n"));
}
