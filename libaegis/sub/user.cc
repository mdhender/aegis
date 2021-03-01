//
// aegis - project change supervisor
// Copyright (C) 1999, 2001, 2003-2008, 2012 Peter Miller
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

#include <common/sizeof.h>
#include <common/symtab.h>
#include <common/trace.h>
#include <common/wstring/list.h>
#include <libaegis/sub.h>
#include <libaegis/sub/user.h>
#include <libaegis/user.h>


static nstring
qemail(user_ty::pointer up)
{
    return up->get_email_address().quote_shell();
}


static nstring
get_user_name(user_ty::pointer up)
{
    return up->name();
}


static nstring
get_user_full_name(user_ty::pointer up)
{
    return up->full_name();
}


static nstring
get_user_group_name(user_ty::pointer up)
{
    return up->get_group_name();
}


static nstring
get_user_home(user_ty::pointer up)
{
    return up->get_home();
}


static nstring
get_user_email_address(user_ty::pointer up)
{
    return up->get_email_address();
}


struct table_ty
{
    const char      *name;
    sub_user_func_ptr func;
};

static table_ty table[] =
{
    { "quoted_email", qemail },
    { "email", get_user_email_address },
    { "group", get_user_group_name },
    { "home", get_user_home },
    { "login", get_user_name },
    { "name", get_user_full_name },
};

static symtab_ty *stp;


sub_user_func_ptr
sub_user_func(const nstring &name)
{
    if (!stp)
    {
        stp = new symtab_ty(SIZEOF(table));
        for (table_ty *tp = table; tp < ENDOF(table); ++tp)
        {
            string_ty *s = str_from_c(tp->name);
            stp->assign(s, (void *)tp->func);
            str_free(s);
        }
    }
    sub_user_func_ptr result =
        (sub_user_func_ptr)stp->query(name);
    if (!result)
    {
        nstring s(stp->query_fuzzy(name));
        if (!s.empty())
        {
            sub_context_ty *scp = sub_context_new();
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
//      sub_user - the user substitution
//
// SYNOPSIS
//      string_ty *sub_user(wstring_list_ty *arg);
//
// DESCRIPTION
//      The sub_user function implements the user substitution.
//      The user substitution is replaced by the login name of the user
//      who executed the current command.
//
// ARGUMENTS
//      arg     - list of arguments, including the function name as [0]
//
// RETURNS
//      a pointer to a string in dynamic memory;
//      or NULL on error, setting suberr appropriately.
//

wstring
sub_user(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_user()\n{\n"));
    wstring result;
    if (arg.size() == 1)
    {
        user_ty::pointer up = user_ty::create();
        result = wstring(up->name());
    }
    else if (arg.size() == 2)
    {
        nstring s = arg[1].to_nstring();
        sub_user_func_ptr func = sub_user_func(s);
        if (!func)
        {
            scp->error_set(i18n("unknown substitution variant"));
        }
        else
        {
            user_ty::pointer up = user_ty::create();
            s = func(up);
            result = wstring(s);
        }
    }
    else
    {
        scp->error_set(i18n("requires one argument"));
    }
    trace(("return %p;\n", result.get_ref()));
    trace(("}\n"));
    return result;
}


// vim: set ts=8 sw=4 et :
