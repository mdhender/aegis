//
// aegis - project change supervisor
// Copyright (C) 1999, 2002-2008, 2011, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/ctype.h>

#include <common/language.h>
#include <common/sizeof.h>
#include <common/symtab.h>
#include <common/trace.h>
#include <common/wstring/list.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/sub.h>
#include <libaegis/sub/project.h>

typedef nstring (*func_ptr)(project *);
struct table_ty
{
    const char      *name;
    func_ptr        func;
};


static nstring
trunk_name_get(project *pp)
{
    return project_name_get(pp->trunk_get());
}


static nstring
trunk_description_get(project *pp)
{
    nstring s = project_description_get(pp->trunk_get());
    const char *cp = s.c_str();
    language_human();
    while (isspace(*cp))
        ++cp;
    const char *ep = cp;
    while (*ep != '\n')
        ++ep;
    while (ep > cp && isspace(ep[-1]))
        --ep;
    language_C();
    return nstring(cp, ep - cp);
}


static table_ty table[] =
{
    {"name", project_name_get, },
    {"description", project_description_get, },
    {"trunk_name", trunk_name_get, },
    {"trunk_description", trunk_description_get, },
    {"version", project_version_short_get },
    {"version_long", project_version_get },
};

static symtab_ty *stp;


static func_ptr
find_func(const nstring &name)
{
    if (!stp)
    {
        stp = new symtab_ty(SIZEOF(table));
        for (const table_ty *tp = table; tp < ENDOF(table); ++tp)
        {
            nstring s(tp->name);
            stp->assign(s, (void *)tp->func);
        }
    }
    func_ptr result = (func_ptr)stp->query(name);
    if (!result)
    {
        nstring guess = stp->query_fuzzy(name);
        if (guess)
        {
            sub_context_ty sc;
            sc.var_set_string("Name", name);
            sc.var_set_string("Guess", guess);
            sc.error_intl(i18n("no \"$name\", guessing \"$guess\""));
            result = (func_ptr)stp->query(guess);
        }
    }
    return result;
}


//
// NAME
//      sub_project - the project substitution
//
// SYNOPSIS
//      string_ty *sub_project(wstring_list_ty *arg);
//
// DESCRIPTION
//      The sub_project function implements the project substitution.
//      The project substitution is replaced by the project name.
//
// ARGUMENTS
//      arg     - list of arguments, including the function name as [0]
//
// RETURNS
//      a pointer to a string in dynamic memory;
//      or NULL on error, setting suberr appropriately.
//

wstring
sub_project(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_project()\n{\n"));
    wstring result;
    project *pp = sub_context_project_get(scp);
    if (!pp)
    {
        scp->error_set(i18n("not valid in current context"));
        trace(("}\n"));
        return result;
    }
    if (arg.size() == 1)
        result = wstring(project_name_get(pp));
    else if (arg.size() == 2)
    {
        nstring s = arg[1].to_nstring();
        func_ptr func = find_func(s);
        if (!func)
        {
            scp->error_set(i18n("unknown substitution variant"));
            trace(("}\n"));
            return result;
        }
        result = wstring(func(pp));
    }
    else
    {
        scp->error_set(i18n("requires one argument"));
        trace(("}\n"));
        return result;
    }
    trace(("return %p;\n", result.get_ref()));
    trace(("}\n"));
    return result;
}


// vim: set ts=8 sw=4 et :
