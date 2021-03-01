//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2005 Peter Miller;
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
// MANIFEST: implementation of the change_reviewer_list class
//

#include <libaegis/change.h>
#include <common/str_list.h>
#include <common/symtab.h>
#include <common/trace.h>


void
change_reviewer_list(change_ty *cp, string_list_ty &result)
{
    trace(("change_reviewer_list(cp = %08lX, result = &%08lX)\n{\n",
	(long)cp, (long)&result));
    //
    // Recapitulate the change's history, tracking review passes,
    // rescinds and failures.
    //
    cstate_ty *cstate_data = change_cstate_get(cp);
    symtab_ty review_st;
    for (size_t i = 0; i < cstate_data->history->length; ++i)
    {
	cstate_history_ty *hp = cstate_data->history->list[i];
	switch (hp->what)
	{
	case cstate_history_what_develop_end:
	case cstate_history_what_review_fail:
	case cstate_history_what_integrate_fail:
	    // When aede happens, all reviews are null and void.
	    review_st.clear();
	    break;

	case cstate_history_what_review_pass:
            //
            // The last thing in the list will be a review pass state
            // transition, because the history row has been added, but
            // not yet committed.  The return value from this fucntion
            // may cause the hp->what value of the last row to be
            // changed.
	    //
	    // fall through...
	case cstate_history_what_review_pass_2ar:
	case cstate_history_what_review_pass_2br:
	    review_st.assign(hp->who, hp);
	    break;

	case cstate_history_what_review_pass_undo:
	case cstate_history_what_review_pass_undo_2ar:
	    // Reviewer rescinded his blessing
	    review_st.remove(hp->who);
	    break;

	case cstate_history_what_develop_begin:
	case cstate_history_what_develop_begin_undo:
	case cstate_history_what_develop_end_2ai:
	case cstate_history_what_develop_end_2ar:
	case cstate_history_what_develop_end_undo:
	case cstate_history_what_integrate_begin:
	case cstate_history_what_integrate_begin_undo:
	case cstate_history_what_integrate_pass:
	case cstate_history_what_new_change:
	case cstate_history_what_review_begin:
	case cstate_history_what_review_begin_undo:
#ifndef DEBUG
	default:
#endif
	    break;
	}
    }

    //
    // The keys of the symbol table are the reviewers.
    //
    result.clear();
    review_st.keys(&result);
    trace(("}\n"));
}
