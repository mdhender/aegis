//
//	aegis - project change supervisor
//	Copyright (C) 1998, 1999, 2001, 2002, 2004 Peter Miller;
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
// MANIFEST: functions to implement the subst builtin function
//

#include <aer/expr.h>
#include <aer/func/substitute.h>
#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <aer/value/string.h>
#include <error.h>
#include <mem.h>
#include <regula_expre.h>
#include <sub.h>
#include <trace.h>


static int
verify(rpt_expr_ty *ep)
{
    return (ep->nchild == 3 || ep->nchild == 4);
}


static rpt_value_ty *
run(rpt_expr_ty	*ep, size_t argc, rpt_value_ty **argv)
{
    trace(("rpt_func_substitute::run()\n{\n"));
    //
    // Get the match pattern.
    //
    rpt_value_ty *arg = argv[0];
    assert(arg->method->type != rpt_value_type_error);
    arg = rpt_value_stringize(arg);
    if (arg->method->type != rpt_value_type_string)
    {
	sub_context_ty sc(__FILE__, __LINE__);
	rpt_value_free(arg);
	sc.var_set_charstar("Function", "subst");
	sc.var_set_long("Number", 1);
	sc.var_set_charstar("Name", argv[0]->method->name);
	string_ty *s =
	    sc.subst_intl
	    (
    i18n("$function: argument $number: string value required (was given $name)")
	    );
	rpt_value_ty *result = rpt_value_error(ep->pos, s);
	str_free(s);
	trace(("}\n"));
	return result;
    }
    nstring lhs(str_copy(rpt_value_string_query(arg)));
    rpt_value_free(arg);

    //
    // Get the replacement pattern.
    //
    arg = argv[1];
    assert(arg->method->type != rpt_value_type_error);
    arg = rpt_value_stringize(arg);
    if (arg->method->type != rpt_value_type_string)
    {
	sub_context_ty sc(__FILE__, __LINE__);
	rpt_value_free(arg);
	sc.var_set_charstar("Function", "subst");
	sc.var_set_long("Number", 2);
	sc.var_set_charstar("Name", argv[1]->method->name);
	string_ty *s =
	    sc.subst_intl
	    (
    i18n("$function: argument $number: string value required (was given $name)")
	    );
	rpt_value_ty *result = rpt_value_error(ep->pos, s);
	str_free(s);
	trace(("}\n"));
	return result;
    }
    nstring rhs(str_copy(rpt_value_string_query(arg)));
    rpt_value_free(arg);

    //
    // Get the string to be worked over.
    //
    arg = argv[2];
    assert(arg->method->type != rpt_value_type_error);
    arg = rpt_value_stringize(arg);
    if (arg->method->type != rpt_value_type_string)
    {
	sub_context_ty sc(__FILE__, __LINE__);
	rpt_value_free(arg);
	sc.var_set_charstar("Function", "subst");
	sc.var_set_long("Number", 3);
	sc.var_set_charstar("Name", argv[2]->method->name);
	string_ty *s =
	    sc.subst_intl
	    (
    i18n("$function: argument $number: string value required (was given $name)")
	    );
	rpt_value_ty *result = rpt_value_error(ep->pos, s);
	str_free(s);
	trace(("}\n"));
	return result;
    }
    nstring input(str_copy(rpt_value_string_query(arg)));
    rpt_value_free(arg);
    arg = 0;

    //
    // Get the count of how many times to match.
    //
    long count = 0;
    if (argc >= 4)
    {
	arg = argv[3];
	assert(arg->method->type != rpt_value_type_error);
	arg = rpt_value_integerize(arg);
	if (arg->method->type != rpt_value_type_integer)
	{
	    sub_context_ty sc(__FILE__, __LINE__);
	    rpt_value_free(arg);
	    sc.var_set_charstar("Function", "subst");
	    sc.var_set_long("Number", 2);
	    sc.var_set_charstar("Name", argv[3]->method->name);
	    string_ty *s =
		sc.subst_intl
		(
   i18n("$function: argument $number: integer value required (was given $name)")
		);
	    rpt_value_ty *result = rpt_value_error(ep->pos, s);
	    str_free(s);
	    trace(("}\n"));
	    return result;
	}
	count = rpt_value_integer_query(arg);
	rpt_value_free(arg);
    }

    //
    // perform the substitution
    //
    regular_expression re(lhs);
    nstring output;
    if (!re.match_and_substitute(rhs, input, count, output))
    {
	//
        // Error... probably the LHS pattern was erroneous, but it could
        // have been the RHS pattern.
	//
	// Return an error result.
	//
	sub_context_ty sc(__FILE__, __LINE__);
	rpt_value_free(arg);
	sc.var_set_charstar("Function", "subst");
	sc.var_set_long("Number", 1);
	sc.var_set_charstar("Message", re.strerror());
	string_ty *s =
	    sc.subst_intl(i18n("$function: argument $number: $message"));
	rpt_value_ty *result = rpt_value_error(ep->pos, s);
	str_free(s);
	trace(("}\n"));
	return result;
    }

    //
    // build the result
    //
    trace_nstring(output);
    trace(("}\n"));
    return rpt_value_string(output.get_ref());
}


rpt_func_ty rpt_func_substitute =
{
    "subst",
    1, // optimizable
    verify,
    run
};
