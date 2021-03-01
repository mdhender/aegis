//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2003-2008 Peter Miller
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

#include <libaegis/change/branch.h>
#include <libaegis/project.h>


string_ty *
change_version_get(change::pointer cp)
{
    string_ty       *s1;
    string_ty       *s2;
    cstate_ty       *cstate_data;

    s1 = project_version_short_get(cp->pp);
    cstate_data = cp->cstate_get();
    if (s1->str_length)
    {
	if (cstate_data->state < cstate_state_being_integrated)
	{
	    s2 =
		str_format
		(
		    "%s.C%3.3ld",
		    s1->str_text,
		    cp->number
		);
	}
	else
	{
	    s2 =
		str_format
		(
		    "%s.D%3.3ld",
	    	    s1->str_text,
		    cstate_data->delta_number
		);
	}
    }
    else
    {
	if (cstate_data->state < cstate_state_being_integrated)
	    s2 = str_format("C%3.3ld", cp->number);
	else
	    s2 = str_format("D%3.3ld", cstate_data->delta_number);
    }
    // not not free s1
    return s2;
}
