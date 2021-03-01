//
//	aegis - project change supervisor
//	Copyright (C) 1997, 1999, 2004, 2005 Peter Miller;
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
// MANIFEST: functions to implement the builtin gettime function
//

#include <libaegis/aer/expr.h>
#include <libaegis/aer/func/gettime.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/aer/value/time.h>
#include <common/error.h>
#include <common/gettime.h>
#include <libaegis/sub.h>


static int
verify(rpt_expr_ty *ep)
{
    return (ep->nchild == 1);
}


static rpt_value_ty *
run(rpt_expr_ty *ep, size_t argc, rpt_value_ty **argv)
{
    sub_context_ty  *scp;
    time_t          t;
    rpt_value_ty    *tmp;
    string_ty       *s;
    rpt_value_ty    *result;

    //
    // See if it looks like a number.
    // Use that if so.
    //
    assert(argc == 1);
    tmp = rpt_value_integerize(argv[0]);
    if (tmp->method->type == rpt_value_type_integer)
    {
	t = rpt_value_integer_query(tmp);
	rpt_value_free(tmp);
	result = rpt_value_time(t);
	return result;
    }
    rpt_value_free(tmp);

    //
    // Coerce the argument to a string.
    // It is an error if it can't be.
    //
    tmp = rpt_value_stringize(argv[0]);
    if (tmp->method->type != rpt_value_type_string)
    {
	rpt_value_free(tmp);
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Function", "gettime");
	sub_var_set_long(scp, "Number", 1);
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

    //
    // Scan the string and try to make a time out of it.
    // It is an error if this can't be done.
    //
    s = rpt_value_string_query(tmp);
    t = date_scan(s->str_text);
    if (t == (time_t)-1)
    {
	rpt_value_free(tmp);
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Function", "gettime");
	sub_var_set_long(scp, "Number", 1);
	s =
	    subst_intl
	    (
	       	scp,
	  i18n("$function: argument $number: cannot convert string into a time")
	    );
	sub_context_delete(scp);
	result = rpt_value_error(ep->pos, s);
	str_free(s);
	return result;
    }

    //
    // build the return value
    //
    rpt_value_free(tmp);
    result = rpt_value_time(t);
    return result;
}


rpt_func_ty rpt_func_gettime =
{
    "gettime",
    1, // optimizable
    verify,
    run,
};

rpt_func_ty rpt_func_mktime =
{
    "mktime",
    1, // optimizable
    verify,
    run,
};
