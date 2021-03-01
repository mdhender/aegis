//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2008 Peter Miller
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

#include <libaegis/change/cstate/improve.h>
#include <common/error.h> // for assert
#include <common/trace.h>
#include <common/uuidentifier.h>


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
	meta_type		*type_p;
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
	    d->branch->reuse_change_numbers = true;
	    d->branch->mask |= cstate_branch_reuse_change_numbers_mask;
	}

	if (!(d->branch->mask &
	    cstate_branch_default_test_regression_exemption_mask))
	{
	    d->branch->default_test_regression_exemption = true;
	    d->branch->mask |=
		cstate_branch_default_test_regression_exemption_mask;
	}
    }
    switch (d->state)
    {
    case cstate_state_awaiting_development:
    case cstate_state_being_developed:
    case cstate_state_awaiting_review:
    case cstate_state_being_reviewed:
    case cstate_state_awaiting_integration:
	break;

    case cstate_state_being_integrated:
    case cstate_state_completed:
	if (!d->delta_uuid)
	{
	    //
            // This is for backwards compatibility.  If anything causes
            // the change set to be written out, this will stick.  On
            // the other hand, if the change set isn't written back, the
            // change will have a different delta_uuid every time.  In
            // some cases users may notice this randomness, but they
            // probably will not.
	    //
	    d->delta_uuid = universal_unique_identifier();
	}
	break;
    }
    trace(("}\n"));
}
