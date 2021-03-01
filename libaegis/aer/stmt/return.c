/*
 *	aegis - project change supervisor
 *	Copyright (C) 1996, 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate return statements
 */

#include <aer/expr.h>
#include <aer/stmt/return.h>
#include <aer/value/void.h>
#include <trace.h>


typedef struct rpt_stmt_return_ty rpt_stmt_return_ty;
struct rpt_stmt_return_ty
{
	RPT_STMT
	rpt_expr_ty	*e;
};


static void
run(rpt_stmt_ty *that, rpt_stmt_result_ty *rp)
{
	rpt_stmt_return_ty *this_thing;

	trace(("return::run()\n{\n"/*}*/));
	this_thing = (rpt_stmt_return_ty *)that;
	rp->status = rpt_stmt_status_return;
	if (this_thing->e)
		rp->thrown = rpt_expr_evaluate(this_thing->e, 0);
	else
		rp->thrown = rpt_value_void();
	trace((/*{*/"}\n"));
}


static rpt_stmt_method_ty method =
{
	sizeof(rpt_stmt_return_ty),
	"return",
	0, /* construct */
	0, /* destruct */
	run
};


rpt_stmt_ty *
rpt_stmt_return(rpt_expr_ty *e)
{
	rpt_stmt_ty	*that;
	rpt_stmt_return_ty *this_thing;

	that = rpt_stmt_alloc(&method);
	this_thing = (rpt_stmt_return_ty *)that;
	this_thing->e = e ? rpt_expr_copy(e) : 0;
	return that;
}
