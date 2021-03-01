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
// MANIFEST: functions to manipulate projects
//

#include <project.h>
#include <project/history.h>
#include <sub.h>
#include <sub/project.h>
#include <symtab.h>
#include <trace.h>
#include <wstr.h>
#include <wstr_list.h>

typedef string_ty *(*func_ptr)(project_ty *);
struct table_ty
{
    const char      *name;
    func_ptr	    func;
};


static string_ty *
trunk_name_get(project_ty *pp)
{
    while (pp->parent)
	pp = pp->parent;
    return project_name_get(pp);
}


static string_ty *
trunk_description_get(project_ty *pp)
{
    while (pp->parent)
	pp = pp->parent;
    return project_description_get(pp);
}


static table_ty table[] =
{
    {"name", project_name_get, },
    {"description", project_description_get, },
    {"trunk_name", trunk_name_get, },
    {"trunk_description", trunk_description_get, },
};

static symtab_ty *stp;


static func_ptr
find_func(string_ty *name)
{
    table_ty	    *tp;
    string_ty	    *s;
    func_ptr	    result;
    sub_context_ty  *scp;

    if (!stp)
    {
	stp = symtab_alloc(SIZEOF(table));
	for (tp = table; tp < ENDOF(table); ++tp)
	{
	    s = str_from_c(tp->name);
	    symtab_assign(stp, s, (void *)tp->func);
	    str_free(s);
	}
    }
    result = (func_ptr)symtab_query(stp, name);
    if (!result)
    {
	s = symtab_query_fuzzy(stp, name);
	if (s)
	{
	    scp = sub_context_new();
	    sub_var_set_string(scp, "Name", name);
	    sub_var_set_string(scp, "Guess", s);
	    error_intl(scp, i18n("no \"$name\", guessing \"$guess\""));
	    sub_context_delete(scp);
	}
	return 0;
    }
    return result;
}


//
// NAME
//	sub_project - the project substitution
//
// SYNOPSIS
//	string_ty *sub_project(wstring_list_ty *arg);
//
// DESCRIPTION
//	The sub_project function implements the project substitution.
//	The project substitution is replaced by the project name.
//
// ARGUMENTS
//	arg	- list of arguments, including the function name as [0]
//
// RETURNS
//	a pointer to a string in dynamic memory;
//	or NULL on error, setting suberr appropriately.
//

wstring_ty *
sub_project(sub_context_ty *scp, wstring_list_ty *arg)
{
    string_ty	    *s;
    wstring_ty	    *result;
    func_ptr	    func;
    project_ty	    *pp;

    trace(("sub_project()\n{\n"));
    pp = sub_context_project_get(scp);
    if (!pp)
    {
	sub_context_error_set(scp, i18n("not valid in current context"));
	result = 0;
    }
    else if (arg->nitems == 1)
	result = str_to_wstr(project_name_get(pp));
    else if (arg->nitems == 2)
    {
	s = wstr_to_str(arg->item[1]);
	func = find_func(s);
	str_free(s);
	if (!func)
	{
	    sub_context_error_set(scp, i18n("unknown substitution variant"));
	    result = 0;
	}
	else
	    result = str_to_wstr(func(pp));
    }
    else
    {
	sub_context_error_set(scp, i18n("requires one argument"));
	result = 0;
    }
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
