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
 * MANIFEST: functions to manipulate for statements
 */

#include <aer/expr.h>
#include <aer/lex.h>
#include <aer/stmt/for.h>
#include <aer/value/boolean.h>
#include <aer/value/error.h>
#include <aer/value/list.h>
#include <aer/value/ref.h>
#include <sub.h>
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


static void for_run _((rpt_stmt_ty *, rpt_stmt_result_ty *));

static void
for_run(sp, rp)
	rpt_stmt_ty	*sp;
	rpt_stmt_result_ty *rp;
{
	rpt_stmt_for_ty	*this;
	rpt_value_ty	*vp;
	rpt_value_ty	*vp2;

	/*
	 * evaluate the initializing expression
	 */
	this = (rpt_stmt_for_ty *)sp;
	vp = rpt_expr_evaluate(this->e[0], 0);
	if (vp->method->type == rpt_value_type_error)
	{
		rp->status = rpt_stmt_status_error;
		rp->thrown = vp;
		return;
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
		vp = rpt_expr_evaluate(this->e[1], 1);
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
			sub_var_set_charstar(scp, "Name", vp2->method->name);
			rpt_value_free(vp2);
			s =
				subst_intl
				(
					scp,
			      i18n("loop condition must be boolean (not $name)")
				);
			sub_context_delete(scp);
			rp->status = rpt_stmt_status_error;
			rp->thrown = rpt_value_error(this->e[1]->pos, s);
			str_free(s);
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
		case rpt_stmt_status_error:
		case rpt_stmt_status_return:
			/*
			 * immediate termination
			 */
			trace(("loop exit\n"));
			return;

		case rpt_stmt_status_normal:
		case rpt_stmt_status_continue:
			/*
			 * evaluate the increment expression
			 */
			trace(("loop normal\n"));
			vp = rpt_expr_evaluate(this->e[2], 0);
			if (vp->method->type == rpt_value_type_error)
			{
				rp->status = rpt_stmt_status_error;
				rp->thrown = vp;
				return;
			}
			rpt_value_free(vp);

			/*
			 * do another iteration
			 */
			continue;

		case rpt_stmt_status_break:
			/*
			 * drop out of the loop
			 */
			trace(("loop break\n"));
			break;
		}
		break;
	}
	rp->status = rpt_stmt_status_normal;
	rp->thrown = 0;
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


static void foreach_run _((rpt_stmt_ty *, rpt_stmt_result_ty *));

static void
foreach_run(sp, rp)
	rpt_stmt_ty	*sp;
	rpt_stmt_result_ty *rp;
{
	rpt_stmt_foreach_ty *this;
	rpt_value_ty	*lhs;
	rpt_value_ty	*rhs;
	size_t		rhs_length;
	size_t		j;
	rpt_value_ty	*vp;

	/*
	 * evaluate the initializing expression
	 */
	trace(("foreach::run()\n{\n"));
	this = (rpt_stmt_foreach_ty *)sp;
	lhs = rpt_expr_evaluate(this->e[0], 0);
	if (lhs->method->type == rpt_value_type_error)
	{
		rp->status = rpt_stmt_status_error;
		rp->thrown = lhs;
		trace(("}\n"));
		return;
	}
	if (lhs->method->type != rpt_value_type_reference)
	{
		sub_context_ty	*scp;
		string_ty	*s;

		scp = sub_context_new();
		sub_var_set_charstar(scp, "Name", lhs->method->name);
		rpt_value_free(lhs);
		s =
			subst_intl
			(
				scp,
			i18n("the loop variable must be modifiable (not $name)")
			);
		sub_context_delete(scp);
		rp->status = rpt_stmt_status_error;
		rp->thrown = rpt_value_error(this->e[0]->pos, s);
		str_free(s);
		trace(("}\n"));
		return;
	}

	rhs = rpt_expr_evaluate(this->e[1], 1);
	if (rhs->method->type == rpt_value_type_error)
	{
		rpt_value_free(lhs);
		rp->status = rpt_stmt_status_error;
		rp->thrown = rhs;
		trace(("}\n"));
		return;
	}
	if (rhs->method->type == rpt_value_type_nul)
	{
		/*
		 * pretend nul is the empty list, and do nothing
		 */
		trace(("loop list nul\n"));
		rpt_value_free(lhs);
		rpt_value_free(rhs);
		rp->status = rpt_stmt_status_normal;
		rp->thrown = 0;
		trace(("}\n"));
		return;
	}
	if (rhs->method->type != rpt_value_type_list)
	{
		sub_context_ty	*scp;
		string_ty	*s;

		rpt_value_free(lhs);
		scp = sub_context_new();
		sub_var_set_charstar(scp, "Name", rhs->method->name);
		rpt_value_free(rhs);
		s =
			subst_intl
			(
				scp,
		     i18n("the loop iteration value must be a list (not $name)")
			);
		sub_context_delete(scp);
		rp->status = rpt_stmt_status_error;
		rp->thrown = rpt_value_error(this->e[1]->pos, s);
		str_free(s);
		trace(("}\n"));
		return;
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
		rpt_stmt_run(this->child[0], rp);
		switch (rp->status)
		{
		case rpt_stmt_status_error:
		case rpt_stmt_status_return:
			/*
			 * immediate termination
			 */
			trace(("loop exit\n"));
			rpt_value_free(rhs);
			rpt_value_free(lhs);
			trace(("}\n"));
			return;

		case rpt_stmt_status_normal:
		case rpt_stmt_status_continue:
			/*
			 * do another iteration
			 */
			trace(("loop normal\n"));
			continue;

		case rpt_stmt_status_break:
			/*
			 * drop out of the loop
			 */
			trace(("loop break\n"));
			break;
		}
		break;
	}
	trace(("loop complete\n"));
	rpt_value_free(rhs);
	rpt_value_free(lhs);
	rp->status = rpt_stmt_status_normal;
	rp->thrown = 0;
	trace(("}\n"));
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
		rpt_expr_parse_error(e0, i18n("the loop variable must be modifiable"));
	this->e[0] = rpt_expr_copy(e0);
	this->e[1] = rpt_expr_copy(e1);
	rpt_stmt_append(sp, sub);
	return sp;
}


static void break_run _((rpt_stmt_ty *, rpt_stmt_result_ty *));

static void
break_run(sp, rp)
	rpt_stmt_ty	*sp;
	rpt_stmt_result_ty *rp;
{
	trace(("stmt_break::run();\n"));
	rp->status = rpt_stmt_status_break;
	rp->thrown = 0;
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


static void continue_run _((rpt_stmt_ty *, rpt_stmt_result_ty *));

static void
continue_run(sp, rp)
	rpt_stmt_ty	*sp;
	rpt_stmt_result_ty *rp;
{
	trace(("stmt_continue::run();\n"));
	rp->status = rpt_stmt_status_continue;
	rp->thrown = 0;
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
