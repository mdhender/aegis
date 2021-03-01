//
//	aegis - project change supervisor
//	Copyright (C) 1997, 1999, 2003, 2004 Peter Miller;
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
// MANIFEST: functions to implement the builtin getenv function
//

#include <ac/stdlib.h>

#include <aer/expr.h>
#include <aer/func/getenv.h>
#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <aer/value/string.h>
#include <aer/value/time.h>
#include <error.h>
#include <sub.h>


static int
verify(rpt_expr_ty *ep)
{
    return (ep->nchild == 1);
}


static rpt_value_ty *
run(rpt_expr_ty *ep, size_t argc, rpt_value_ty **argv)
{
    const char      *cp;
    rpt_value_ty    *tmp;
    string_ty       *s;
    rpt_value_ty    *result;

    assert(argc == 1);

    //
    // Coerce the argument to a string.
    // It is an error if it can't be.
    //
    tmp = rpt_value_stringize(argv[0]);
    if (tmp->method->type != rpt_value_type_string)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	rpt_value_free(tmp);
	sub_var_set_charstar(scp, "Function", "getenv");
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
    cp = getenv(s->str_text);
    if (!cp)
	cp = "";
    rpt_value_free(tmp);

    //
    // build the return value
    //
    s = str_from_c(cp);
    result = rpt_value_string(s);
    str_free(s);
    return result;
}


rpt_func_ty rpt_func_getenv =
{
    "getenv",
    1, // optimizable
    verify,
    run,
};
