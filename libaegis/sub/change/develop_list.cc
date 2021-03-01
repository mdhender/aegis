//
//      aegis - project change supervisor
//      Copyright (C) 2004-2008, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/nstring/list.h>
#include <common/trace.h>
#include <common/wstring/list.h>
#include <libaegis/change.h>
#include <libaegis/sub.h>
#include <libaegis/sub/change/develop_list.h>


wstring
sub_change_developer_list(sub_context_ty *scp, const wstring_list &)
{
    trace(("sub_change_developer_list()\n{\n"));
    wstring result;
    change::pointer cp = sub_context_change_get(scp);
    if (!cp)
    {
        scp->error_set(i18n("not valid in current context"));
        trace(("}\n"));
        return result;
    }

    //
    // Recapitulate the change's history, tracking develop begins.
    //
    cstate_ty *cstate_data = cp->cstate_get();
    nstring_list developer_list;
    for (size_t i = 0; i < cstate_data->history->length; ++i)
    {
        cstate_history_ty *hp = cstate_data->history->list[i];
        switch (hp->what)
        {
        case cstate_history_what_develop_begin:
        case cstate_history_what_develop_end:
            developer_list.push_back_unique(nstring(hp->who));
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
    nstring s = developer_list.unsplit();

    //
    // Turn the narrow character string into a wide character string
    // to form the result of the substitution.
    //
    result = wstring(s);

    trace(("return %p;\n", result.get_ref()));
    trace(("}\n"));
    return result;
}


// vim: set ts=8 sw=4 et :
