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
 * MANIFEST: functions to manipulate for statements
 */

#include <aer/expr.h>
#include <aer/lex.h>
#include <aer/stmt/for.h>
#include <aer/value/boolean.h>
#include <aer/value/list.h>
#include <aer/value/ref.h>
#include <trace.h>


typedef struct rpt_stmt_for_ty rpt_stmt_for_ty;
struct rpt_stmt_for_ty
{
	RPT_STMT
	rpt_expr_ty	*e[3];
};


static void for_destruct _((rpt_stmt_ty *));

static void
for_destruct(sp)
	rpt_stmt_ty	*sp;
{
	rpt_stmt_for_ty *this;

	this = (rpt_stmt_for_ty *)sp;
	rpt_expr_free(this->e[0]);
	rpt_expr_free(this->e[1]);
	rpt_expr_free(this->e[2]);
}


static rpt_stmt_status_ty for_run _((rpt_stmt_ty *));

static rpt_stmt_status_ty
for_run(sp)
	rpt_stmt_ty	*sp;
{
	rpt_stmt_for_ty	*this;
	rpt_value_ty	*vp;
	rpt_value_ty	*vp2;
	rpt_stmt_status_ty result;

	/*
	 * evaluate the initializing expression
	 */
	this = (rpt_stmt_for_ty *)sp;
	vp = rpt_expr_evaluate(this->e[0]);
	if (vp->method->type == rpt_value_type_error)
	{
		rpt_value_free(vp);
		return rpt_stmt_status_error;
	}
	rpt_value_free(vp);

	/*
	 * loop until need to stop
	 */
	for (;;)
	{
		/*
		 * evaluate the looping condition
		 */
		vp = rpt_expr_evaluate(this->e[1]);
		if (vp->method->type == rpt_value_type_error)
		{
			rpt_value_free(vp);
			return rpt_stmt_status_error;
		}

		/*
		 * coerce the looping condition to boolean
		 *	it is an error if we can't
		 */
		vp2 = rpt_value_booleanize(vp);
		rpt_value_free(vp);
		if (vp2->method->type != rpt_value_type_boolean)
		{
			rpt_expr_error
			(
				this->e[1],
				"loop condition must be boolean (not %s)",
				vp2->method->name
			);
			rpt_value_free(vp2);
			return rpt_stmt_status_error;
		}

		/*
		 * if looping condition is false,
		 *	stop looping
		 */
		if (!rpt_value_boolean_query(vp2))
		{
			rpt_value_free(vp2);
			break;
		}
		rpt_value_free(vp2);

		/*
		 * run the inner statement
		 */
		result = rpt_stmt_run(this->child[0]);
		if (result == rpt_stmt_status_break)
			break;
		if
		(
			result != rpt_stmt_status_normal
		&&
			result != rpt_stmt_status_continue
		)
			return result;

		/*
		 * evaluate the incriment expression
		 */
		vp = rpt_expr_evaluate(this->e[2]);
		if (vp->method->type == rpt_value_type_error)
		{
			rpt_value_free(vp);
			return rpt_stmt_status_error;
		}
		rpt_value_free(vp);
	}
	return rpt_stmt_status_normal;
}


static rpt_stmt_method_ty for_method =
{
	sizeof(rpt_stmt_for_ty),
	"for",
	0, /* construct */
	for_destruct,
	for_run,
};


rpt_stmt_ty *
rpt_stmt_for(e0, e1, e2, sub)
	rpt_expr_ty	*e0;
	rpt_expr_ty	*e1;
	rpt_expr_ty	*e2;
	rpt_stmt_ty	*sub;
{
	rpt_stmt_ty	*sp;
	rpt_stmt_for_ty *this;

	sp = rpt_stmt_alloc(&for_method);
	this = (rpt_stmt_for_ty *)sp;
	this->e[0] = rpt_expr_copy(e0);
	this->e[1] = rpt_expr_copy(e1);
	this->e[2] = rpt_expr_copy(e2);
	rpt_stmt_append(sp, sub);
	return sp;
}


typedef struct rpt_stmt_foreach_ty rpt_stmt_foreach_ty;
struct rpt_stmt_foreach_ty
{
	RPT_STMT
	rpt_expr_ty	*e[2];
};


static void foreach_destruct _((rpt_stmt_ty *));

static void
foreach_destruct(sp)
	rpt_stmt_ty	*sp;
{
	rpt_stmt_foreach_ty *this;

	this = (rpt_stmt_foreach_ty *)sp;
	rpt_expr_free(this->e[0]);
	rpt_expr_free(this->e[1]);
}


