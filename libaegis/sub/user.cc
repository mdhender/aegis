//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001, 2003, 2004 Peter Miller;
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
// MANIFEST: functions to manipulate users
//

#include <sub.h>
#include <sub/user.h>
#include <symtab.h>
#include <trace.h>
#include <user.h>
#include <wstr.h>
#include <wstr_list.h>


static string_ty *
qemail(user_ty *up)
{
    static string_ty *result;
    string_ty	    *s;

    s = user_email_address(up);
    if (result)
	str_free(result);
    result = str_quote_shell(s);
    return result;
}


struct table_ty
{
    const char	    *name;
    sub_user_func_ptr func;
};

static table_ty table[] =
{
    {"quoted_email", qemail, },
    {"email", user_email_address, },
    {"group", user_group, },
    {"home", user_home, },
    {"login", user_name, },
    {"name", user_name2, },
};

static symtab_ty *stp;


sub_user_func_ptr
sub_user_func(string_ty *name)
{
    table_ty	    *tp;
    string_ty	    *s;
    sub_user_func_ptr result;
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
    result = (sub_user_func_ptr)symtab_query(stp, name);
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
//	sub_user - the user substitution
//
// SYNOPSIS
//	string_ty *sub_user(wstring_list_ty *arg);
//
// DESCRIPTION
//	The sub_user function implements the user substitution.
//	The user substitution is replaced by the login name of the user
//	who executed the current command.
//
// ARGUMENTS
//	arg	- list of arguments, including the function name as [0]
//
// RETURNS
//	a pointer to a string in dynamic memory;
//	or NULL on error, setting suberr appropriately.
//

wstring_ty *
sub_user(sub_context_ty *scp, wstring_list_ty *arg)
{
    string_ty	    *s;
    wstring_ty	    *result;
    user_ty	    *up;
    sub_user_func_ptr func;

    trace(("sub_user()\n{\n"));
    if (arg->nitems == 1)
    {
	up = user_executing(0);
	result = str_to_wstr(user_name(up));
	user_free(up);
    }
    else if (arg->nitems == 2)
    {
	s = wstr_to_str(arg->item[1]);
	func = sub_user_func(s);
	str_free(s);
	if (!func)
	{
	    sub_context_error_set(scp, i18n("unknown substitution variant"));
	    result = 0;
	}
	else
	{
	    up = user_executing(0);
	    s = func(up);
	    result = str_to_wstr(s);
	    user_free(up);
	    // do not str_free(s)
	}
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
