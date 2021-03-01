//
//	aegis - project change supervisor
//	Copyright (C) 2000, 2002-2008 Peter Miller
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

#include <common/error.h> // for assert
#include <common/trace.h>
#include <libaegis/arglex2.h>
#include <libaegis/change.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>


void
change_top_path_set(change::pointer cp, string_ty *s)
{
    change_top_path_set(cp, nstring(s));
}

void
change_top_path_set(change::pointer cp, const nstring  &s)
{
    cstate_ty       *cstate_data;

    //
    // To cope with automounters, directories are stored as given,
    // or are derived from the home directory in the passwd file.
    // Within aegis, pathnames have their symbolic links resolved,
    // and any comparison of paths is done on this "system idea"
    // of the pathname.
    //
    // cp->top_path_resolved is resolved
    // cp->top_path_unresolved is unresolved
    // cstate_data->development_directory is unresolved
    //
    trace(("change_top_path_set(cp = %08lX, s = \"%s\")\n{\n",
	(long)cp, s.c_str()));

    // May only be applied to new branches
    // assert(change_was_a_branch(cp));
    assert(cp->reference_count >= 1);
    if (cp->top_path_unresolved)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_charstar
	(
	    scp,
	    "Name",
	    arglex_token_name(arglex_token_directory)
	);
	fatal_intl(scp, i18n("duplicate $name option"));
	// NOTREACHED
	sub_context_delete(scp);
    }
    assert(s[0] == '/');
    cp->top_path_unresolved = str_copy(s.get_ref());
    cp->development_directory_resolved = 0;

    //
    // Now set the branch's development_directory atribute, which
    // points at top_path, not top_path/baseline.
    //
    // assert(change_was_a_branch(cp));
    cstate_data = cp->cstate_get();
    if (!cstate_data->development_directory)
    {
	nstring         dir;
        nstring         home = nstring(project_Home_path_get(cp->pp));

	dir = os_below_dir(os_canonify_dirname(home), s);
        trace_nstring(dir);
	if (!dir.empty())
	    cstate_data->development_directory = str_copy(dir.get_ref());
	else
	    cstate_data->development_directory = str_copy(s.get_ref());
    }
    trace(("}\n"));
}
