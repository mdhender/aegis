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
 * MANIFEST: functions to manipulate incriment/decriment expressions
 */

#include <aer/expr/incr.h>
#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <aer/value/real.h>
#include <aer/value/ref.h>
#include <error.h>


static rpt_value_ty *inc_pre_evaluate _((rpt_expr_ty *));

static rpt_value_ty *
inc_pre_evaluate(this)
	rpt_expr_ty	*this;
{
	rpt_value_ty	*vp;
	rpt_value_ty	*v2;
	rpt_value_ty	*v2a;
	rpt_value_ty	*result;

	assert(this->nchild == 1);
	vp = rpt_expr_evaluate(this->child[0]);
	if (vp->method->type == rpt_value_type_error)
		return vp;
	if (vp->method->type != rpt_value_type_reference)
	{
		rpt_value_free(vp);
		result =
			rpt_value_error
			(
		      "modifiable lvalue required for incriment (was given %s)",
				vp->method->name
			);
		rpt_value_free(vp);
		rpt_expr_error(this, "%S", rpt_value_error_query(result));
		return result;
	}
	
	v2 = rpt_value_reference_get(vp);
	v2a = rpt_value_arithmetic(v2);
	rpt_value_free(v2);

	switch (v2a->method->type)
	{
	case rpt_value_type_integer:
		result = rpt_value_integer(1 + rpt_value_integer_query(v2a));
		break;

	case rpt_value_type_real:
		result = rpt_value_real(1 + rpt_value_real_query(v2a));
		break;

	default:
		result =
			rpt_value_error
			(
			"arithmetic type required for incriment (was given %s)",
				v2a->method->name
			);
		break;
	}
	rpt_value_free(v2a);
	rpt_value_reference_set(vp, result);
	rpt_value_free(vp);
	return result;
}


