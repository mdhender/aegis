/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994 Peter Miller.
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
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: functions to manipulate power (exponentiation) expressions
 */

#include <errno.h>
#include <ac/string.h>
#include <math.h>

#include <aer/expr/power.h>
#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <aer/value/real.h>
#include <error.h>
#include <trace.h>


static rpt_value_ty *evaluate _((rpt_expr_ty *));

static rpt_value_ty *
evaluate(this)
	rpt_expr_ty	*this;
{
	rpt_value_ty	*v1;
	rpt_value_ty	*v1a;
	double		v1d;
	rpt_value_ty	*v2;
	rpt_value_ty	*v2a;
	double		v2d;
	rpt_value_ty	*result;
	double		n;

	/*
	 * evaluate the left hand side
	 */
	trace(("power::evaluate()\n{\n"/*}*/));
	assert(this->nchild == 2);
	v1 = rpt_expr_evaluate(this->child[0]);
	if (v1->method->type == rpt_value_type_error)
	{
		trace((/*{*/"}\n"));
		return v1;
	}

	/*
	 * coerce the left hand side to an arithmetic type
	 *	(will not give error if can't, will copy instead)
	 */
	v1a = rpt_value_arithmetic(v1);
	rpt_value_free(v1);

	/*
	 * evaluate the right hand side
	 */
	v2 = rpt_expr_evaluate(this->child[1]);
	if (v2->method->type == rpt_value_type_error)
	{
		rpt_value_free(v1a);
		trace((/*{*/"}\n"));
		return v2;
	}

	/*
	 * coerce the right hand side to an arithmetic type
	 *	(will not give error if can't, will copy instead)
	 */
	v2a = rpt_value_arithmetic(v2);
	rpt_value_free(v2);

	/*
	 * the type of the result depends on
	 * the types of the operands
	 */
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
		result =
			rpt_value_error
			(
				"illegal power (%s ** %s)",
				v1a->method->name,
				v2a->method->name
			);
		rpt_expr_error(this, "%S", rpt_value_error_query(result));
		trace((/*{*/"}\n"));
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
		result =
			rpt_value_error
			(
				"%g ** %g: %s",
				v1d,
				v2d,
				strerror(errno)
			);
		rpt_expr_error(this, "%S", rpt_value_error_query(result));
	}
		
	trace(("return %08lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


static rpt_expr_method_ty method =
{
	sizeof(rpt_expr_ty),
	"**",
	0, /* construct */
	0, /* destruct */
	evaluate,
	0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_power(e1, e2)
	rpt_expr_ty	*e1;
	rpt_expr_ty	*e2;
{
	rpt_expr_ty	*this;

	this = rpt_expr_alloc(&method);
	rpt_expr_append(this, e1);
	rpt_expr_append(this, e2);
	return this;
}
