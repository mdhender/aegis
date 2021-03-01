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
// MANIFEST: functions to manipulate baselines
//

#include <project.h>
#include <sub.h>
#include <sub/project/baseline.h>
#include <trace.h>
#include <wstr.h>
#include <wstr_list.h>


//
// NAME
//	sub_baseline - the baseline substitution
//
// SYNOPSIS
//	wstring_ty *sub_baseline(wstring_list_ty *arg);
//
// DESCRIPTION
//	The sub_baseline function implements the baseline substitution.
//	The baseline substitution is used to insert the absolute path
//	of the baseline.
//
// ARGUMENTS
//	arg	- list of arguments, including the function name as [0]
//
// RETURNS
//	a pointer to a string in dynamic memory;
//	or NULL on error, setting suberr appropriately.
//

wstring_ty *
sub_baseline(sub_context_ty *scp, wstring_list_ty *arg)
{
    wstring_ty	    *result;
    project_ty	    *pp;

    trace(("sub_baseline()\n{\n"));
    if (arg->nitems != 1)
    {
	sub_context_error_set(scp, i18n("requires zero arguments"));
	result = 0;
	goto done;
    }
    pp = sub_context_project_get(scp);
    if (!pp)
    {
	sub_context_error_set(scp, i18n("not valid in current context"));
	result = 0;
	goto done;
    }

    result = str_to_wstr(project_baseline_path_get(pp, 0));

    done:
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
