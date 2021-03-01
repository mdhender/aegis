/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1995, 1996 Peter Miller;
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
 * MANIFEST: functions to manipulate shift expressionss
 */

#include <aer/expr/shift.h>
#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <error.h>
#include <sub.h>


static rpt_value_ty *shift_left_evaluate _((rpt_expr_ty *));

static rpt_value_ty *
shift_left_evaluate(this)
	rpt_expr_ty	*this;
{
	sub_context_ty	*scp;
	rpt_value_ty	*v1;
	rpt_value_ty	*v1i;
	long		v1n;
	rpt_value_ty	*v2;
	rpt_value_ty	*v2i;
	long		v2n;
	rpt_value_ty	*result;

	assert(this->nchild == 2);
	v1 = rpt_expr_evaluate(this->child[0], 1);
	if (v1->method->type == rpt_value_type_error)
		return v1;
	v1i = rpt_value_integerize(v1);
	rpt_value_free(v1);

	v2 = rpt_expr_evaluate(this->child[1], 1);
	if (v2->method->type == rpt_value_type_error)
		return v2;
	v2i = rpt_value_integerize(v2);
	rpt_value_free(v2);

	if
	(
		v1i->method->type != rpt_value_type_integer
	||
		v2i->method->type != rpt_value_type_integer
	)
	{
		string_ty	*s;

		scp = sub_context_new();
		sub_var_set(scp, "Name1", "%s", v1i->method->name);
		sub_var_set(scp, "Name2", "%s", v2i->method->name);
		s = subst_intl(scp, i18n("illegal shift ($name1 << $name2)"));
		sub_context_delete(scp);
		result = rpt_value_error(this->pos, s);
		str_free(s);
		return result;
	}

	v1n = rpt_value_integer_query(v1i);
	v2n = rpt_value_integer_query(v2i);
	rpt_value_free(v1i);
	rpt_value_free(v2i);
	return rpt_value_integer((unsigned long)v1n << v2n);
}


static rpt_expr_method_ty shift_left_method =
{
	sizeof(rpt_expr_ty),
	"shift left",
	0, /* construct */
	0, /* destruct */
	shift_left_evaluate,
	0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_shift_left(e1, e2)
	rpt_expr_ty	*e1;
	rpt_expr_ty	*e2;
{
	rpt_expr_ty	*this;

	this = rpt_expr_alloc(&shift_left_method);
	rpt_expr_append(this, e1);
	rpt_expr_append(this, e2);
	return this;
}


static rpt_value_ty *shift_right_evaluate _((rpt_expr_ty *));

static rpt_value_ty *
shift_right_evaluate(this)
	rpt_expr_ty	*this;
{
	sub_context_ty	*scp;
	rpt_value_ty	*v1;
	rpt_value_ty	*v1i;
	long		v1n;
	rpt_value_ty	*v2;
	rpt_value_ty	*v2i;
	long		v2n;
	rpt_value_ty	*result;

	assert(this->nchild == 2);
	v1 = rpt_expr_evaluate(this->child[0], 1);
	if (v1->method->type == rpt_value_type_error)
		return v1;
	v1i = rpt_value_integerize(v1);
	rpt_value_free(v1);

	v2 = rpt_expr_evaluate(this->child[1], 1);
	if (v2->method->type == rpt_value_type_error)
		return v2;
	v2i = rpt_value_integerize(v2);
	rpt_value_free(v2);

	if
	(
		v1i->method->type != rpt_value_type_integer
	||
		v2i->method->type != rpt_value_type_integer
	)
	{
		string_ty	*s;

		scp = sub_context_new();
		sub_var_set(scp, "Name1", "%s", v1i->method->name);
		sub_var_set(scp, "Name2", "%s", v2i->method->name);
		s = subst_intl(scp, i18n("illegal shift ($name1 >> $name2)"));
		sub_context_delete(scp);
		result = rpt_value_error(this->pos, s);
		str_free(s);
		return result;
	}

	v1n = rpt_value_integer_query(v1i);
	v2n = rpt_value_integer_query(v2i);
	rpt_value_free(v1i);
	rpt_value_free(v2i);
	return rpt_value_integer((unsigned long)v1n >> v2n);
}


static rpt_expr_method_ty shift_right_method =
{
	sizeof(rpt_expr_ty),
	"shift right",
	0, /* construct */
	0, /* destruct */
	shift_right_evaluate,
	0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_shift_right(e1, e2)
	rpt_expr_ty	*e1;
	rpt_expr_ty	*e2;
{
	rpt_expr_ty	*this;

	this = rpt_expr_alloc(&shift_right_method);
	rpt_expr_append(this, e1);
	rpt_expr_append(this, e2);
	return this;
}