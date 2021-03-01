//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003-2008 Peter Miller
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

#include <libaegis/change.h>
#include <common/error.h> // for assert
#include <common/trace.h>


cstate_architecture_times_ty *
change_architecture_times_find(change::pointer cp, string_ty *un)
{
    cstate_ty       *cstate_data;
    size_t          j;
    cstate_architecture_times_ty *tp;

    //
    // find this variant in the times list
    //
    trace(("change_architecture_times_find(cp = %8.8lX, un = %8.8lX)\n{\n",
	(long)cp, (long)un));
    assert(cp->reference_count >= 1);
    cstate_data = cp->cstate_get();
    if (!cstate_data->architecture_times)
	cstate_data->architecture_times =
	    (cstate_architecture_times_list_ty *)
    	    cstate_architecture_times_list_type.alloc();
    for (j = 0; j < cstate_data->architecture_times->length; ++j)
    {
	if (str_equal(un, cstate_data->architecture_times->list[j]->variant))
	    break;
    }
    if (j >= cstate_data->architecture_times->length)
    {
	meta_type		*type_p;
	cstate_architecture_times_ty **data_p;

	data_p =
	    (cstate_architecture_times_ty **)
	    cstate_architecture_times_list_type.list_parse
	    (
	       	cstate_data->architecture_times,
	       	&type_p
	    );
	assert(type_p == &cstate_architecture_times_type);
	tp =
	    (cstate_architecture_times_ty *)
            cstate_architecture_times_type.alloc();
	*data_p = tp;
	tp->variant = str_copy(un);
    }
    else
	    tp = cstate_data->architecture_times->list[j];
    trace(("return %8.8lX;\n", (long)tp));
    trace(("}\n"));
    return tp;
}
