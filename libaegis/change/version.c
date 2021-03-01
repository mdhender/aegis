/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001, 2003 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate versions
 */

#include <change/branch.h>
#include <project.h>


string_ty *
change_version_get(change_ty *cp)
{
    string_ty       *s1;
    string_ty       *s2;
    cstate_ty       *cstate_data;

    s1 = project_version_short_get(cp->pp);
    cstate_data = change_cstate_get(cp);
    if (s1->str_length)
    {
	if (cstate_data->state < cstate_state_being_integrated)
	    s2 = str_format("%S.C%3.3ld", s1, cp->number);
	else
	    s2 = str_format("%S.D%3.3ld", s1, cstate_data->delta_number);
    }
    else
    {
	if (cstate_data->state < cstate_state_being_integrated)
	    s2 = str_format("C%3.3ld", cp->number);
	else
	    s2 = str_format("D%3.3ld", cstate_data->delta_number);
    }
    /* not not free s1 */
    return s2;
}
