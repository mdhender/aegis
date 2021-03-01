//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2004 Peter Miller;
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
// MANIFEST: functions to manipulate improves
//

#include <change/cstate/improve.h>
#include <error.h> // for assert
#include <trace.h>


void
change_cstate_improve(cstate_ty *d)
{
    trace(("improve(d = %8.8lX)\n{\n", (long)d));
    if (!d->history)
    {
	d->history = (cstate_history_list_ty *)cstate_history_list_type.alloc();
    }
    assert(!d->src);
    if (!(d->mask & cstate_regression_test_exempt_mask))
    {
	d->regression_test_exempt =
	    (boolean_ty)
    	    (
       		d->cause != change_cause_internal_improvement
    	    &&
       		d->cause != change_cause_external_improvement
    	    );
    }
    if (!d->architecture)
    {
	d->architecture =
	    (cstate_architecture_list_ty *)
            cstate_architecture_list_type.alloc();
    }
    if (!d->architecture->length)
    {
	type_ty		*type_p;
	string_ty	**str_p;

	str_p =
	    (string_ty **)
            cstate_architecture_list_type.list_parse(d->architecture, &type_p);
	assert(type_p == &string_type);
	*str_p = str_from_c("unspecified");
    }
    if (d->branch)
    {
	if (!(d->branch->mask & cstate_branch_umask_mask))
	    d->branch->umask = DEFAULT_UMASK;
	d->branch->umask = (d->branch->umask & 5) | 022;
	if (d->branch->umask == 023)
	    d->branch->umask = 022;
	// 022, 026 and 027 are OK

	if (!(d->branch->mask & cstate_branch_reuse_change_numbers_mask))
	{
	    d->branch->reuse_change_numbers = (boolean_ty)1;
	    d->branch->mask |= cstate_branch_reuse_change_numbers_mask;
	}
    }
    trace(("}\n"));
}
