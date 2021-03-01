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
#include <common/error.h>


void
change_copyright_year_append(change::pointer cp, int year)
{
    cstate_ty       *cstate_data;
    size_t          j;
    long            *year_p;
    meta_type         *type_p;

    cstate_data = cp->cstate_get();
    if (!cstate_data->copyright_years)
    {
	cstate_data->copyright_years =
	    (cstate_copyright_years_list_ty *)
    	    cstate_copyright_years_list_type.alloc();
    }
    for (j = 0; j < cstate_data->copyright_years->length; ++j)
    {
	if (cstate_data->copyright_years->list[j] == year)
        {
    	    return;
        }
    }
    year_p =
	(long int *)
	cstate_copyright_years_list_type.list_parse
	(
    	    cstate_data->copyright_years,
    	    &type_p
	);
    assert(type_p == &integer_type);
    *year_p = year;
}
