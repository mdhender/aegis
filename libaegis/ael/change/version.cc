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

#include <common/ac/stdio.h>

#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/ael/change/version.h>
#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/identifier.h>
#include <libaegis/option.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/user.h>


void
list_version(change_identifier &cid, string_list_ty *)
{
    trace(("list_version()\n{\n"));
    cstate_ty *cstate_data = cid.get_cp()->cstate_get();
    string_ty *vs = project_version_short_get(cid.get_pp());
    if (option_terse_get())
    {
	if (cstate_data->state == cstate_state_being_developed)
	{
	    // ...punctuation?
	    printf
	    (
		"%s.C%3.3ld\n",
		vs->str_text,
		magic_zero_decode(cid.get_cp()->number)
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
	    printf
            (
                "change_number = %ld;\n",
                magic_zero_decode(cid.get_cp()->number)
            );
	}
	else
	{
	    printf("delta_number = %ld;\n", cstate_data->delta_number);
	}
	s = project_version_previous_get(cid.get_pp());
	if (s)
	    printf("version_previous = \"%s\";\n", s->str_text);
	change_copyright_years_get(cid.get_cp(), cy, SIZEOF(cy), &ncy);
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
    trace(("}\n"));
}
