/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1996, 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate expression statements
 */

#include <aer/expr.h>
#include <aer/stmt/expr.h>
#include <aer/value.h>
#include <error.h>
#include <trace.h>


typedef struct rpt_stmt_expr_ty rpt_stmt_expr_ty;
struct rpt_stmt_expr_ty
{
	RPT_STMT
	rpt_expr_ty	*ep;
};


static void
destruct(rpt_stmt_ty *sp)
{
	rpt_stmt_expr_ty *this_thing;

	this_thing = (rpt_stmt_expr_ty *)sp;
	rpt_expr_free(this_thing->ep);
}


static void
run(rpt_stmt_ty *sp, rpt_stmt_result_ty *rp)
{
	rpt_stmt_expr_ty *this_thing;
	rpt_value_ty	*vp;

	trace(("stmt_expr::run()\n{\n"/*}*/));
	this_thing = (rpt_stmt_expr_ty *)sp;
	vp = rpt_expr_evaluate(this_thing->ep, 0);
	assert(vp->reference_count >= 1);
	if (vp->method->type == rpt_value_type_error)
	{
		rp->status = rpt_stmt_status_error;
		rp->thrown = vp;
	}
	else
	{
		rp->status = rpt_stmt_status_normal;
		rp->thrown = 0;
		rpt_value_free(vp);
	}
	trace((/*{*/"}\n"));
}


static rpt_stmt_method_ty method =
{
	sizeof(rpt_stmt_expr_ty),
	"expression",
	0, /* construct */
	destruct,
	run,
};


rpt_stmt_ty *
rpt_stmt_expr(rpt_expr_ty *ep)
{
	rpt_stmt_expr_ty *this_thing;

	this_thing = (rpt_stmt_expr_ty *)rpt_stmt_alloc(&method);
	this_thing->ep = rpt_expr_copy(ep);
	return (rpt_stmt_ty *)this_thing;
}
