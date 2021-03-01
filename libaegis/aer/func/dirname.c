/*
 *	aegis - project change supervisor
 *	Copyright (C) 1998, 1999 Peter Miller;
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
 * MANIFEST: functions to implement the dirname builtin function
 */

#include <ac/string.h>

#include <aer/expr.h>
#include <aer/func/dirname.h>
#include <aer/value/error.h>
#include <aer/value/string.h>
#include <error.h>
#include <mem.h>
#include <sub.h>


static int
basename_verify(rpt_expr_ty *ep)
{
    return (ep->nchild == 1);
}


static rpt_value_ty *
basename_run(rpt_expr_ty *ep, size_t argc, rpt_value_ty **argv)
{
    rpt_value_ty    *arg;
    rpt_value_ty    *result;
    string_ty       *pathname;
    string_ty       *s;
    char            *cp;

    assert(argc == 1);
    arg = argv[0];
    assert(arg->method->type != rpt_value_type_error);
    arg = rpt_value_stringize(arg);
    if (arg->method->type != rpt_value_type_string)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	rpt_value_free(arg);
	sub_var_set_charstar(scp, "Function", "basename");
	sub_var_set_charstar(scp, "Number", "1");
	sub_var_set_charstar(scp, "Name", argv[0]->method->name);
	s =
	    subst_intl
	    (
	       	scp,
    i18n("$function: argument $number: string value required (was given $name)")
	    );
	sub_context_delete(scp);
	result = rpt_value_error(ep->pos, s);
	str_free(s);
	return result;
    }
    pathname = str_copy(rpt_value_string_query(arg));
    rpt_value_free(arg);

    /*
     * extract the final path element
     */
    cp = strrchr(pathname->str_text, '/');
    if (cp)
	s = str_from_c(cp + 1);
    else
	s = str_copy(pathname);
    str_free(pathname);

    /*
     * build the result
     */
    result = rpt_value_string(s);
    str_free(s);
    return result;
}


rpt_func_ty rpt_func_basename =
{
    "basename",
    1, /* optimizable */
    basename_verify,
    basename_run
};


static int
dirname_verify(rpt_expr_ty *ep)
{
    return (ep->nchild == 1);
}


static rpt_value_ty *
dirname_run(rpt_expr_ty *ep, size_t argc, rpt_value_ty **argv)
{
    rpt_value_ty    *arg;
    rpt_value_ty    *result;
    string_ty       *f;
    string_ty       *d;
    char            *cp;

    assert(argc == 1);
    arg = argv[0];
    assert(arg->method->type != rpt_value_type_error);
    arg = rpt_value_stringize(arg);
    if (arg->method->type != rpt_value_type_string)
    {
	sub_context_ty	*scp;
	string_ty	*s;

	scp = sub_context_new();
	rpt_value_free(arg);
	sub_var_set_charstar(scp, "Function", "dirname");
	sub_var_set_charstar(scp, "Number", "1");
	sub_var_set_charstar(scp, "Name", argv[0]->method->name);
	s =
	    subst_intl
	    (
	       	scp,
    i18n("$function: argument $number: string value required (was given $name)")
	    );
	sub_context_delete(scp);
	result = rpt_value_error(ep->pos, s);
	str_free(s);
	return result;
    }
    f = str_copy(rpt_value_string_query(arg));
    rpt_value_free(arg);

    /*
     * extract the final path element
     */
    cp = strrchr(f->str_text, '/');
    if (cp)
    {
	if (cp == f->str_text)
    	    d = str_from_c("/");
	else
    	    d = str_n_from_c(f->str_text, cp - f->str_text);
    }
    else
	d = str_from_c(".");
    str_free(f);

    /*
     * build the result
     */
    result = rpt_value_string(d);
    str_free(d);
    return result;
}


rpt_func_ty rpt_func_dirname =
{
    "dirname",
    1, /* optimizable */
    dirname_verify,
    dirname_run
};
