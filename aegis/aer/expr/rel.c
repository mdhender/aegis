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
 * MANIFEST: functions to manipulate relative comparison expressions
 */

#include <aer/expr/rel.h>
#include <aer/value/boolean.h>
#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <aer/value/real.h>
#include <aer/value/string.h>
#include <error.h>
#include <fstrcmp.h>
#include <str.h>
#include <trace.h>

#define PAIR(a, b)	((a) * rpt_value_type_MAX + (b))


static rpt_value_ty *lt_evaluate _((rpt_expr_ty *));

static rpt_value_ty *
lt_evaluate(this)
	rpt_expr_ty	*this;
{
	rpt_value_ty	*lv1;
	rpt_value_ty	*lv2;
	rpt_value_ty	*rv1;
	rpt_value_ty	*rv2;
	rpt_value_ty	*vp;

	/*
	 * evaluate the left hand side
	 */
	trace(("lt::evaluate()\n{\n"/*}*/));
	assert(this->nchild == 2);
	lv1 = rpt_expr_evaluate(this->child[0]);
	if (lv1->method->type == rpt_value_type_error)
	{
		trace((/*{*/"}\n"));
		return lv1;
	}

	/*
	 * coerce the left hand side to an arithmetic type
	 *	(will not give error if can't, will copy instead)
	 */
	lv2 = rpt_value_arithmetic(lv1);
	rpt_value_free(lv1);

	/*
	 * evaluate the right hand side
	 */
	rv1 = rpt_expr_evaluate(this->child[1]);
	if (rv1->method->type == rpt_value_type_error)
	{
		rpt_value_free(lv2);
		trace((/*{*/"}\n"));
		return rv1;
	}

	/*
	 * coerce the right hand side to an arithmetic type
	 *	(will not give error if can't, will copy instead)
	 */
	rv2 = rpt_value_arithmetic(rv1);
	rpt_value_free(rv1);

	/*
	 * what to do depends on
	 * the types of the operands
	 */
	switch (PAIR(lv2->method->type, rv2->method->type))
	{
	case PAIR(rpt_value_type_real, rpt_value_type_real):
		vp =
			rpt_value_boolean
			(
				rpt_value_real_query(lv2)
			<
				rpt_value_real_query(rv2)
			);
		trace(("(%g < %g) --> %d\n", rpt_value_real_query(lv2), rpt_value_real_query(rv2), rpt_value_boolean_query(vp)));
		break;

	case PAIR(rpt_value_type_real, rpt_value_type_integer):
		vp =
			rpt_value_boolean
			(
				rpt_value_real_query(lv2)
			<
				rpt_value_integer_query(rv2)
			);
		break;

	case PAIR(rpt_value_type_integer, rpt_value_type_real):
		vp =
			rpt_value_boolean
			(
				rpt_value_integer_query(lv2)
			<
				rpt_value_real_query(rv2)
			);
		break;

	case PAIR(rpt_value_type_integer, rpt_value_type_integer):
		vp =
			rpt_value_boolean
			(
				rpt_value_integer_query(lv2)
			<
				rpt_value_integer_query(rv2)
			);
		break;

	case PAIR(rpt_value_type_string, rpt_value_type_string):
		vp =
			rpt_value_boolean
			(
				strcmp
				(
					rpt_value_string_query(lv2)->str_text,
					rpt_value_string_query(rv2)->str_text
				)
			<
				0
			);
		break;

	default:
		vp =
			rpt_value_error
			(
				"illegal comparison (%s < %s)",
				lv2->method->name,
				rv2->method->name
			);
		rpt_expr_error(this, "%S", rpt_value_error_query(vp));
		break;
	}
	rpt_value_free(lv2);
	rpt_value_free(rv2);
	trace(("return %08lX;\n", (long)vp));
	trace((/*{*/"}\n"));
	return vp;
}


