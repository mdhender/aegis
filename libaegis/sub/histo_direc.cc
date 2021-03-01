//
//	aegis - project change supervisor
//	Copyright (C) 2001-2007 Peter Miller
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
// MANIFEST: functions to manipulate histo_direcs
//

#include <libaegis/project.h>
#include <libaegis/sub.h>
#include <libaegis/sub/histo_direc.h>
#include <common/trace.h>
#include <common/wstring/list.h>


//
// NAME
//	sub_history_directory - the history_directory substitution
//
// SYNOPSIS
//	wstring_ty *sub_history_directory(wstring_list_ty *arg);
//
// DESCRIPTION
//	The sub_history_directory function implements the
//	history_directory substitution.	 The history_directory
//	substitution is used to insert the absolute path of the project
//	history directory.
//
// ARGUMENTS
//	arg	- list of arguments, including the function name as [0]
//
// RETURNS
//	a pointer to a string in dynamic memory;
//	or NULL on error, setting suberr appropriately.
//

wstring
sub_history_directory(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_history_directory()\n{\n"));
    wstring result;
    if (arg.size() != 1)
    {
	scp->error_set(i18n("requires zero arguments"));
        trace(("}\n"));
        return result;
    }
    project_ty *pp = sub_context_project_get(scp);
    if (!pp)
    {
	scp->error_set(i18n("not valid in current context"));
        trace(("}\n"));
        return result;
    }

    result = wstring(pp->history_path_get());

    trace(("return %8.8lX;\n", (long)result.get_ref()));
    trace(("}\n"));
    return result;
}
