/*
 *	aegis - project change supervisor
 *	Copyright (C) 2000-2003 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate uconf values
 */

#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <aer/value/list.h>
#include <aer/value/string.h>
#include <aer/value/struct.h>
#include <aer/value/uconf.h>
#include <getpw_cache.h>
#include <os.h>
#include <sub.h>
#include <trace.h>
#include <user.h>


static rpt_value_ty *
build_result(struct passwd *pw)
{
    uconf	    tmp;
    rpt_value_ty    *result;
    string_ty	    *name;
    rpt_value_ty    *value;
    string_ty	    *s;
    user_ty         *up;

    trace(("build_struct()\n{\n"));

    /*
     * Use the same method as other portions of Aegis to establish
     * the user's attributes.  It is *not* as simple as reading the
     * $HOME/.aegisrc file.
     *
     * The side-effect of the aparrently-useless user_email_address()
     * function call is to establish the tmp->email_address field
     * correctly.
     */
    up = user_numeric(0, pw->pw_uid);
    user_email_address(up);
    tmp = user_uconf_get(up);

    /*
     * Convert the data
     */
    result = uconf_type.convert(&tmp);

    /*
     * Insert the user name into the result.
     */
    trace(("name\n"));
    name = str_from_c("name");
    s = str_from_c(pw->pw_name);
    value = rpt_value_string(s);
    str_free(s);
    rpt_value_struct__set(result, name, value);
    str_free(name);
    rpt_value_free(value);

    /*
     * All done.
     */
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static rpt_value_ty *
lookup(rpt_value_ty *lhs, rpt_value_ty *rhs, int lvalue)
{
    rpt_value_ty    *rhs2;
    rpt_value_ty    *result;
    struct passwd   *pw;
    string_ty	    *s;

    trace(("value_uconf::lookup()\n{\n"));
    rhs2 = rpt_value_arithmetic(rhs);
    if (rhs2->method->type == rpt_value_type_integer)
    {
	int		uid;

	uid = rpt_value_integer_query(rhs2);
	pw = getpwuid_cached(uid);
	if (pw)
	    result = build_result(pw);
	else
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_long(scp, "Number", uid);
	    s = subst_intl(scp, i18n("uid $number unknown"));
	    sub_context_delete(scp);
	    result = rpt_value_error((struct rpt_pos_ty *)0, s);
	    str_free(s);
	}
    }
    else
    {
	rpt_value_free(rhs2);
	rhs2 = rpt_value_stringize(rhs);
	if (rhs2->method->type == rpt_value_type_string)
	{
	    string_ty	    *name;

	    name = rpt_value_string_query(rhs2);
	    pw = getpwnam_cached(name);
	    if (pw)
		result = build_result(pw);
	    else
	    {
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set_string(scp, "Name", name);
		s = subst_intl(scp, i18n("user $name unknown"));
		sub_context_delete(scp);
		result = rpt_value_error((struct rpt_pos_ty *)0, s);
		str_free(s);
	    }
	}
	else
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_charstar(scp, "Name1", "user");
	    sub_var_set_charstar(scp, "Name2", rhs->method->name);
	    s = subst_intl(scp, i18n("illegal lookup ($name1[$name2])"));
	    sub_context_delete(scp);
	    result = rpt_value_error((struct rpt_pos_ty *)0, s);
	    str_free(s);
	}
	rpt_value_free(rhs2);
    }
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static rpt_value_ty *
keys(rpt_value_ty *vp)
{
    rpt_value_ty    *result;

    result = rpt_value_list();
    for (;;)
    {
	struct passwd   *pw;
	string_ty       *s;
	rpt_value_ty    *ep;

	pw = getpwent();
	if (!pw)
    	    break;
	s = str_from_c(pw->pw_name);
	ep = rpt_value_string(s);
	str_free(s);
	rpt_value_list_append(result, ep);
	rpt_value_free(ep);
    }
    return result;
}


static rpt_value_ty *
count(rpt_value_ty *vp)
{
    /*
     * See rpt_value_uconf::keys comment.
     * If you change that function, change this function too.
     */
    return rpt_value_integer(0);
}


static const char *
type_of(rpt_value_ty *this_thing)
{
    return "struct";
}


static rpt_value_method_ty method =
{
    sizeof(rpt_value_ty),
    "user",
    rpt_value_type_structure,
    0, /* construct */
    0, /* destruct */
    0, /* arithmetic */
    0, /* stringize */
    0, /* booleanize */
    lookup,
    keys,
    count,
    type_of,
    0, /* undefer */
};


rpt_value_ty *
rpt_value_uconf()
{
    static rpt_value_ty *vp;

    if (!vp)
	vp = rpt_value_alloc(&method);
    return rpt_value_copy(vp);
}
