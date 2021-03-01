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
 * MANIFEST: functions to manipulate logical expressions
 */

#include <aer/expr/logical.h>
#include <aer/value/boolean.h>
#include <aer/value/error.h>
#include <error.h>
#include <sub.h>
#include <trace.h>


static rpt_value_ty *and_evaluate _((rpt_expr_ty *));

static rpt_value_ty *
and_evaluate(this)
	rpt_expr_ty	*this;
{
	rpt_value_ty	*v1;
	rpt_value_ty	*v1i;
	rpt_value_ty	*v2;
	rpt_value_ty	*v2i;
	rpt_value_ty	*result;

	/*
	 * eveluate the left hand side
	 */
	assert(this->nchild == 2);
	v1 = rpt_expr_evaluate(this->child[0], 1);
	if (v1->method->type == rpt_value_type_error)
		return v1;
	v1i = rpt_value_booleanize(v1);
	if (v1i->method->type != rpt_value_type_boolean)
	{
		sub_context_ty	*scp;
		string_ty	*s;

		scp = sub_context_new();
		rpt_value_free(v1i);
		sub_var_set(scp, "Name", "%s", v1->method->name);
		rpt_value_free(v1);
		s =
			subst_intl
			(
				scp,
		i18n("boolean value required for logical and (was given $name)")
			);
		sub_context_delete(scp);
		result = rpt_value_error(this->child[0]->pos, s);
		str_free(s);
		return result;
	}
	rpt_value_free(v1);

	/*
	 * short circuit the evaluation if the LHS is false
	 */
	if (!rpt_value_boolean_query(v1i))
		return v1i;
	rpt_value_free(v1i);

	/*
	 * evaluate the right hand side
	 */
	v2 = rpt_expr_evaluate(this->child[1], 1);
	if (v2->method->type == rpt_value_type_error)
		return v2;
	v2i = rpt_value_booleanize(v2);
	if (v2i->method->type != rpt_value_type_boolean)
	{
		sub_context_ty	*scp;
		string_ty	*s;

		scp = sub_context_new();
		rpt_value_free(v2i);
		sub_var_set(scp, "Name", "%s", v2->method->name);
		rpt_value_free(v2);
		s =
			subst_intl
			(
				scp,
		i18n("boolean value required for logical and (was given $name)")
			);
		sub_context_delete(scp);
		result = rpt_value_error(this->child[1]->pos, s);
		str_free(s);
		return result;
	}
	rpt_value_free(v2);
	return v2i;
}


