//
//	aegis - project change supervisor
//	Copyright (C) 1995, 1996, 1999, 2003, 2004 Peter Miller;
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
// MANIFEST: functions to implement the builtin wrap function
//

#include <ac/string.h>

#include <aer/expr.h>
#include <aer/func/split.h>
#include <aer/value/error.h>
#include <aer/value/list.h>
#include <aer/value/string.h>
#include <error.h>
#include <sub.h>


static int
verify(rpt_expr_ty *ep)
{
    return (ep->nchild == 2);
}


static rpt_value_ty *
run(rpt_expr_ty *ep, size_t argc, rpt_value_ty **argv)
{
    rpt_value_ty    *a1;
    rpt_value_ty    *a2;
    rpt_value_ty    *result;
    rpt_value_ty    *tmp;
    char            *sp;
    const char      *sep;

    a1 = argv[0];
    assert(a1->method->type != rpt_value_type_error);
    a1 = rpt_value_stringize(a1);
    if (a1->method->type != rpt_value_type_string)
    {
	sub_context_ty	*scp;
	string_ty	*s;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Function", "split");
	sub_var_set_long(scp, "Number", 1);
	sub_var_set_charstar(scp, "Name", a1->method->name);
	rpt_value_free(a1);
	s =
	    subst_intl
	    (
	       	scp,
    i18n("$function: argument $number: string value required (was given $name)")
	    );
	sub_context_delete(scp);
	tmp = rpt_value_error(ep->pos, s);
	str_free(s);
	return tmp;
    }

    a2 = argv[1];
    assert(a2->method->type != rpt_value_type_error);
    a2 = rpt_value_stringize(a2);
    if (a2->method->type != rpt_value_type_string)
    {
	sub_context_ty	*scp;
	string_ty	*s;

	scp = sub_context_new();
	rpt_value_free(a1);
	sub_var_set_charstar(scp, "Function", "split");
	sub_var_set_long(scp, "Number", 2);
	sub_var_set_charstar(scp, "Name", a2->method->name);
	rpt_value_free(a2);
	s =
	    subst_intl
	    (
	       	scp,
    i18n("$function: argument $number: string value required (was given $name)")
	    );
	sub_context_delete(scp);
	tmp = rpt_value_error(ep->pos, s);
	str_free(s);
	return tmp;
    }

    sp = rpt_value_string_query(a1)->str_text;
    sep = rpt_value_string_query(a2)->str_text;
    if (!*sep)
	sep = " \n\r\t\f\b";

    //
    // the result is a list
    // create an empty one so we can start filling it
    //
    result = rpt_value_list();

    while (*sp)
    {
	char		*end_p;
	string_ty	*os;

	//
	// find where the line ends
	//
	end_p = sp;
	while (*end_p && !strchr(sep, *end_p))
	    ++end_p;

	//
	// append the line to the result
	//
	os = str_n_from_c(sp, end_p - sp);
	tmp = rpt_value_string(os);
	str_free(os);
	rpt_value_list_append(result, tmp);
	rpt_value_free(tmp);

	//
	// skip the separator
	//
	sp = end_p;
	if (*sp)
	    ++sp;
    }

    //
    // clean up and go home
    //
    rpt_value_free(a1);
    rpt_value_free(a2);
    return result;
}


rpt_func_ty rpt_func_split =
{
    "split",
    1, // optimizable
    verify,
    run
};
