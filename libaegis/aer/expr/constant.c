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
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate constants in expressions
 */

#include <aer/expr/constant.h>
#include <aer/value/ref.h>
#include <error.h>
#include <trace.h>


typedef struct rpt_expr_constant_ty rpt_expr_constant_ty;
struct rpt_expr_constant_ty
{
	RPT_EXPR
	int		lvalue;
	rpt_value_ty	*value;
};


static void destruct _((rpt_expr_ty *));

static void
destruct(ep)
	rpt_expr_ty	*ep;
{
	rpt_expr_constant_ty *this;

	this = (rpt_expr_constant_ty *)ep;
	rpt_value_free(this->value);
}


static rpt_value_ty *evaluate _((rpt_expr_ty *));

static rpt_value_ty *
evaluate(ep)
	rpt_expr_ty	*ep;
{
	rpt_expr_constant_ty *this;
	rpt_value_ty	*result;

	trace(("expr_constant::evaluate(this = %08lX)\n{\n"/*}*/, (long)ep));
	this = (rpt_expr_constant_ty *)ep;
	assert(this->value->reference_count >= 1);
	if (!this->lvalue && this->value->method->type == rpt_value_type_reference)
	{
		result = rpt_value_reference_get(this->value);
		assert(result->method->type != rpt_value_type_reference);
	}
	else
		result = rpt_value_copy(this->value);
	assert(result->reference_count >= 2);
	trace(("return %08lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


static int lvalue _((rpt_expr_ty *));

static int
lvalue(ep)
	rpt_expr_ty	*ep;
{
	rpt_expr_constant_ty *this;

	this = (rpt_expr_constant_ty *)ep;
	this->lvalue = 1;
	return (this->value->method->type == rpt_value_type_reference);
}


static rpt_expr_method_ty method =
{
	sizeof(rpt_expr_constant_ty),
	"constant",
	0, /* construct */
	destruct,
	evaluate,
	lvalue,
};


rpt_expr_ty *
rpt_expr_constant(value)
	rpt_value_ty	*value;
{
	rpt_expr_constant_ty *this;

	trace(("rpt_expr_constant(value = %08lX)\n{\n"/*}*/, (long)value));
	assert(value->reference_count >= 1);
	this = (rpt_expr_constant_ty *)rpt_expr_alloc(&method);
	this->lvalue = 0;
	this->value = rpt_value_copy(value);
	trace(("return %08lX;\n", (long)this));
	trace((/*{*/"}\n"));
	return (rpt_expr_ty *)this;
}