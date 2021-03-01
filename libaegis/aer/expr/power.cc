//
//	aegis - project change supervisor
//	Copyright (C) 1994-1996, 1999, 2002-2005 Peter Miller;
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
// MANIFEST: functions to manipulate power (exponentiation) expressions
//

#include <common/ac/errno.h>
#include <common/ac/string.h>
#include <common/ac/math.h>

#include <libaegis/aer/expr/power.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/real.h>
#include <common/error.h>
#include <libaegis/sub.h>
#include <common/trace.h>


static rpt_value_ty *
evaluate(rpt_expr_ty *this_thing)
{
    sub_context_ty  *scp;
    rpt_value_ty    *v1;
    rpt_value_ty    *v1a;
    double	    v1d;
    rpt_value_ty    *v2;
    rpt_value_ty    *v2a;
    double	    v2d;
    rpt_value_ty    *result;
    double	    n;
    string_ty       *s;

    //
    // evaluate the left hand side
    //
    trace(("power::evaluate()\n{\n"));
    assert(this_thing->nchild == 2);
    v1 = rpt_expr_evaluate(this_thing->child[0], 1);
    if (v1->method->type == rpt_value_type_error)
    {
	trace(("}\n"));
	return v1;
    }

    //
    // coerce the left hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    v1a = rpt_value_arithmetic(v1);
    rpt_value_free(v1);

    //
    // evaluate the right hand side
    //
    v2 = rpt_expr_evaluate(this_thing->child[1], 1);
    if (v2->method->type == rpt_value_type_error)
    {
	rpt_value_free(v1a);
	trace(("}\n"));
	return v2;
    }

    //
    // coerce the right hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    v2a = rpt_value_arithmetic(v2);
    rpt_value_free(v2);

    //
    // the type of the result depends on
    // the types of the operands
    //
    switch (v1a->method->type)
    {
    case rpt_value_type_real:
	v1d = rpt_value_real_query(v1a);
	break;

    case rpt_value_type_integer:
	v1d = rpt_value_integer_query(v1a);
	break;

    default:
	illegal_power:
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name1", v1a->method->name);
	sub_var_set_charstar(scp, "Name2", v2a->method->name);
	rpt_value_free(v1a);
	rpt_value_free(v2a);
	s = subst_intl(scp, i18n("illegal power ($name1 ** $name2)"));
	sub_context_delete(scp);
	result = rpt_value_error(this_thing->pos, s);
	str_free(s);
	trace(("}\n"));
	return result;
    }
    switch (v2a->method->type)
    {
    case rpt_value_type_real:
	v2d = rpt_value_real_query(v2a);
	break;

    case rpt_value_type_integer:
	v2d = rpt_value_integer_query(v2a);
	break;

    default:
	goto illegal_power;
    }
    rpt_value_free(v1a);
    rpt_value_free(v2a);

    errno = 0;
    n = pow(v1d, v2d);
    if (errno == 0)
	result = rpt_value_real(n);
    else
    {
	int             errno_old;

	errno_old = errno;
	scp = sub_context_new();
	sub_errno_setx(scp, errno_old);
	sub_var_set_format(scp, "Value1", "%g", v1d);
	sub_var_set_format(scp, "Value2", "%g", v2d);
	s = subst_intl(scp, i18n("$value1 ** $value2: $errno"));
	sub_context_delete(scp);
	result = rpt_value_error(this_thing->pos, s);
	str_free(s);
    }

    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static rpt_expr_method_ty method =
{
    sizeof(rpt_expr_ty),
    "**",
    0, // construct
    0, // destruct
    evaluate,
    0, // lvalue
};


rpt_expr_ty *
rpt_expr_power(rpt_expr_ty *e1, rpt_expr_ty *e2)
{
    rpt_expr_ty     *this_thing;

    this_thing = rpt_expr_alloc(&method);
    rpt_expr_append(this_thing, e1);
    rpt_expr_append(this_thing, e2);
    return this_thing;
}
