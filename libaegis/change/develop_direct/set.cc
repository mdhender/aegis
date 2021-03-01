//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2000, 2002-2005 Peter Miller;
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
// MANIFEST: functions to manipulate sets
//

#include <libaegis/arglex2.h>
#include <libaegis/change.h>
#include <common/error.h> // for assert
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>
#include <common/trace.h>


void
change_development_directory_set(change_ty *cp, string_ty *s)
{
    cstate_ty       *cstate_data;

    //
    // To cope with automounters, directories are stored as given,
    // or are derived from the home directory in the passwd file.
    // Within aegis, pathnames have their symbolic links resolved,
    // and any comparison of paths is done on this "system idea"
    // of the pathname.
    //
    // cp->development_directory is resolved
    // cstate_data->development_directory is unresolved
    //
    trace(("change_development_directory_set(cp = %08lX, s = \"%s\")\n{\n",
	(long)cp, s->str_text));

    // may only be applied to non-branches
    // assert(!change_was_a_branch(cp));
    assert(cp->reference_count >= 1);
    if (cp->development_directory_unresolved)
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
    assert(s->str_text[0] == '/');
    cp->top_path_unresolved = str_copy(s);
    cp->top_path_resolved = 0;
    cp->development_directory_unresolved = 0;
    cp->development_directory_resolved = 0;

    //
    // Now set the change appropriately.
    //
    cstate_data = change_cstate_get(cp);
    if (!cstate_data->development_directory)
    {
	string_ty	*dir;

	dir = os_below_dir(project_Home_path_get(cp->pp), s);
	if (dir)
	{
	    if (!dir->str_length)
	    {
		str_free(dir);
		dir = str_from_c(".");
	    }
	    cstate_data->development_directory = str_copy(dir);
	}
	else
	    cstate_data->development_directory = str_copy(s);
	str_free(dir);
    }
    trace(("}\n"));
}
