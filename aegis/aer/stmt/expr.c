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


static void destruct _((rpt_stmt_ty *));

static void
destruct(sp)
	rpt_stmt_ty	*sp;
{
	rpt_stmt_expr_ty *this;

	this = (rpt_stmt_expr_ty *)sp;
	rpt_expr_free(this->ep);
}


static rpt_stmt_status_ty run _((rpt_stmt_ty *));

static rpt_stmt_status_ty
run(sp)
	rpt_stmt_ty	*sp;
{
	rpt_stmt_expr_ty *this;
	rpt_value_ty	*vp;
	rpt_stmt_status_ty result;

	trace(("stmt_expr::run()\n{\n"/*}*/));
	this = (rpt_stmt_expr_ty *)sp;
	vp = rpt_expr_evaluate(this->ep);
	if (vp->method->type == rpt_value_type_error)
		result = rpt_stmt_status_error;
	else
		result = rpt_stmt_status_normal;
	assert(vp->reference_count >= 1);
	rpt_value_free(vp);
	trace(("return %d;\n", result));
	trace((/*{*/"}\n"));
	return result;
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
rpt_stmt_expr(ep)
	rpt_expr_ty	*ep;
{
	rpt_stmt_expr_ty *this;

	this = (rpt_stmt_expr_ty *)rpt_stmt_alloc(&method);
	this->ep = rpt_expr_copy(ep);
	return (rpt_stmt_ty *)this;
}
