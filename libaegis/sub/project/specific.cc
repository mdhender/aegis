//
//	aegis - project change supervisor
//	Copyright (C) 2001-2005 Peter Miller;
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
// MANIFEST: functions to manipulate specifics
//

#include <common/ac/string.h>

#include <common/error.h>
#include <common/trace.h>
#include <common/wstr.h>
#include <common/wstr/list.h>
#include <libaegis/attribute.h>
#include <libaegis/change.h>
#include <libaegis/pconf.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>
#include <libaegis/sub/project/specific.h>


static string_ty *
pconf_project_specific_find(pconf_ty *pconf_data, string_ty *name)
{
    assert(pconf_data);
    assert(name);
    attributes_ty *psp =
	attributes_list_find(pconf_data->project_specific, name->str_text);
    if (psp)
    {
	assert(psp->value);
	return psp->value;
    }
    return 0;
}


//
// NAME
//	sub_project_specific - the project_specific substitution
//
// SYNOPSIS
//	string_ty *sub_project_specific(wstring_list_ty *arg);
//
// DESCRIPTION
//	The sub_project_specific function implements the project_specific
//	substitution.  The project_specific substitution is replaced
//	by the corresponding entry in the project-specific array in the
//	project config file.
//
// ARGUMENTS
//	arg	- list of arguments, including the function name as [0]
//
// RETURNS
//	a pointer to a string in dynamic memory;
//	or NULL on error, setting suberr appropriately.
//

wstring_ty *
sub_project_specific(sub_context_ty *scp, wstring_list_ty *arg)
{
    string_ty	    *name;
    string_ty	    *value;
    wstring_ty	    *result;
    change_ty	    *cp;
    pconf_ty        *pconf_data;

    trace(("sub_project_specific()\n{\n"));
    cp = sub_context_change_get(scp);
    if (!cp)
    {
	project_ty *pp = sub_context_project_get(scp);
	if (!pp)
	{
	    sub_context_error_set(scp, i18n("not valid in current context"));
	    trace(("return NULL;\n"));
	    trace(("}\n"));
	    result = 0;
	    goto done;
	}
	cp = pp->change_get();
    }
    if (arg->size() != 2)
    {
	sub_context_error_set(scp, i18n("requires one argument"));
	result = 0;
	goto done;
    }

    name = wstr_to_str(arg->get(1));
    pconf_data = change_pconf_get(cp, 0);
    value = pconf_project_specific_find(pconf_data, name);
    str_free(name);
    if (!value)
    {
	sub_context_error_set(scp, i18n("unknown substitution variant"));
	result = 0;
	goto done;
    }

    result = str_to_wstr(value);

    done:
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