static rpt_stmt_status_ty foreach_run _((rpt_stmt_ty *));

static rpt_stmt_status_ty
foreach_run(sp)
	rpt_stmt_ty	*sp;
{
	rpt_stmt_foreach_ty *this;
	rpt_value_ty	*lhs;
	rpt_value_ty	*rhs;
	size_t		rhs_length;
	size_t		j;
	rpt_value_ty	*vp;
	rpt_stmt_status_ty result;

	/*
	 * evaluate the initializing expression
	 */
	trace(("foreach::run()\n{\n"/*}*/));
	this = (rpt_stmt_foreach_ty *)sp;
	lhs = rpt_expr_evaluate(this->e[0]);
	if (lhs->method->type == rpt_value_type_error)
	{
		rpt_value_free(lhs);
		result = rpt_stmt_status_error;
		goto done;
	}
	if (lhs->method->type != rpt_value_type_reference)
	{
		rpt_expr_error
		(
			this->e[0],
			"the loop variable must be modifiable (was given %s)",
			lhs->method->name
		);
		rpt_value_free(lhs);
		result = rpt_stmt_status_error;
		goto done;
	}

	rhs = rpt_expr_evaluate(this->e[1]);
	if (rhs->method->type == rpt_value_type_error)
	{
		rpt_value_free(rhs);
		result =  rpt_stmt_status_error;
		goto done;
	}
	if (rhs->method->type != rpt_value_type_list)
	{
		rpt_expr_error
		(
			this->e[0],
		      "the loop iteration values must be a list (was given %s)",
			rhs->method->name
		);
		rpt_value_free(rhs);
		result = rpt_stmt_status_error;
		goto done;
	}

	/*
	 * loop across all values
	 */
	rhs_length = rpt_value_list_length(rhs);
	for (j = 0; j < rhs_length; ++j)
	{
		/*
		 * set the loop variable
		 */
		trace(("loop index %ld;\n", j));
		vp = rpt_value_list_nth(rhs, j);
		rpt_value_reference_set(lhs, vp);

		/*
		 * run the inner statement
		 */
		result = rpt_stmt_run(this->child[0]);
		if (result == rpt_stmt_status_break)
			break;
		if
		(
			result != rpt_stmt_status_normal
		&&
			result != rpt_stmt_status_continue
		)
			goto done;
	}
	trace(("loop complete\n"));
	rpt_value_free(rhs);
	rpt_value_free(lhs);
	result = rpt_stmt_status_normal;

	/*
	 * here for all exits
	 */
	done:
	trace(("return %d;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


static rpt_stmt_method_ty foreach_method =
{
	sizeof(rpt_stmt_foreach_ty),
	"for in",
	0, /* construct */
	foreach_destruct,
	foreach_run,
};


rpt_stmt_ty *
rpt_stmt_foreach(e0, e1, sub)
	rpt_expr_ty	*e0;
	rpt_expr_ty	*e1;
	rpt_stmt_ty	*sub;
{
	rpt_stmt_ty	*sp;
	rpt_stmt_foreach_ty *this;

	sp = rpt_stmt_alloc(&foreach_method);
	this = (rpt_stmt_foreach_ty *)sp;
	if (!rpt_expr_lvalue(e0))
		report_error("the loop control variable must be a modifiable lvalue");
	this->e[0] = rpt_expr_copy(e0);
	this->e[1] = rpt_expr_copy(e1);
	rpt_stmt_append(sp, sub);
	return sp;
}


static rpt_stmt_status_ty break_run _((rpt_stmt_ty *));

static rpt_stmt_status_ty
break_run(sp)
	rpt_stmt_ty	*sp;
{
	trace(("stmt_break::run();\n"));
	return rpt_stmt_status_break;
}


static rpt_stmt_method_ty break_method =
{
	sizeof(rpt_stmt_ty),
	"break",
	0, /* construct */
	0, /* destruct */
	break_run,
};


rpt_stmt_ty *
rpt_stmt_break()
{
	return rpt_stmt_alloc(&break_method);
}


static rpt_stmt_status_ty continue_run _((rpt_stmt_ty *));

static rpt_stmt_status_ty
continue_run(sp)
	rpt_stmt_ty	*sp;
{
	trace(("stmt_continue::run();\n"));
	return rpt_stmt_status_continue;
}


static rpt_stmt_method_ty continue_method =
{
	sizeof(rpt_stmt_ty),
	"continue",
	0, /* construct */
	0, /* destruct */
	continue_run,
};


rpt_stmt_ty *
rpt_stmt_continue()
{
	return rpt_stmt_alloc(&continue_method);
}