static rpt_expr_method_ty and_method =
{
	sizeof(rpt_expr_ty),
	"&&",
	0, /* construct */
	0, /* destruct */
	and_evaluate,
	0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_and_logical(e1, e2)
	rpt_expr_ty	*e1;
	rpt_expr_ty	*e2;
{
	rpt_expr_ty	*this;

	this = rpt_expr_alloc(&and_method);
	rpt_expr_append(this, e1);
	rpt_expr_append(this, e2);
	return this;
}


static rpt_value_ty *or_evaluate _((rpt_expr_ty *));

static rpt_value_ty *
or_evaluate(this)
	rpt_expr_ty	*this;
{
	rpt_value_ty	*v1;
	rpt_value_ty	*v1i;
	rpt_value_ty	*v2;
	rpt_value_ty	*v2i;
	rpt_value_ty	*result;

	/*
	 * evaluate the left hand side
	 */
	assert(this->nchild == 2);
	v1 = rpt_expr_evaluate(this->child[0], 1);
	if (v1->method->type == rpt_value_type_error)
		return v1;
	v1i = rpt_value_booleanize(v1);
	if (v1i->method->type != rpt_value_type_boolean)
	{
		sub_context_ty	*scp;
		string_ty	*s;

		scp = sub_context_new();
		rpt_value_free(v1i);
		sub_var_set(scp, "Name", "%s", v1->method->name);
		rpt_value_free(v1);
		s =
			subst_intl
			(
				scp,
		 i18n("boolean value required for logical or (was given $name)")
			);
		sub_context_delete(scp);
		result = rpt_value_error(this->child[0]->pos, s);
		str_free(s);
		return result;
	}
	rpt_value_free(v1);

	/*
	 * short circuit the evaluation if LHS is true
	 */
	if (rpt_value_boolean_query(v1i))
		return v1i;
	rpt_value_free(v1i);

	/*
	 * evaluate the right hand side
	 */
	v2 = rpt_expr_evaluate(this->child[1], 1);
	if (v2->method->type == rpt_value_type_error)
		return v2;
	v2i = rpt_value_booleanize(v2);
	if (v2i->method->type != rpt_value_type_boolean)
	{
		sub_context_ty	*scp;
		string_ty	*s;

		scp = sub_context_new();
		rpt_value_free(v2i);
		sub_var_set(scp, "Name", "%s", v2->method->name);
		rpt_value_free(v2);
		s =
			subst_intl
			(
				scp,
		 i18n("boolean value required for logical or (was given $name)")
			);
		sub_context_delete(scp);
		result = rpt_value_error(this->child[1]->pos, s);
		str_free(s);
		return result;
	}
	rpt_value_free(v2);
	return v2i;
}


static rpt_expr_method_ty or_method =
{
	sizeof(rpt_expr_ty),
	"||",
	0, /* construct */
	0, /* destruct */
	or_evaluate,
	0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_or_logical(e1, e2)
	rpt_expr_ty	*e1;
	rpt_expr_ty	*e2;
{
	rpt_expr_ty	*this;

	this = rpt_expr_alloc(&or_method);
	rpt_expr_append(this, e1);
	rpt_expr_append(this, e2);
	return this;
}


static rpt_value_ty *not_evaluate _((rpt_expr_ty *));

static rpt_value_ty *
not_evaluate(this)
	rpt_expr_ty	*this;
{
	rpt_value_ty	*v1;
	rpt_value_ty	*v2;
	rpt_value_ty	*vp;

	/*
	 * evaluate the argument
	 */
	trace(("not::evaluate()\n{\n"/*}*/));
	assert(this->nchild == 1);
	v1 = rpt_expr_evaluate(this->child[0], 1);
	if (v1->method->type == rpt_value_type_error)
	{
		trace((/*{*/"}\n"));
		return v1;
	}

	/*
	 * coerce the argument to boolean type
	 *	(will not give error if can't, will copy instead)
	 */
	v2 = rpt_value_booleanize(v1);
	rpt_value_free(v1);

	/*
	 * the type of the result depends on
	 * the types of the argument
	 */
	if (v2->method->type == rpt_value_type_boolean)
		vp = rpt_value_boolean(!rpt_value_boolean_query(v2));
	else
	{
		sub_context_ty	*scp;
		string_ty	*s;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%s", v2->method->name);
		s = subst_intl(scp, i18n("illegal logical not ($name)"));
		sub_context_delete(scp);
		vp = rpt_value_error(this->child[0]->pos, s);
		str_free(s);
	}
	rpt_value_free(v2);
	trace(("return %08lX;\n", (long)vp));
	trace((/*{*/"}\n"));
	return vp;
}


static rpt_expr_method_ty not_method =
{
	sizeof(rpt_expr_ty),
	"!e",
	0, /* construct */
	0, /* destruct */
	not_evaluate,
	0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_not_logical(a)
	rpt_expr_ty	*a;
{
	rpt_expr_ty	*this;

	this = rpt_expr_alloc(&not_method);
	rpt_expr_append(this, a);
	return this;
}


static rpt_value_ty *if_evaluate _((rpt_expr_ty *));

static rpt_value_ty *
if_evaluate(this)
	rpt_expr_ty	*this;
{
	rpt_value_ty	*v1;
	rpt_value_ty	*v1b;
	int		cond;
	rpt_value_ty	*result;

	/*
	 * evaluate the argument
	 */
	trace(("if::evaluate()\n{\n"/*}*/));
	assert(this->nchild == 3);
	v1 = rpt_expr_evaluate(this->child[0], 1);
	if (v1->method->type == rpt_value_type_error)
	{
		trace((/*{*/"}\n"));
		return v1;
	}

	/*
	 * coerce the argument to boolean type
	 *	(will not give error if can't, will copy instead)
	 */
	v1b = rpt_value_booleanize(v1);
	rpt_value_free(v1);
	if (v1b->method->type != rpt_value_type_boolean)
	{
		sub_context_ty	*scp;
		string_ty	*s;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%s", v1b->method->name);
		rpt_value_free(v1b);
		s =
			subst_intl
			(
				scp,
	      i18n("boolean value required for arithmetic if (was given $name)")
			);
		sub_context_delete(scp);
		result = rpt_value_error(this->child[0]->pos, s);
		str_free(s);
		trace((/*{*/"}\n"));
		return result;
	}

	cond = rpt_value_boolean_query(v1b);
	rpt_value_free(v1b);
	if (cond)
		result = rpt_expr_evaluate(this->child[1], 0);
	else
		result = rpt_expr_evaluate(this->child[2], 0);

	trace(("return %08lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


static rpt_expr_method_ty if_method =
{
	sizeof(rpt_expr_ty),
	"arithmetic if",
	0, /* construct */
	0, /* destruct */
	if_evaluate,
	0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_if(e1, e2, e3)
	rpt_expr_ty	*e1;
	rpt_expr_ty	*e2;
	rpt_expr_ty	*e3;
{
	rpt_expr_ty	*this;

	this = rpt_expr_alloc(&if_method);
	rpt_expr_append(this, e1);
	rpt_expr_append(this, e2);
	rpt_expr_append(this, e3);
	return this;
}