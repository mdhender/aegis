/*
 *	aegis - project change supervisor
 *	Copyright (C) 2000-2002 Peter Miller;
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


static rpt_value_ty *build_result _((struct passwd *));

static rpt_value_ty *
build_result(pw)
    struct passwd   *pw;
{
    string_ty	    *path;
    int		    err;
    uconf	    tmp;
    rpt_value_ty    *result;
    string_ty	    *name;
    rpt_value_ty    *value;
    string_ty	    *s;
    user_ty         *up;

    trace(("build_struct()\n{\n"));
    path = str_format("%s/.aegisrc", pw->pw_dir);

    up = user_numeric(0, pw->pw_uid);

    /*
     * Read the file if it exists and is readable,
     * otherwise default lots of things.
     */
    os_become_orig();
    err = os_readable(path);
    if (err)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_errno_setx(scp, err);
	sub_var_set_string(scp, "File_Name", path);
	str_free(path);
	s = subst_intl(scp, "stat $filename: $errno");
	sub_context_delete(scp);
	result = rpt_value_error((void *)0, s);
	str_free(s);
	os_become_undo();
	goto done;
    }
    tmp = uconf_read_file(path);
    os_become_undo();
    if (!tmp->email_address)
      tmp->email_address = user_email_address(up);
    str_free(path);

    /*
     * Convert the data
     */
    result = uconf_type.convert(&tmp);
    uconf_type.free(tmp);

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
    done:
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static rpt_value_ty *lookup _((rpt_value_ty *, rpt_value_ty *, int));

static rpt_value_ty *
lookup(lhs, rhs, lvalue)
    rpt_value_ty    *lhs;
    rpt_value_ty    *rhs;
    int		    lvalue;
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


static rpt_value_ty *keys _((rpt_value_ty *));

static rpt_value_ty *
keys(vp)
    rpt_value_ty    *vp;
{
    rpt_value_ty    *result;

    /*
     * We always return the empty list.	 I suppose we could read the
     * passwd file and return that as the list.	 Or worse, I could
     * go through looking for .aegisrc files in each home directory,
     * and return that as the list.  There doesn't seem to be a real
     * need, so only implement something if folks yelp.
     */
    result = rpt_value_list();
    return result;
}


static rpt_value_ty *count _((rpt_value_ty *));

static rpt_value_ty *
count(vp)
    rpt_value_ty    *vp;
{
    /*
     * See rpt_value_uconf::keys comment.
     * If you change that function, change this function too.
     */
    return rpt_value_integer(0);
}


static char *type_of _((rpt_value_ty *));

static char *
type_of(this)
    rpt_value_ty    *this;
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
