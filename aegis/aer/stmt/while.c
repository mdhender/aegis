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
 * MANIFEST: functions to manipulate while statements
 */

#include <aer/expr.h>
#include <aer/lex.h>
#include <aer/stmt/while.h>
#include <aer/value/boolean.h>
#include <trace.h>


typedef struct rpt_stmt_while_ty rpt_stmt_while_ty;
struct rpt_stmt_while_ty
{
	RPT_STMT
	rpt_expr_ty	*e;
};


static void while_destruct _((rpt_stmt_ty *));

static void
while_destruct(sp)
	rpt_stmt_ty	*sp;
{
	rpt_stmt_while_ty *this;

	this = (rpt_stmt_while_ty *)sp;
	rpt_expr_free(this->e);
}


static rpt_stmt_status_ty while_run _((rpt_stmt_ty *));

static rpt_stmt_status_ty
while_run(sp)
	rpt_stmt_ty	*sp;
{
	rpt_stmt_while_ty	*this;
	rpt_value_ty	*vp;
	rpt_value_ty	*vp2;
	rpt_stmt_status_ty result;

	/*
	 * loop until you drop
	 */
	this = (rpt_stmt_while_ty *)sp;
	for (;;)
	{
		/*
		 * evaluate the looping condition
		 */
		vp = rpt_expr_evaluate(this->e);
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
				this->e,
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
	}
	return rpt_stmt_status_normal;
}


static rpt_stmt_method_ty while_method =
{
	sizeof(rpt_stmt_while_ty),
	"while",
	0, /* construct */
	while_destruct,
	while_run,
};


rpt_stmt_ty *
rpt_stmt_while(e, sub)
	rpt_expr_ty	*e;
	rpt_stmt_ty	*sub;
{
	rpt_stmt_ty	*sp;
	rpt_stmt_while_ty *this;

	sp = rpt_stmt_alloc(&while_method);
	this = (rpt_stmt_while_ty *)sp;
	this->e = rpt_expr_copy(e);
	rpt_stmt_append(sp, sub);
	return sp;
}


typedef struct rpt_stmt_do_ty rpt_stmt_do_ty;
struct rpt_stmt_do_ty
{
	RPT_STMT
	rpt_expr_ty	*e;
};


static void do_destruct _((rpt_stmt_ty *));

static void
do_destruct(sp)
	rpt_stmt_ty	*sp;
{
	rpt_stmt_do_ty	*this;

	this = (rpt_stmt_do_ty *)sp;
	rpt_expr_free(this->e);
}


static rpt_stmt_status_ty do_run _((rpt_stmt_ty *));

static rpt_stmt_status_ty
do_run(sp)
	rpt_stmt_ty	*sp;
{
	rpt_stmt_while_ty	*this;
	rpt_value_ty	*vp;
	rpt_value_ty	*vp2;
	rpt_stmt_status_ty result;

	/*
	 * loop until you drop
	 */
	this = (rpt_stmt_while_ty *)sp;
	for (;;)
	{
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
		 * evaluate the looping condition
		 */
		vp = rpt_expr_evaluate(this->e);
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
				this->e,
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
	}
	return rpt_stmt_status_normal;
}


static rpt_stmt_method_ty do_method =
{
	sizeof(rpt_stmt_do_ty),
	"do while",
	0, /* construct */
	do_destruct,
	do_run,
};


rpt_stmt_ty *
rpt_stmt_do(e, sub)
	rpt_expr_ty	*e;
	rpt_stmt_ty	*sub;
{
	rpt_stmt_ty	*sp;
	rpt_stmt_do_ty	*this;

	sp = rpt_stmt_alloc(&do_method);
	this = (rpt_stmt_do_ty *)sp;
	this->e = rpt_expr_copy(e);
	rpt_stmt_append(sp, sub);
	return sp;
}