static rpt_expr_method_ty inc_pre_method =
{
	sizeof(rpt_expr_ty),
	"++e",
	0, /* construct */
	0, /* destruct */
	inc_pre_evaluate,
	0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_inc_pre(e)
	rpt_expr_ty	*e;
{
	rpt_expr_ty	*this;

	this = rpt_expr_alloc(&inc_pre_method);
	rpt_expr_append(this, e);
	if (!rpt_expr_lvalue(e))
		rpt_expr_error(e, "modifiable lvalue required for incriment");
	return this;
}


static rpt_value_ty *dec_pre_evaluate _((rpt_expr_ty *));

static rpt_value_ty *
dec_pre_evaluate(this)
	rpt_expr_ty	*this;
{
	rpt_value_ty	*vp;
	rpt_value_ty	*v2;
	rpt_value_ty	*v2a;
	rpt_value_ty	*result;

	assert(this->nchild == 1);
	vp = rpt_expr_evaluate(this->child[0]);
	if (vp->method->type == rpt_value_type_error)
		return vp;
	if (vp->method->type != rpt_value_type_reference)
	{
		rpt_value_free(vp);
		result =
			rpt_value_error
			(
		      "modifiable lvalue required for decriment (was given %s)",
				vp->method->name
			);
		rpt_value_free(vp);
		rpt_expr_error(this, "%S", rpt_value_error_query(result));
		return result;
	}
	
	v2 = rpt_value_reference_get(vp);
	v2a = rpt_value_arithmetic(v2);
	rpt_value_free(v2);

	switch (v2a->method->type)
	{
	case rpt_value_type_integer:
		result = rpt_value_integer(rpt_value_integer_query(v2a) - 1);
		break;

	case rpt_value_type_real:
		result = rpt_value_real(rpt_value_real_query(v2a) - 1);
		break;

	default:
		result =
			rpt_value_error
			(
			"arithmetic type required for decriment (was given %s)",
				v2a->method->name
			);
		break;
	}
	rpt_value_free(v2a);
	rpt_value_reference_set(vp, result);
	rpt_value_free(vp);
	return result;
}


static rpt_expr_method_ty dec_pre_method =
{
	sizeof(rpt_expr_ty),
	"++e",
	0, /* construct */
	0, /* destruct */
	dec_pre_evaluate,
	0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_dec_pre(e)
	rpt_expr_ty	*e;
{
	rpt_expr_ty	*this;

	this = rpt_expr_alloc(&dec_pre_method);
	rpt_expr_append(this, e);
	if (!rpt_expr_lvalue(e))
		rpt_expr_error(e, "modifiable lvalue required for decriment");
	return this;
}


static rpt_value_ty *inc_post_evaluate _((rpt_expr_ty *));

static rpt_value_ty *
inc_post_evaluate(this)
	rpt_expr_ty	*this;
{
	rpt_value_ty	*vp;
	rpt_value_ty	*v2;
	rpt_value_ty	*v2a;
	rpt_value_ty	*result;

	assert(this->nchild == 1);
	vp = rpt_expr_evaluate(this->child[0]);
	if (vp->method->type == rpt_value_type_error)
		return vp;
	if (vp->method->type != rpt_value_type_reference)
	{
		rpt_value_free(vp);
		result =
			rpt_value_error
			(
		      "modifiable lvalue required for incriment (was given %s)",
				vp->method->name
			);
		rpt_value_free(vp);
		rpt_expr_error(this, "%S", rpt_value_error_query(result));
		return result;
	}
	
	v2 = rpt_value_reference_get(vp);
	v2a = rpt_value_arithmetic(v2);
	rpt_value_free(v2);

	switch (v2a->method->type)
	{
	case rpt_value_type_integer:
		result = rpt_value_integer(1 + rpt_value_integer_query(v2a));
		break;

	case rpt_value_type_real:
		result = rpt_value_real(1 + rpt_value_real_query(v2a));
		break;

	default:
		result =
			rpt_value_error
			(
			"arithmetic type required for incriment (was given %s)",
				v2a->method->name
			);
		return result;
	}
	rpt_value_reference_set(vp, result);
	rpt_value_free(vp);
	rpt_value_free(result);
	return v2a;
}


static rpt_expr_method_ty inc_post_method =
{
	sizeof(rpt_expr_ty),
	"++e",
	0, /* construct */
	0, /* destruct */
	inc_post_evaluate,
	0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_inc_post(e)
	rpt_expr_ty	*e;
{
	rpt_expr_ty	*this;

	this = rpt_expr_alloc(&inc_post_method);
	rpt_expr_append(this, e);
	if (!rpt_expr_lvalue(e))
		rpt_expr_error(e, "modifiable lvalue required for incriment");
	return this;
}


static rpt_value_ty *dec_post_evaluate _((rpt_expr_ty *));

static rpt_value_ty *
dec_post_evaluate(this)
	rpt_expr_ty	*this;
{
	rpt_value_ty	*vp;
	rpt_value_ty	*v2;
	rpt_value_ty	*v2a;
	rpt_value_ty	*result;

	assert(this->nchild == 1);
	vp = rpt_expr_evaluate(this->child[0]);
	if (vp->method->type == rpt_value_type_error)
		return vp;
	if (vp->method->type != rpt_value_type_reference)
	{
		rpt_value_free(vp);
		result =
			rpt_value_error
			(
		      "modifiable lvalue required for decriment (was given %s)",
				vp->method->name
			);
		rpt_value_free(vp);
		rpt_expr_error(this, "%S", rpt_value_error_query(result));
		return result;
	}
	
	v2 = rpt_value_reference_get(vp);
	v2a = rpt_value_arithmetic(v2);
	rpt_value_free(v2);

	switch (v2a->method->type)
	{
	case rpt_value_type_integer:
		result = rpt_value_integer(rpt_value_integer_query(v2a) - 1);
		break;

	case rpt_value_type_real:
		result = rpt_value_real(rpt_value_real_query(v2a) - 1);
		break;

	default:
		result =
			rpt_value_error
			(
			"arithmetic type required for decriment (was given %s)",
				v2a->method->name
			);
		return result;
	}
	rpt_value_reference_set(vp, result);
	rpt_value_free(vp);
	rpt_value_free(result);
	return v2a;
}


static rpt_expr_method_ty dec_post_method =
{
	sizeof(rpt_expr_ty),
	"++e",
	0, /* construct */
	0, /* destruct */
	dec_post_evaluate,
	0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_dec_post(e)
	rpt_expr_ty	*e;
{
	rpt_expr_ty	*this;

	this = rpt_expr_alloc(&dec_post_method);
	rpt_expr_append(this, e);
	if (!rpt_expr_lvalue(e))
		rpt_expr_error(e, "modifiable lvalue required for decriment");
	return this;
}
