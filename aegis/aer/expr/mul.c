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
 * MANIFEST: functions to manipulate mul expressions
 */

#include <math.h>

#include <aer/expr/mul.h>
#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <aer/value/real.h>
#include <error.h>
#include <trace.h>

#define PAIR(a, b)	((a) * rpt_value_type_MAX + (b))


static rpt_value_ty *mul_evaluate _((rpt_expr_ty *));

static rpt_value_ty *
mul_evaluate(this)
	rpt_expr_ty	*this;
{
	rpt_value_ty	*v1;
	rpt_value_ty	*v1a;
	rpt_value_ty	*v2;
	rpt_value_ty	*v2a;
	rpt_value_ty	*vp;

	/*
	 * evaluate the left hand side
	 */
	trace(("mul::evaluate()\n{\n"/*}*/));
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
	switch (PAIR(v1a->method->type, v2a->method->type))
	{
	case PAIR(rpt_value_type_real, rpt_value_type_real):
		vp =
			rpt_value_real
			(
				rpt_value_real_query(v1a)
			*
				rpt_value_real_query(v2a)
			);
		trace(("vp = %g real\n", rpt_value_real_query(vp)));
		break;

	case PAIR(rpt_value_type_real, rpt_value_type_integer):
		vp =
			rpt_value_real
			(
				rpt_value_real_query(v1a)
			*
				rpt_value_integer_query(v2a)
			);
		trace(("vp = %g real\n", rpt_value_real_query(vp)));
		break;

	case PAIR(rpt_value_type_integer, rpt_value_type_real):
		vp =
			rpt_value_real
			(
				rpt_value_integer_query(v1a)
			*
				rpt_value_real_query(v2a)
			);
		trace(("vp = %g real\n", rpt_value_real_query(vp)));
		break;

	case PAIR(rpt_value_type_integer, rpt_value_type_integer):
		vp =
			rpt_value_integer
			(
				rpt_value_integer_query(v1a)
			*
				rpt_value_integer_query(v2a)
			);
		trace(("vp = %ld integer\n", rpt_value_integer_query(vp)));
		break;

	default:
		vp =
			rpt_value_error
			(
				"illegal multiplication (%s * %s)",
				v1a->method->name,
				v2a->method->name
			);
		rpt_expr_error(this, "%S", rpt_value_error_query(vp));
		break;
	}
	rpt_value_free(v1a);
	rpt_value_free(v2a);
	trace(("return %08lX;\n", (long)vp));
	trace((/*{*/"}\n"));
	return vp;
}


