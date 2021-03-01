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
 * MANIFEST: functions to manipulate while statements
 */

#include <aer/expr.h>
#include <aer/lex.h>
#include <aer/stmt/while.h>
#include <aer/value/boolean.h>
#include <aer/value/error.h>
#include <sub.h>


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


static void while_run _((rpt_stmt_ty *, rpt_stmt_result_ty *));

static void
while_run(sp, rp)
	rpt_stmt_ty	*sp;
	rpt_stmt_result_ty *rp;
{
	rpt_stmt_while_ty *this;
	rpt_value_ty	*vp;
	rpt_value_ty	*vp2;

	/*
	 * loop until you drop
	 */
	this = (rpt_stmt_while_ty *)sp;
	for (;;)
	{
		/*
		 * evaluate the looping condition
		 */
		vp = rpt_expr_evaluate(this->e, 1);
		if (vp->method->type == rpt_value_type_error)
		{
			rp->status = rpt_stmt_status_error;
			rp->thrown = vp;
			return;
		}

		/*
		 * coerce the looping condition to boolean
		 *	it is an error if we can't
		 */
		vp2 = rpt_value_booleanize(vp);
		rpt_value_free(vp);
		if (vp2->method->type != rpt_value_type_boolean)
		{
			sub_context_ty	*scp;
			string_ty	*s;

			scp = sub_context_new();
			sub_var_set(scp, "Name", "%s", vp2->method->name);
			rpt_value_free(vp2);
			s =
				subst_intl
				(
					scp,
			      i18n("loop condition must be boolean (not $name)")
				);
			sub_context_delete(scp);
			rp->status = rpt_stmt_status_error;
			rp->thrown = rpt_value_error(this->e->pos, s);
			return;
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
		rpt_stmt_run(this->child[0], rp);
		switch (rp->status)
		{
		case rpt_stmt_status_normal:
		case rpt_stmt_status_continue:
			continue;

		case rpt_stmt_status_error:
		case rpt_stmt_status_return:
			return;

		case rpt_stmt_status_break:
			break;
		}
		break;
	}
	rp->status = rpt_stmt_status_normal;
	rp->thrown = 0;
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


static void do_run _((rpt_stmt_ty *, rpt_stmt_result_ty *));

static void
do_run(sp, rp)
	rpt_stmt_ty	*sp;
	rpt_stmt_result_ty *rp;
{
	rpt_stmt_while_ty *this;
	rpt_value_ty	*vp;
	rpt_value_ty	*vp2;

	/*
	 * loop until you drop
	 */
	this = (rpt_stmt_while_ty *)sp;
	for (;;)
	{
		/*
		 * run the inner statement
		 */
		rpt_stmt_run(this->child[0], rp);
		if (rp->status == rpt_stmt_status_break)
			break;
		if
		(
			rp->status != rpt_stmt_status_normal
		&&
			rp->status != rpt_stmt_status_continue
		)
			return;

		/*
		 * evaluate the looping condition
		 */
		vp = rpt_expr_evaluate(this->e, 1);
		if (vp->method->type == rpt_value_type_error)
		{
			rp->status = rpt_stmt_status_error;
			rp->thrown = vp;
			return;
		}

		/*
		 * coerce the looping condition to boolean
		 *	it is an error if we can't
		 */
		vp2 = rpt_value_booleanize(vp);
		rpt_value_free(vp);
		if (vp2->method->type != rpt_value_type_boolean)
		{
			sub_context_ty	*scp;
			string_ty	*s;

			scp = sub_context_new();
			sub_var_set(scp, "Name", "%s", vp2->method->name);
			rpt_value_free(vp2);
			s =
				subst_intl
				(
					scp,
			      i18n("loop condition must be boolean (not $name)")
				);
			sub_context_delete(scp);
			rp->status = rpt_stmt_status_error;
			rp->thrown = rpt_value_error(this->e->pos, s);
			return;
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
	rp->status = rpt_stmt_status_normal;
	rp->thrown = 0;
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
