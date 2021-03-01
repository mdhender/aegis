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
// MANIFEST: implementation of the sub_change_develop_list class
//

#include <libaegis/change.h>
#include <common/str_list.h>
#include <libaegis/sub.h>
#include <libaegis/sub/change/develop_list.h>
#include <common/trace.h>
#include <common/wstr/list.h>


wstring_ty *
sub_change_developer_list(sub_context_ty *scp, wstring_list_ty *arg)
{
    trace(("sub_change_developer_list()\n{\n"));
    wstring_ty *result = 0;
    change_ty *cp = sub_context_change_get(scp);
    if (!cp)
    {
	sub_context_error_set(scp, i18n("not valid in current context"));
	trace(("return NULL;\n"));
	trace(("}\n"));
	return 0;
    }

    //
    // Recapitulate the change's history, tracking develop begins.
    //
    cstate_ty *cstate_data = change_cstate_get(cp);
    string_list_ty developer_list;
    for (size_t i = 0; i < cstate_data->history->length; ++i)
    {
	cstate_history_ty *hp = cstate_data->history->list[i];
	switch (hp->what)
	{
	case cstate_history_what_develop_begin:
	case cstate_history_what_develop_end:
	    developer_list.push_back_unique(hp->who);
	    break;

	case cstate_history_what_develop_begin_undo:
	case cstate_history_what_develop_end_2ai:
	case cstate_history_what_develop_end_2ar:
	case cstate_history_what_develop_end_undo:
	case cstate_history_what_integrate_begin:
	case cstate_history_what_integrate_begin_undo:
	case cstate_history_what_integrate_fail:
	case cstate_history_what_integrate_pass:
	case cstate_history_what_new_change:
	case cstate_history_what_review_begin:
	case cstate_history_what_review_begin_undo:
	case cstate_history_what_review_fail:
	case cstate_history_what_review_pass:
	case cstate_history_what_review_pass_2ar:
	case cstate_history_what_review_pass_2br:
	case cstate_history_what_review_pass_undo:
	case cstate_history_what_review_pass_undo_2ar:
	    break;
	}
    }

    //
    // Turn the list of developer names into a single space-separated string.
    //
    string_ty *s = developer_list.unsplit(0, developer_list.size(), 0);

    //
    // Turn the narrow character string into a wide character string
    // to form the result of the substitution.
    //
    result = str_to_wstr(s);
    str_free(s);

    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
