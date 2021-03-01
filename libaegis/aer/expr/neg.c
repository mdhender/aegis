/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1995, 1996, 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate unary expressions
 */

#include <aer/expr/neg.h>
#include <aer/value/boolean.h>
#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <aer/value/real.h>
#include <error.h>
#include <sub.h>
#include <trace.h>


static rpt_value_ty *neg_evaluate _((rpt_expr_ty *));

static rpt_value_ty *
neg_evaluate(this)
	rpt_expr_ty	*this;
{
	sub_context_ty	*scp;
	rpt_value_ty	*v1;
	rpt_value_ty	*v2;
	rpt_value_ty	*vp;
	string_ty	*s;

	/*
	 * evaluate the argument
	 */
	trace(("neg::evaluate()\n{\n"/*}*/));
	assert(this->nchild == 1);
	v1 = rpt_expr_evaluate(this->child[0], 1);
	if (v1->method->type == rpt_value_type_error)
	{
		trace((/*{*/"}\n"));
		return v1;
	}

	/*
	 * coerce the argument to an arithmetic type
	 *	(will not give error if can't, will copy instead)
	 */
	v2 = rpt_value_arithmetic(v1);
	rpt_value_free(v1);

	/*
	 * the type of the result depends on
	 * the types of the argument
	 */
	switch (v2->method->type)
	{
	case rpt_value_type_real:
		vp = rpt_value_real(-rpt_value_real_query(v2));
		trace(("vp = %g real\n", rpt_value_real_query(vp)));
		break;

	case rpt_value_type_integer:
		vp = rpt_value_integer(-rpt_value_integer_query(v2));
		trace(("vp = %ld integer\n", rpt_value_integer_query(vp)));
		break;

	default:
		scp = sub_context_new();
		sub_var_set_charstar(scp, "Name", v2->method->name);
		s = subst_intl(scp, i18n("illegal negative ($name)"));
		sub_context_delete(scp);
		vp = rpt_value_error(this->child[0]->pos, s);
		str_free(s);
		break;
	}
	rpt_value_free(v2);
	trace(("return %08lX;\n", (long)vp));
	trace((/*{*/"}\n"));
	return vp;
}


static rpt_expr_method_ty neg_method =
{
	sizeof(rpt_expr_ty),
	"negative",
	0, /* construct */
	0, /* destruct */
	neg_evaluate,
	0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_neg(a)
	rpt_expr_ty	*a;
{
	rpt_expr_ty	*this;

	this = rpt_expr_alloc(&neg_method);
	rpt_expr_append(this, a);
	return this;
}


static rpt_value_ty *pos_evaluate _((rpt_expr_ty *));

static rpt_value_ty *
pos_evaluate(this)
	rpt_expr_ty	*this;
{
	sub_context_ty	*scp;
	rpt_value_ty	*v1;
	rpt_value_ty	*v2;
	rpt_value_ty	*vp;
	string_ty	*s;

	/*
	 * evaluate the argument
	 */
	trace(("pos::evaluate()\n{\n"/*}*/));
	assert(this->nchild == 1);
	v1 = rpt_expr_evaluate(this->child[0], 1);
	if (v1->method->type == rpt_value_type_error)
	{
		trace((/*{*/"}\n"));
		return v1;
	}

	/*
	 * coerce the argument to an arithmetic type
	 *	(will not give error if can't, will copy instead)
	 */
	v2 = rpt_value_arithmetic(v1);
	rpt_value_free(v1);

	/*
	 * the type of the result depends on
	 * the types of the argument
	 */
	switch (v2->method->type)
	{
	case rpt_value_type_real:
	case rpt_value_type_integer:
		vp = rpt_value_copy(v2);
		break;

	default:
		scp = sub_context_new();
		sub_var_set_charstar(scp, "Name", v2->method->name);
		s = subst_intl(scp, i18n("illegal positive ($name)"));
		sub_context_delete(scp);
		vp = rpt_value_error(this->child[0]->pos, s);
		str_free(s);
		break;
	}
	rpt_value_free(v2);
	trace(("return %08lX;\n", (long)vp));
	trace((/*{*/"}\n"));
	return vp;
}


static rpt_expr_method_ty pos_method =
{
	sizeof(rpt_expr_ty),
	"positive",
	0, /* construct */
	0, /* destruct */
	pos_evaluate,
	0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_pos(a)
	rpt_expr_ty	*a;
{
	rpt_expr_ty	*this;

	this = rpt_expr_alloc(&pos_method);
	rpt_expr_append(this, a);
	return this;
}
