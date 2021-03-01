//
//      aegis - project change supervisor
//      Copyright (C) 2001-2008, 2011, 2012 Peter Miller
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

#include <common/ac/assert.h>
#include <common/ac/string.h>

#include <common/trace.h>
#include <common/wstring/list.h>
#include <libaegis/attribute.h>
#include <libaegis/change.h>
#include <libaegis/pconf.fmtgen.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>
#include <libaegis/sub/project/specific.h>


static nstring
pconf_project_specific_find(pconf_ty *pconf_data, const nstring &name)
{
    assert(pconf_data);
    attributes_ty *psp =
        attributes_list_find(pconf_data->project_specific, name.c_str());
    if (!psp)
        return "";
    assert(psp->value);
    return nstring(psp->value);
}


//
// NAME
//      sub_project_specific - the project_specific substitution
//
// SYNOPSIS
//      string_ty *sub_project_specific(wstring_list_ty *arg);
//
// DESCRIPTION
//      The sub_project_specific function implements the project_specific
//      substitution.  The project_specific substitution is replaced
//      by the corresponding entry in the project-specific array in the
//      project config file.
//
// ARGUMENTS
//      arg     - list of arguments, including the function name as [0]
//
// RETURNS
//      a pointer to a string in dynamic memory;
//      or NULL on error, setting suberr appropriately.
//

wstring
sub_project_specific(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_project_specific()\n{\n"));
    wstring result;
    change::pointer cp = sub_context_change_get(scp);
    if (!cp)
    {
        project *pp = sub_context_project_get(scp);
        if (!pp)
        {
            scp->error_set(i18n("not valid in current context"));
            trace(("}\n"));
            return result;
        }
        cp = pp->change_get();
    }
    if (arg.size() != 2)
    {
        scp->error_set(i18n("requires one argument"));
        trace(("}\n"));
        return result;
    }

    nstring name = arg[1].to_nstring();
    pconf_ty *pconf_data = change_pconf_get(cp, 0);
    nstring value(pconf_project_specific_find(pconf_data, name));
    result = wstring(value);
    trace(("return %p;\n", result.get_ref()));
    trace(("}\n"));
    return result;
}


// vim: set ts=8 sw=4 et :