static rpt_expr_method_ty mul_method =
{
	sizeof(rpt_expr_ty),
	"mul",
	0, /* construct */
	0, /* destruct */
	mul_evaluate,
	0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_mul(a, b)
	rpt_expr_ty	*a;
	rpt_expr_ty	*b;
{
	rpt_expr_ty	*this;

	this = rpt_expr_alloc(&mul_method);
	rpt_expr_append(this, a);
	rpt_expr_append(this, b);
	return this;
}


static rpt_value_ty *div_evaluate _((rpt_expr_ty *));

static rpt_value_ty *
div_evaluate(this)
	rpt_expr_ty	*this;
{
	rpt_value_ty	*v1;
	rpt_value_ty	*v1a;
	rpt_value_ty	*v2;
	rpt_value_ty	*v2a;
	double		den;
	long		lden;
	rpt_value_ty	*result;

	/*
	 * evaluate the left hand side
	 */
	trace(("div::evaluate()\n{\n"/*}*/));
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
	switch (PAIR(v1a->method->type, v2a->method->type))
	{
	case PAIR(rpt_value_type_real, rpt_value_type_real):
		den = rpt_value_real_query(v2a);
		if (den == 0)
		{
			div_by_zero:
			result = rpt_value_error("division by zero");
			rpt_expr_error(this, "%S", rpt_value_error_query(result));
			break;
		}
		result = rpt_value_real(rpt_value_real_query(v1a) / den);
		trace(("result = %g real\n", rpt_value_real_query(result)));
		break;

	case PAIR(rpt_value_type_real, rpt_value_type_integer):
		lden = rpt_value_integer_query(v2a);
		if (lden == 0)
			goto div_by_zero;
		result = rpt_value_real(rpt_value_real_query(v1a) / lden);
		trace(("result = %g real\n", rpt_value_real_query(result)));
		break;

	case PAIR(rpt_value_type_integer, rpt_value_type_real):
		den = rpt_value_real_query(v2a);
		if (den == 0)
			goto div_by_zero;
		result = rpt_value_real(rpt_value_integer_query(v1a) / den);
		trace(("result = %g real\n", rpt_value_real_query(result)));
		break;

	case PAIR(rpt_value_type_integer, rpt_value_type_integer):
		lden = rpt_value_integer_query(v2a);
		if (lden == 0)
			goto div_by_zero;
		result = rpt_value_integer(rpt_value_integer_query(v1a) / lden);
		trace(("result = %ld integer\n", rpt_value_integer_query(result)));
		break;

	default:
		result =
			rpt_value_error
			(
				"illegal division (%s / %s)",
				v1a->method->name,
				v2a->method->name
			);
		rpt_expr_error(this, "%S", rpt_value_error_query(result));
		break;
	}
	rpt_value_free(v1a);
	rpt_value_free(v2a);
	trace(("return %08lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


static rpt_expr_method_ty div_method =
{
	sizeof(rpt_expr_ty),
	"div",
	0, /* construct */
	0, /* destruct */
	div_evaluate,
	0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_div(a, b)
	rpt_expr_ty	*a;
	rpt_expr_ty	*b;
{
	rpt_expr_ty	*this;

	this = rpt_expr_alloc(&div_method);
	rpt_expr_append(this, a);
	rpt_expr_append(this, b);
	return this;
}


static rpt_value_ty *mod_evaluate _((rpt_expr_ty *));

static rpt_value_ty *
mod_evaluate(this)
	rpt_expr_ty	*this;
{
	rpt_value_ty	*v1;
	rpt_value_ty	*v1a;
	rpt_value_ty	*v2;
	rpt_value_ty	*v2a;
	double		den;
	long		lden;
	rpt_value_ty	*result;

	/*
	 * evaluate the left hand side
	 */
	trace(("mod::evaluate()\n{\n"/*}*/));
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
	switch (PAIR(v1a->method->type, v2a->method->type))
	{
	case PAIR(rpt_value_type_real, rpt_value_type_real):
		den = rpt_value_real_query(v2a);
		if (den == 0)
		{
			mod_by_zero:
			result = rpt_value_error("modulo by zero");
			rpt_expr_error(this, "%S", rpt_value_error_query(result));
			break;
		}
		result = rpt_value_real(fmod(rpt_value_real_query(v1a), den));
		trace(("result = %g real\n", rpt_value_real_query(result)));
		break;

	case PAIR(rpt_value_type_real, rpt_value_type_integer):
		den = rpt_value_integer_query(v2a);
		if (den == 0)
			goto mod_by_zero;
		result = rpt_value_real(fmod(rpt_value_real_query(v1a), den));
		trace(("result = %g real\n", rpt_value_real_query(result)));
		break;

	case PAIR(rpt_value_type_integer, rpt_value_type_real):
		den = rpt_value_real_query(v2a);
		if (den == 0)
			goto mod_by_zero;
		result = rpt_value_real(fmod((double)rpt_value_integer_query(v1a), den));
		trace(("result = %g real\n", rpt_value_real_query(result)));
		break;

	case PAIR(rpt_value_type_integer, rpt_value_type_integer):
		lden = rpt_value_integer_query(v2a);
		if (lden == 0)
			goto mod_by_zero;
		result = rpt_value_integer(rpt_value_integer_query(v1a) % lden);
		trace(("result = %ld integer\n", rpt_value_integer_query(result)));
		break;

	default:
		result =
			rpt_value_error
			(
				"illegal modulo (%s % %s)",
				v1a->method->name,
				v2a->method->name
			);
		rpt_expr_error(this, "%S", rpt_value_error_query(result));
		break;
	}
	rpt_value_free(v1a);
	rpt_value_free(v2a);
	trace(("return %08lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


static rpt_expr_method_ty mod_method =
{
	sizeof(rpt_expr_ty),
	"mod",
	0, /* construct */
	0, /* destruct */
	mod_evaluate,
	0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_mod(a, b)
	rpt_expr_ty	*a;
	rpt_expr_ty	*b;
{
	rpt_expr_ty	*this;

	this = rpt_expr_alloc(&mod_method);
	rpt_expr_append(this, a);
	rpt_expr_append(this, b);
	return this;
}
