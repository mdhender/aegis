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
 * MANIFEST: functions to manipulate bit manipulation expressions
 */

#include <aer/expr/bit.h>
#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <error.h>
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

	assert(this->nchild == 2);
	v1 = rpt_expr_evaluate(this->child[0]);
	if (v1->method->type == rpt_value_type_error)
		return v1;
	v1i = rpt_value_integerize(v1);
	if (v1i->method->type != rpt_value_type_integer)
	{
		rpt_value_free(v1i);
		result =
			rpt_value_error
			(
			    "integer value required for bit and (was given %s)",
				v1->method->name
			);
		rpt_expr_error(this, "%S", rpt_value_error_query(result));
		rpt_value_free(v1);
		return result;
	}
	rpt_value_free(v1);

	v2 = rpt_expr_evaluate(this->child[1]);
	if (v2->method->type == rpt_value_type_error)
	{
		rpt_value_free(v1i);
		return v2;
	}
	v2i = rpt_value_integerize(v2);
	if (v2i->method->type != rpt_value_type_integer)
	{
		rpt_value_free(v1i);
		rpt_value_free(v2i);
		result =
			rpt_value_error
			(
			    "integer value required for bit and (was given %s)",
				v2->method->name
			);
		rpt_expr_error(this, "%S", rpt_value_error_query(result));
		rpt_value_free(v2);
		return result;
	}
	rpt_value_free(v2);

	result =
		rpt_value_integer
		(
			rpt_value_integer_query(v1i)
		&
			rpt_value_integer_query(v2i)
		);
	rpt_value_free(v1i);
	rpt_value_free(v2i);
	return result;
}


static rpt_expr_method_ty and_method =
{
	sizeof(rpt_expr_ty),
	"bitwise and",
	0, /* construct */
	0, /* destruct */
	and_evaluate,
	0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_and_bit(e1, e2)
	rpt_expr_ty	*e1;
	rpt_expr_ty	*e2;
{
	rpt_expr_ty	*this;

	this = rpt_expr_alloc(&and_method);
	rpt_expr_append(this, e1);
	rpt_expr_append(this, e2);
	return this;
}


static rpt_value_ty *xor_evaluate _((rpt_expr_ty *));

static rpt_value_ty *
xor_evaluate(this)
	rpt_expr_ty	*this;
{
	rpt_value_ty	*v1;
	rpt_value_ty	*v1i;
	rpt_value_ty	*v2;
	rpt_value_ty	*v2i;
	rpt_value_ty	*result;

	assert(this->nchild == 2);
	v1 = rpt_expr_evaluate(this->child[0]);
	if (v1->method->type == rpt_value_type_error)
		return v1;
	v1i = rpt_value_integerize(v1);
	if (v1i->method->type != rpt_value_type_integer)
	{
		rpt_value_free(v1i);
		result =
			rpt_value_error
			(
			    "integer value required for bit xor (was given %s)",
				v1->method->name
			);
		rpt_expr_error(this, "%S", rpt_value_error_query(result));
		rpt_value_free(v1);
		return result;
	}
	rpt_value_free(v1);

	v2 = rpt_expr_evaluate(this->child[1]);
	if (v2->method->type == rpt_value_type_error)
	{
		rpt_value_free(v1i);
		return v2;
	}
	v2i = rpt_value_integerize(v2);
	if (v2i->method->type != rpt_value_type_integer)
	{
		rpt_value_free(v1i);
		rpt_value_free(v2i);
		result =
			rpt_value_error
			(
			    "integer value required for bit xor (was given %s)",
				v2->method->name
			);
		rpt_expr_error(this, "%S", rpt_value_error_query(result));
		rpt_value_free(v2);
		return result;
	}
	rpt_value_free(v2);

	result =
		rpt_value_integer
		(
			rpt_value_integer_query(v1i)
		^
			rpt_value_integer_query(v2i)
		);
	rpt_value_free(v1i);
	rpt_value_free(v2i);
	return result;
}


static rpt_expr_method_ty xor_method =
{
	sizeof(rpt_expr_ty),
	"bitwise xor",
	0, /* construct */
	0, /* destruct */
	xor_evaluate,
	0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_xor_bit(e1, e2)
	rpt_expr_ty	*e1;
	rpt_expr_ty	*e2;
{
	rpt_expr_ty	*this;

	this = rpt_expr_alloc(&xor_method);
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

	assert(this->nchild == 2);
	v1 = rpt_expr_evaluate(this->child[0]);
	if (v1->method->type == rpt_value_type_error)
		return v1;
	v1i = rpt_value_integerize(v1);
	if (v1i->method->type != rpt_value_type_integer)
	{
		rpt_value_free(v1i);
		result =
			rpt_value_error
			(
			    "integer value required for bit or (was given %s)",
				v1->method->name
			);
		rpt_expr_error(this, "%S", rpt_value_error_query(result));
		rpt_value_free(v1);
		return result;
	}
	rpt_value_free(v1);

	v2 = rpt_expr_evaluate(this->child[1]);
	if (v2->method->type == rpt_value_type_error)
	{
		rpt_value_free(v1i);
		return v2;
	}
	v2i = rpt_value_integerize(v2);
	if (v2i->method->type != rpt_value_type_integer)
	{
		rpt_value_free(v1i);
		rpt_value_free(v2i);
		result =
			rpt_value_error
			(
			    "integer value required for bit or (was given %s)",
				v2->method->name
			);
		rpt_expr_error(this, "%S", rpt_value_error_query(result));
		rpt_value_free(v2);
		return result;
	}
	rpt_value_free(v2);

	result =
		rpt_value_integer
		(
			rpt_value_integer_query(v1i)
		|
			rpt_value_integer_query(v2i)
		);
	rpt_value_free(v1i);
	rpt_value_free(v2i);
	return result;
}


static rpt_expr_method_ty or_method =
{
	sizeof(rpt_expr_ty),
	"bitwise or",
	0, /* construct */
	0, /* destruct */
	or_evaluate,
	0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_or_bit(e1, e2)
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
	v1 = rpt_expr_evaluate(this->child[0]);
	if (v1->method->type == rpt_value_type_error)
	{
		trace((/*{*/"}\n"));
		return v1;
	}

	/*
	 * coerce the argument to an arithmetic type
	 *	(will not give error if can't, will copy instead)
	 */
	v2 = rpt_value_integerize(v1);
	rpt_value_free(v1);

	/*
	 * the type of the result depends on
	 * the types of the argument
	 */
	if (v2->method->type == rpt_value_type_integer)
		vp = rpt_value_integer(~rpt_value_integer_query(v2));
	else
	{
		vp =
			rpt_value_error
			(
				"illegal bit not (%s)",
				v2->method->name
			);
		rpt_expr_error(this, "%S", rpt_value_error_query(vp));
	}
	rpt_value_free(v2);
	trace(("return %08lX;\n", (long)vp));
	trace((/*{*/"}\n"));
	return vp;
}


static rpt_expr_method_ty not_method =
{
	sizeof(rpt_expr_ty),
	"bitwise not",
	0, /* construct */
	0, /* destruct */
	not_evaluate,
	0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_not_bit(a)
	rpt_expr_ty	*a;
{
	rpt_expr_ty	*this;

	this = rpt_expr_alloc(&not_method);
	rpt_expr_append(this, a);
	return this;
}
