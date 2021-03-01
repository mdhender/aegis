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
 * MANIFEST: functions to manipulate if statements
 */

#include <aer/expr.h>
#include <aer/stmt/if.h>
#include <aer/value/boolean.h>
#include <error.h>
#include <trace.h>


typedef struct rpt_stmt_if_ty rpt_stmt_if_ty;
struct rpt_stmt_if_ty
{
	RPT_STMT
	rpt_expr_ty	*condition;
};


static void destruct _((rpt_stmt_ty *));

static void
destruct(sp)
	rpt_stmt_ty	*sp;
{
	rpt_stmt_if_ty	*this;

	this = (rpt_stmt_if_ty *)sp;
	rpt_expr_free(this->condition);
}


static rpt_stmt_status_ty run _((rpt_stmt_ty *));

static rpt_stmt_status_ty
run(sp)
	rpt_stmt_ty	*sp;
{
	rpt_stmt_if_ty	*this;
	rpt_value_ty	*vp;
	rpt_value_ty	*vp2;
	int		flag;
	rpt_stmt_status_ty result;

	trace(("stmt_if::run()\n{\n"/*}*/));
	this = (rpt_stmt_if_ty *)sp;
	assert(this->nchild == 1 || this->nchild == 2);
	vp = rpt_expr_evaluate(this->condition);
	if (vp->method->type == rpt_value_type_error)
	{
		rpt_value_free(vp);
		result = rpt_stmt_status_error;
		goto done;
	}
	vp2 = rpt_value_booleanize(vp);
	rpt_value_free(vp);
	if (vp2->method->type != rpt_value_type_boolean)
	{
		rpt_expr_error
		(
			this->condition,
			"boolean value required in if statement (was given %s)",
			vp2->method->name
		);
		rpt_value_free(vp2);
		result = rpt_stmt_status_error;
		goto done;
	}
	flag = !rpt_value_boolean_query(vp2);
	trace(("condition was %s\n", (flag ? "false" : "true")));
	rpt_value_free(vp2);
	if (flag < this->nchild)
		result = rpt_stmt_run(this->child[flag]);
	else
		result = rpt_stmt_status_normal;

	/*
	 * here for all exits
	 */
	done:
	trace(("return %d;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


static rpt_stmt_method_ty method =
{
	sizeof(rpt_stmt_if_ty),
	"if",
	0, /* construct */
	destruct,
	run,
};


rpt_stmt_ty *
rpt_stmt_if(ep)
	rpt_expr_ty	*ep;
{
	rpt_stmt_if_ty	*this;

	this = (rpt_stmt_if_ty *)rpt_stmt_alloc(&method);
	this->condition = rpt_expr_copy(ep);
	return (rpt_stmt_ty *)this;
}
