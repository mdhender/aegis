//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
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
#include <libaegis/sub.h>
#include <libaegis/sub/change/reviewr_list.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <common/wstr/list.h>


wstring
sub_change_reviewer_list(sub_context_ty *scp, const wstring_list &)
{
    trace(("sub_change_reviewer_list()\n{\n"));
    wstring result;
    change::pointer cp = sub_context_change_get(scp);
    if (!cp)
    {
	scp->error_set(i18n("not valid in current context"));
	trace(("}\n"));
	return result;
    }

    //
    // Recapitulate the change's history, tracking review passes,
    // rescinds and failures.
    //
    string_list_ty reviewers;
    change_reviewer_list(cp, reviewers);

    //
    // Turn the list of reviewer names into a single space-separated string.
    //
    nstring s(reviewers.unsplit(0, reviewers.size(), 0));

    //
    // Turn the narrow character string into a wide character string
    // to form the result of the substitution.
    //
    result = wstring(s);

    trace(("return %8.8lX;\n", (long)result.get_ref()));
    trace(("}\n"));
    return result;
}