static rpt_expr_method_ty lt_method =
{
	sizeof(rpt_expr_ty),
	"less than",
	0, /* construct */
	0, /* destruct */
	lt_evaluate,
	0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_lt(a, b)
	rpt_expr_ty	*a;
	rpt_expr_ty	*b;
{
	rpt_expr_ty	*this;

	this = rpt_expr_alloc(&lt_method);
	rpt_expr_append(this, a);
	rpt_expr_append(this, b);
	return this;
}


static rpt_value_ty *le_evaluate _((rpt_expr_ty *));

static rpt_value_ty *
le_evaluate(this)
	rpt_expr_ty	*this;
{
	rpt_value_ty	*lv1;
	rpt_value_ty	*lv2;
	rpt_value_ty	*rv1;
	rpt_value_ty	*rv2;
	rpt_value_ty	*vp;

	/*
	 * evaluate the left hand side
	 */
	trace(("le::evaluate()\n{\n"/*}*/));
	assert(this->nchild == 2);
	lv1 = rpt_expr_evaluate(this->child[0]);
	if (lv1->method->type == rpt_value_type_error)
	{
		trace((/*{*/"}\n"));
		return lv1;
	}

	/*
	 * coerce the left hand side to an arithmetic type
	 *	(will not give error if can't, will copy instead)
	 */
	lv2 = rpt_value_arithmetic(lv1);
	rpt_value_free(lv1);

	/*
	 * evaluate the right hand side
	 */
	rv1 = rpt_expr_evaluate(this->child[1]);
	if (rv1->method->type == rpt_value_type_error)
	{
		rpt_value_free(lv2);
		trace((/*{*/"}\n"));
		return rv1;
	}

	/*
	 * coerce the right hand side to an arithmetic type
	 *	(will not give error if can't, will copy instead)
	 */
	rv2 = rpt_value_arithmetic(rv1);
	rpt_value_free(rv1);

	/*
	 * what to do depends on
	 * the types of the operands
	 */
	switch (PAIR(lv2->method->type, rv2->method->type))
	{
	case PAIR(rpt_value_type_real, rpt_value_type_real):
		vp =
			rpt_value_boolean
			(
				rpt_value_real_query(lv2)
			<=
				rpt_value_real_query(rv2)
			);
		break;

	case PAIR(rpt_value_type_real, rpt_value_type_integer):
		vp =
			rpt_value_boolean
			(
				rpt_value_real_query(lv2)
			<=
				rpt_value_integer_query(rv2)
			);
		break;

	case PAIR(rpt_value_type_integer, rpt_value_type_real):
		vp =
			rpt_value_boolean
			(
				rpt_value_integer_query(lv2)
			<=
				rpt_value_real_query(rv2)
			);
		break;

	case PAIR(rpt_value_type_integer, rpt_value_type_integer):
		vp =
			rpt_value_boolean
			(
				rpt_value_integer_query(lv2)
			<=
				rpt_value_integer_query(rv2)
			);
		break;

	case PAIR(rpt_value_type_string, rpt_value_type_string):
		vp =
			rpt_value_boolean
			(
				strcmp
				(
					rpt_value_string_query(lv2)->str_text,
					rpt_value_string_query(rv2)->str_text
				)
			<=
				0
			);
		break;

	default:
		vp =
			rpt_value_error
			(
				"illegal comparison (%s <= %s)",
				lv2->method->name,
				rv2->method->name
			);
		rpt_expr_error(this, "%S", rpt_value_error_query(vp));
		break;
	}
	rpt_value_free(lv2);
	rpt_value_free(rv2);
	trace(("return %08lX;\n", (long)vp));
	trace((/*{*/"}\n"));
	return vp;
}


static rpt_expr_method_ty le_method =
{
	sizeof(rpt_expr_ty),
	"less than or equal to",
	0, /* construct */
	0, /* destruct */
	le_evaluate,
	0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_le(a, b)
	rpt_expr_ty	*a;
	rpt_expr_ty	*b;
{
	rpt_expr_ty	*this;

	this = rpt_expr_alloc(&le_method);
	rpt_expr_append(this, a);
	rpt_expr_append(this, b);
	return this;
}


static rpt_value_ty *gt_evaluate _((rpt_expr_ty *));

static rpt_value_ty *
gt_evaluate(this)
	rpt_expr_ty	*this;
{
	rpt_value_ty	*lv1;
	rpt_value_ty	*lv2;
	rpt_value_ty	*rv1;
	rpt_value_ty	*rv2;
	rpt_value_ty	*vp;

	/*
	 * evaluate the left hand side
	 */
	trace(("gt::evaluate()\n{\n"/*}*/));
	assert(this->nchild == 2);
	lv1 = rpt_expr_evaluate(this->child[0]);
	if (lv1->method->type == rpt_value_type_error)
	{
		trace((/*{*/"}\n"));
		return lv1;
	}

	/*
	 * coerce the left hand side to an arithmetic type
	 *	(will not give error if can't, will copy instead)
	 */
	lv2 = rpt_value_arithmetic(lv1);
	rpt_value_free(lv1);

	/*
	 * evaluate the right hand side
	 */
	rv1 = rpt_expr_evaluate(this->child[1]);
	if (rv1->method->type == rpt_value_type_error)
	{
		rpt_value_free(lv2);
		trace((/*{*/"}\n"));
		return rv1;
	}

	/*
	 * coerce the right hand side to an arithmetic type
	 *	(will not give error if can't, will copy instead)
	 */
	rv2 = rpt_value_arithmetic(rv1);
	rpt_value_free(rv1);

	/*
	 * what to do depends on
	 * the types of the operands
	 */
	switch (PAIR(lv2->method->type, rv2->method->type))
	{
	case PAIR(rpt_value_type_real, rpt_value_type_real):
		vp =
			rpt_value_boolean
			(
				rpt_value_real_query(lv2)
			>
				rpt_value_real_query(rv2)
			);
		break;

	case PAIR(rpt_value_type_real, rpt_value_type_integer):
		vp =
			rpt_value_boolean
			(
				rpt_value_real_query(lv2)
			>
				rpt_value_integer_query(rv2)
			);
		break;

	case PAIR(rpt_value_type_integer, rpt_value_type_real):
		vp =
			rpt_value_boolean
			(
				rpt_value_integer_query(lv2)
			>
				rpt_value_real_query(rv2)
			);
		break;

	case PAIR(rpt_value_type_integer, rpt_value_type_integer):
		vp =
			rpt_value_boolean
			(
				rpt_value_integer_query(lv2)
			>
				rpt_value_integer_query(rv2)
			);
		break;

	case PAIR(rpt_value_type_string, rpt_value_type_string):
		vp =
			rpt_value_boolean
			(
				strcmp
				(
					rpt_value_string_query(lv2)->str_text,
					rpt_value_string_query(rv2)->str_text
				)
			>
				0
			);
		break;

	default:
		vp =
			rpt_value_error
			(
				"illegal comparison (%s > %s)",
				lv2->method->name,
				rv2->method->name
			);
		rpt_expr_error(this, "%S", rpt_value_error_query(vp));
		break;
	}
	rpt_value_free(lv2);
	rpt_value_free(rv2);
	trace(("return %08lX;\n", (long)vp));
	trace((/*{*/"}\n"));
	return vp;
}


static rpt_expr_method_ty gt_method =
{
	sizeof(rpt_expr_ty),
	"greater than",
	0, /* construct */
	0, /* destruct */
	gt_evaluate,
	0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_gt(a, b)
	rpt_expr_ty	*a;
	rpt_expr_ty	*b;
{
	rpt_expr_ty	*this;

	this = rpt_expr_alloc(&gt_method);
	rpt_expr_append(this, a);
	rpt_expr_append(this, b);
	return this;
}


static rpt_value_ty *ge_evaluate _((rpt_expr_ty *));

static rpt_value_ty *
ge_evaluate(this)
	rpt_expr_ty	*this;
{
	rpt_value_ty	*lv1;
	rpt_value_ty	*lv2;
	rpt_value_ty	*rv1;
	rpt_value_ty	*rv2;
	rpt_value_ty	*vp;

	/*
	 * evaluate the left hand side
	 */
	trace(("ge::evaluate()\n{\n"/*}*/));
	assert(this->nchild == 2);
	lv1 = rpt_expr_evaluate(this->child[0]);
	if (lv1->method->type == rpt_value_type_error)
	{
		trace((/*{*/"}\n"));
		return lv1;
	}

	/*
	 * coerce the left hand side to an arithmetic type
	 *	(will not give error if can't, will copy instead)
	 */
	lv2 = rpt_value_arithmetic(lv1);
	rpt_value_free(lv1);

	/*
	 * evaluate the right hand side
	 */
	rv1 = rpt_expr_evaluate(this->child[1]);
	if (rv1->method->type == rpt_value_type_error)
	{
		rpt_value_free(lv2);
		trace((/*{*/"}\n"));
		return rv1;
	}

	/*
	 * coerce the right hand side to an arithmetic type
	 *	(will not give error if can't, will copy instead)
	 */
	rv2 = rpt_value_arithmetic(rv1);
	rpt_value_free(rv1);

	/*
	 * what to do depends on
	 * the types of the operands
	 */
	switch (PAIR(lv2->method->type, rv2->method->type))
	{
	case PAIR(rpt_value_type_real, rpt_value_type_real):
		vp =
			rpt_value_boolean
			(
				rpt_value_real_query(lv2)
			>=
				rpt_value_real_query(rv2)
			);
		break;

	case PAIR(rpt_value_type_real, rpt_value_type_integer):
		vp =
			rpt_value_boolean
			(
				rpt_value_real_query(lv2)
			>=
				rpt_value_integer_query(rv2)
			);
		break;

	case PAIR(rpt_value_type_integer, rpt_value_type_real):
		vp =
			rpt_value_boolean
			(
				rpt_value_integer_query(lv2)
			>=
				rpt_value_real_query(rv2)
			);
		break;

	case PAIR(rpt_value_type_integer, rpt_value_type_integer):
		vp =
			rpt_value_boolean
			(
				rpt_value_integer_query(lv2)
			>=
				rpt_value_integer_query(rv2)
			);
		break;

	case PAIR(rpt_value_type_string, rpt_value_type_string):
		vp =
			rpt_value_boolean
			(
				strcmp
				(
					rpt_value_string_query(lv2)->str_text,
					rpt_value_string_query(rv2)->str_text
				)
			>=
				0
			);
		break;

	default:
		vp =
			rpt_value_error
			(
				"illegal comparison (%s >= %s)",
				lv2->method->name,
				rv2->method->name
			);
		rpt_expr_error(this, "%S", rpt_value_error_query(vp));
		break;
	}
	rpt_value_free(lv2);
	rpt_value_free(rv2);
	trace(("return %08lX;\n", (long)vp));
	trace((/*{*/"}\n"));
	return vp;
}


static rpt_expr_method_ty ge_method =
{
	sizeof(rpt_expr_ty),
	"greater than or equal to",
	0, /* construct */
	0, /* destruct */
	ge_evaluate,
	0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_ge(a, b)
	rpt_expr_ty	*a;
	rpt_expr_ty	*b;
{
	rpt_expr_ty	*this;

	this = rpt_expr_alloc(&ge_method);
	rpt_expr_append(this, a);
	rpt_expr_append(this, b);
	return this;
}


static rpt_value_ty *eq_evaluate _((rpt_expr_ty *));

static rpt_value_ty *
eq_evaluate(this)
	rpt_expr_ty	*this;
{
	rpt_value_ty	*lv1;
	rpt_value_ty	*lv2;
	rpt_value_ty	*rv1;
	rpt_value_ty	*rv2;
	rpt_value_ty	*vp;

	/*
	 * evaluate the left hand side
	 */
	trace(("eq::evaluate()\n{\n"/*}*/));
	assert(this->nchild == 2);
	lv1 = rpt_expr_evaluate(this->child[0]);
	if (lv1->method->type == rpt_value_type_error)
	{
		trace((/*{*/"}\n"));
		return lv1;
	}

	/*
	 * coerce the left hand side to an arithmetic type
	 *	(will not give error if can't, will copy instead)
	 */
	lv2 = rpt_value_arithmetic(lv1);
	rpt_value_free(lv1);

	/*
	 * evaluate the right hand side
	 */
	rv1 = rpt_expr_evaluate(this->child[1]);
	if (rv1->method->type == rpt_value_type_error)
	{
		rpt_value_free(lv2);
		trace((/*{*/"}\n"));
		return rv1;
	}

	/*
	 * coerce the right hand side to an arithmetic type
	 *	(will not give error if can't, will copy instead)
	 */
	rv2 = rpt_value_arithmetic(rv1);
	rpt_value_free(rv1);

	/*
	 * what to do depends on
	 * the types of the operands
	 */
	switch (PAIR(lv2->method->type, rv2->method->type))
	{
	case PAIR(rpt_value_type_real, rpt_value_type_real):
		vp =
			rpt_value_boolean
			(
				rpt_value_real_query(lv2)
			==
				rpt_value_real_query(rv2)
			);
		break;

	case PAIR(rpt_value_type_real, rpt_value_type_integer):
		vp =
			rpt_value_boolean
			(
				rpt_value_real_query(lv2)
			==
				rpt_value_integer_query(rv2)
			);
		break;

	case PAIR(rpt_value_type_integer, rpt_value_type_real):
		vp =
			rpt_value_boolean
			(
				rpt_value_integer_query(lv2)
			==
				rpt_value_real_query(rv2)
			);
		break;

	case PAIR(rpt_value_type_integer, rpt_value_type_integer):
		vp =
			rpt_value_boolean
			(
				rpt_value_integer_query(lv2)
			==
				rpt_value_integer_query(rv2)
			);
		break;

	case PAIR(rpt_value_type_string, rpt_value_type_string):
		vp =
			rpt_value_boolean
			(
				str_equal
				(
					rpt_value_string_query(lv2),
					rpt_value_string_query(rv2)
				)
			);
		break;

	default:
		vp =
			rpt_value_error
			(
				"illegal comparison (%s == %s)",
				lv2->method->name,
				rv2->method->name
			);
		rpt_expr_error(this, "%S", rpt_value_error_query(vp));
		break;
	}
	rpt_value_free(lv2);
	rpt_value_free(rv2);
	trace(("return %08lX;\n", (long)vp));
	trace((/*{*/"}\n"));
	return vp;
}


static rpt_expr_method_ty eq_method =
{
	sizeof(rpt_expr_ty),
	"equal to",
	0, /* construct */
	0, /* destruct */
	eq_evaluate,
	0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_eq(a, b)
	rpt_expr_ty	*a;
	rpt_expr_ty	*b;
{
	rpt_expr_ty	*this;

	this = rpt_expr_alloc(&eq_method);
	rpt_expr_append(this, a);
	rpt_expr_append(this, b);
	return this;
}


static rpt_value_ty *ne_evaluate _((rpt_expr_ty *));

static rpt_value_ty *
ne_evaluate(this)
	rpt_expr_ty	*this;
{
	rpt_value_ty	*lv1;
	rpt_value_ty	*lv2;
	rpt_value_ty	*rv1;
	rpt_value_ty	*rv2;
	rpt_value_ty	*vp;

	/*
	 * evaluate the left hand side
	 */
	trace(("ne::evaluate()\n{\n"/*}*/));
	assert(this->nchild == 2);
	lv1 = rpt_expr_evaluate(this->child[0]);
	if (lv1->method->type == rpt_value_type_error)
	{
		trace((/*{*/"}\n"));
		return lv1;
	}

	/*
	 * coerce the left hand side to an arithmetic type
	 *	(will not give error if can't, will copy instead)
	 */
	lv2 = rpt_value_arithmetic(lv1);
	rpt_value_free(lv1);

	/*
	 * evaluate the right hand side
	 */
	rv1 = rpt_expr_evaluate(this->child[1]);
	if (rv1->method->type == rpt_value_type_error)
	{
		rpt_value_free(lv2);
		trace((/*{*/"}\n"));
		return rv1;
	}

	/*
	 * coerce the right hand side to an arithmetic type
	 *	(will not give error if can't, will copy instead)
	 */
	rv2 = rpt_value_arithmetic(rv1);
	rpt_value_free(rv1);

	/*
	 * what to do depends on
	 * the types of the operands
	 */
	switch (PAIR(lv2->method->type, rv2->method->type))
	{
	case PAIR(rpt_value_type_real, rpt_value_type_real):
		vp =
			rpt_value_boolean
			(
				rpt_value_real_query(lv2)
			!=
				rpt_value_real_query(rv2)
			);
		break;

	case PAIR(rpt_value_type_real, rpt_value_type_integer):
		vp =
			rpt_value_boolean
			(
				rpt_value_real_query(lv2)
			!=
				rpt_value_integer_query(rv2)
			);
		break;

	case PAIR(rpt_value_type_integer, rpt_value_type_real):
		vp =
			rpt_value_boolean
			(
				rpt_value_integer_query(lv2)
			!=
				rpt_value_real_query(rv2)
			);
		break;

	case PAIR(rpt_value_type_integer, rpt_value_type_integer):
		vp =
			rpt_value_boolean
			(
				rpt_value_integer_query(lv2)
			!=
				rpt_value_integer_query(rv2)
			);
		break;

	case PAIR(rpt_value_type_string, rpt_value_type_string):
		vp =
			rpt_value_boolean
			(
				!str_equal
				(
					rpt_value_string_query(lv2),
					rpt_value_string_query(rv2)
				)
			);
		break;

	default:
		vp =
			rpt_value_error
			(
				"illegal comparison (%s != %s)",
				lv2->method->name,
				rv2->method->name
			);
		rpt_expr_error(this, "%S", rpt_value_error_query(vp));
		break;
	}
	rpt_value_free(lv2);
	rpt_value_free(rv2);
	trace(("return %08lX;\n", (long)vp));
	trace((/*{*/"}\n"));
	return vp;
}


static rpt_expr_method_ty ne_method =
{
	sizeof(rpt_expr_ty),
	"not equal to",
	0, /* construct */
	0, /* destruct */
	ne_evaluate,
	0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_ne(a, b)
	rpt_expr_ty	*a;
	rpt_expr_ty	*b;
{
	rpt_expr_ty	*this;

	this = rpt_expr_alloc(&ne_method);
	rpt_expr_append(this, a);
	rpt_expr_append(this, b);
	return this;
}


static rpt_value_ty *match_evaluate _((rpt_expr_ty *));

static rpt_value_ty *
match_evaluate(this)
	rpt_expr_ty	*this;
{
	rpt_value_ty	*lv1;
	rpt_value_ty	*lv2;
	rpt_value_ty	*rv1;
	rpt_value_ty	*rv2;
	rpt_value_ty	*vp;

	/*
	 * evaluate the left hand side
	 */
	trace(("match::evaluate()\n{\n"/*}*/));
	assert(this->nchild == 2);
	lv1 = rpt_expr_evaluate(this->child[0]);
	if (lv1->method->type == rpt_value_type_error)
	{
		trace((/*{*/"}\n"));
		return lv1;
	}

	/*
	 * coerce the left hand side to a string
	 *	(will not give error if can't, will copy instead)
	 */
	lv2 = rpt_value_stringize(lv1);
	rpt_value_free(lv1);

	/*
	 * evaluate the right hand side
	 */
	rv1 = rpt_expr_evaluate(this->child[1]);
	if (rv1->method->type == rpt_value_type_error)
	{
		rpt_value_free(lv2);
		trace((/*{*/"}\n"));
		return rv1;
	}

	/*
	 * coerce the right hand side to a string
	 *	(will not give error if can't, will copy instead)
	 */
	rv2 = rpt_value_stringize(rv1);
	rpt_value_free(rv1);

	/*
	 * what to do depends on
	 * the types of the operands
	 */
	switch (PAIR(lv2->method->type, rv2->method->type))
	{
	case PAIR(rpt_value_type_string, rpt_value_type_string):
		vp =
			rpt_value_real
			(
				fstrcmp
				(
					rpt_value_string_query(lv2)->str_text,
					rpt_value_string_query(rv2)->str_text
				)
			);
		break;

	default:
		vp =
			rpt_value_error
			(
				"illegal comparison (%s ~~ %s)",
				lv2->method->name,
				rv2->method->name
			);
		rpt_expr_error(this, "%S", rpt_value_error_query(vp));
		break;
	}
	rpt_value_free(lv2);
	rpt_value_free(rv2);
	trace(("return %08lX;\n", (long)vp));
	trace((/*{*/"}\n"));
	return vp;
}


static rpt_expr_method_ty match_method =
{
	sizeof(rpt_expr_ty),
	"match",
	0, /* construct */
	0, /* destruct */
	match_evaluate,
	0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_match(a, b)
	rpt_expr_ty	*a;
	rpt_expr_ty	*b;
{
	rpt_expr_ty	*this;

	this = rpt_expr_alloc(&match_method);
	rpt_expr_append(this, a);
	rpt_expr_append(this, b);
	return this;
}


static rpt_value_ty *nmatch_evaluate _((rpt_expr_ty *));

static rpt_value_ty *
nmatch_evaluate(this)
	rpt_expr_ty	*this;
{
	rpt_value_ty	*lv1;
	rpt_value_ty	*lv2;
	rpt_value_ty	*rv1;
	rpt_value_ty	*rv2;
	rpt_value_ty	*vp;

	/*
	 * evaluate the left hand side
	 */
	trace(("nmatch::evaluate()\n{\n"/*}*/));
	assert(this->nchild == 2);
	lv1 = rpt_expr_evaluate(this->child[0]);
	if (lv1->method->type == rpt_value_type_error)
	{
		trace((/*{*/"}\n"));
		return lv1;
	}

	/*
	 * coerce the left hand side to a string
	 *	(will not give error if can't, will copy instead)
	 */
	lv2 = rpt_value_arithmetic(lv1);
	rpt_value_free(lv1);

	/*
	 * evaluate the right hand side
	 */
	rv1 = rpt_expr_evaluate(this->child[1]);
	if (rv1->method->type == rpt_value_type_error)
	{
		rpt_value_free(lv2);
		trace((/*{*/"}\n"));
		return rv1;
	}

	/*
	 * coerce the right hand side to a string
	 *	(will not give error if can't, will copy instead)
	 */
	rv2 = rpt_value_arithmetic(rv1);
	rpt_value_free(rv1);

	/*
	 * what to do depends on
	 * the types of the operands
	 */
	switch (PAIR(lv2->method->type, rv2->method->type))
	{
	case PAIR(rpt_value_type_string, rpt_value_type_string):
		vp =
			rpt_value_real
			(
				1
			-
				fstrcmp
				(
					rpt_value_string_query(lv2)->str_text,
					rpt_value_string_query(rv2)->str_text
				)
			);
		break;

	default:
		vp =
			rpt_value_error
			(
				"illegal comparison (%s !~ %s)",
				lv2->method->name,
				rv2->method->name
			);
		rpt_expr_error(this, "%S", rpt_value_error_query(vp));
		break;
	}
	rpt_value_free(lv2);
	rpt_value_free(rv2);
	trace(("return %08lX;\n", (long)vp));
	trace((/*{*/"}\n"));
	return vp;
}


static rpt_expr_method_ty nmatch_method =
{
	sizeof(rpt_expr_ty),
	"does not match",
	0, /* construct */
	0, /* destruct */
	nmatch_evaluate,
	0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_nmatch(a, b)
	rpt_expr_ty	*a;
	rpt_expr_ty	*b;
{
	rpt_expr_ty	*this;

	this = rpt_expr_alloc(&nmatch_method);
	rpt_expr_append(this, a);
	rpt_expr_append(this, b);
	return this;
}
