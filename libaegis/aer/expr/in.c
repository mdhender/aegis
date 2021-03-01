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
 * MANIFEST: functions to manipulate ``in'' expressions
 */

#include <aer/expr/constant.h>
#include <aer/expr/rel.h>
#include <aer/expr/in.h>
#include <aer/lex.h>
#include <aer/value/boolean.h>
#include <aer/value/error.h>
#include <aer/value/list.h>
#include <error.h>
#include <sub.h>


static rpt_value_ty *evaluate _((rpt_expr_ty *));

static rpt_value_ty *
evaluate(this)
	rpt_expr_ty	*this;
{
	rpt_value_ty	*lhs;
	rpt_value_ty	*rhs;
	rpt_value_ty	*result;
	long		n;
	long		j;
	rpt_expr_ty	*e1;
	rpt_expr_ty	*e2;
	rpt_expr_ty	*e3;

	lhs = rpt_expr_evaluate(this->child[0], 1);
	if (lhs->method->type == rpt_value_type_error)
		return lhs;

	rhs = rpt_expr_evaluate(this->child[1], 1);
	if (rhs->method->type == rpt_value_type_error)
	{
		rpt_value_free(lhs);
		return rhs;
	}
	if (rhs->method->type != rpt_value_type_list)
	{
		sub_context_ty	*scp;
		string_ty	*s;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%s", rhs->method->name);
		rpt_value_free(lhs);
		rpt_value_free(rhs);
		s =
			subst_intl
			(
				scp,
				i18n("list value required (was given $name)")
			);
		sub_context_delete(scp);
		result = rpt_value_error(this->child[1]->pos, s);
		str_free(s);
		return result;
	}

	e1 = rpt_expr_constant(lhs);
	assert(!e1->pos);
	e1->pos = rpt_pos_copy(this->child[0]->pos);
	n = rpt_value_list_length(rhs);
	for (j = 0; j < n; ++j)
	{
		rpt_value_ty	*vp;

		vp = rpt_value_list_nth(rhs, j);
		e2 = rpt_expr_constant(vp);
		assert(!e2->pos);
		e2->pos = rpt_pos_copy(this->child[1]->pos);
		e3 = rpt_expr_eq(e1, e2);
		rpt_expr_free(e2);
		result = rpt_expr_evaluate(e3, 1);
		rpt_expr_free(e3);
		if
		(
			result->method->type == rpt_value_type_error
		||
			(
				result->method->type == rpt_value_type_boolean
			&&
				rpt_value_boolean_query(result)
			)
		)
			goto done;
		rpt_value_free(result);
	}

	result = rpt_value_boolean(0);
	done:
	rpt_expr_free(e1);
	return result;
}


static rpt_expr_method_ty method =
{
	sizeof(rpt_expr_ty),
	"in",
	0, /* construct */
	0, /* destruct */
	evaluate,
	0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_in(a, b)
	rpt_expr_ty	*a;
	rpt_expr_ty	*b;
{
	rpt_expr_ty	*result;

	result = rpt_expr_alloc(&method);
	rpt_expr_append(result, a);
	rpt_expr_append(result, b);
	return result;
}
