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
 * MANIFEST: functions to manipulate lookup expressions
 */

#include <aer/expr/constant.h>
#include <aer/expr/lookup.h>
#include <aer/lex.h>
#include <aer/value/error.h>
#include <aer/value/ref.h>
#include <aer/value/string.h>
#include <aer/value/struct.h>
#include <error.h>
#include <str.h>
#include <trace.h>


static rpt_value_ty *evaluate _((rpt_expr_ty *));

static rpt_value_ty *
evaluate(this)
	rpt_expr_ty	*this;
{
	rpt_value_ty	*lhs;
	rpt_value_ty	*rhs;
	int		lvalue;
	rpt_value_ty	*result;

	trace(("expr_lookup::evaluate(this = %08lX)\n{\n"/*}*/, (long)this));
	assert(this->nchild == 2);
	trace(("evaluate lhs\n"));
	lhs = rpt_expr_evaluate(this->child[0]);
	if (lhs->method->type == rpt_value_type_error)
	{
		trace((/*{*/"}\n"));
		return lhs;
	}
	assert(lhs->reference_count >= 1);
	trace(("evaluate rhs\n"));
	rhs = rpt_expr_evaluate(this->child[1]);
	if (rhs->method->type == rpt_value_type_error)
	{
		rpt_value_free(lhs);
		trace((/*{*/"}\n"));
		return rhs;
	}
	assert(rhs->reference_count >= 1);
	lvalue = (lhs->method->type == rpt_value_type_reference);
	if (lvalue)
	{
		rpt_value_ty	*vp1;

		trace(("make sure lvalue is a struct\n"));
		vp1 = rpt_value_reference_get(lhs);
		assert(vp1->reference_count >= 2);
		if (vp1->method->type != rpt_value_type_structure)
		{
			rpt_value_ty	*vp2;

			trace(("create and assign empty struct\n"));
			vp2 = rpt_value_struct((struct symtab_ty *)0);
			rpt_value_reference_set(lhs, vp2);
			assert(vp2->reference_count == 2);
			rpt_value_free(vp2);
		}
		rpt_value_free(vp1);
	}

	/*
	 * do the lookup
	 * it will not report the error itself
	 */
	trace(("do the lookup\n"));
	result = rpt_value_lookup(lhs, rhs, lvalue);
	if (result->method->type == rpt_value_type_error)
		rpt_expr_error(this, "%S", rpt_value_error_query(result));

	/*
	 * clean up and go home
	 */
	trace(("clean up and go home\n"));
	rpt_value_free(lhs);
	assert(rhs->reference_count >= 1);
	rpt_value_free(rhs);
	assert(result->reference_count >= 1);
	trace((/*{*/"}\n"));
	return result;
}


static int lvalue _((rpt_expr_ty *));

static int
lvalue(ep)
	rpt_expr_ty	*ep;
{
	assert(ep->nchild == 2);
	return rpt_expr_lvalue(ep->child[0]);
}


static rpt_expr_method_ty method =
{
	sizeof(rpt_expr_ty),
	"lookup",
	0, /* construct */
	0, /* destruct */
	evaluate,
	lvalue,
};


rpt_expr_ty *
rpt_expr_lookup(e1, e2)
	rpt_expr_ty	*e1;
	rpt_expr_ty	*e2;
{
	rpt_expr_ty	*this;

	this = rpt_expr_alloc(&method);
	rpt_expr_append(this, e1);
	rpt_expr_append(this, e2);
	return this;
}


rpt_expr_ty *
rpt_expr_dot(e1, name)
	rpt_expr_ty	*e1;
	string_ty	*name;
{
	rpt_value_ty	*vp;
	rpt_expr_ty	*e2;
	rpt_expr_ty	*result;

	vp = rpt_value_string(name);
	e2 = rpt_expr_constant(vp);
	e2->pos = rpt_lex_pos_get();
	rpt_value_free(vp);
	result = rpt_expr_lookup(e1, e2);
	rpt_expr_free(e2);
	return result;
}
