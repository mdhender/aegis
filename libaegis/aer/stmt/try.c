/*
 *	aegis - project change supervisor
 *	Copyright (C) 1996, 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate try/catch statements
 */

#include <aer/stmt/try.h>
#include <aer/value/error.h>
#include <aer/value/ref.h>
#include <aer/value/string.h>
#include <error.h>
#include <str.h>
#include <sub.h>
#include <trace.h>


typedef struct rpt_stmt_try_ty rpt_stmt_try_ty;
struct rpt_stmt_try_ty
{
	RPT_STMT
	rpt_expr_ty	*e;
};


static void run _((rpt_stmt_ty *, rpt_stmt_result_ty *));

static void
run(that, rp)
	rpt_stmt_ty	*that;
	rpt_stmt_result_ty *rp;
{
	rpt_stmt_try_ty	*this;
	rpt_value_ty	*lhs;

	/*
	 * evaluate the catch variable's address
	 */
	trace(("try::run()\n{\n"/*}*/));
	this = (rpt_stmt_try_ty *)that;
	lhs = rpt_expr_evaluate(this->e, 0);
	if (lhs->method->type == rpt_value_type_error)
	{
		rp->status = rpt_stmt_status_error;
		rp->thrown = lhs;
		trace((/*{*/"}\n"));
		return;
	}
	if (lhs->method->type != rpt_value_type_reference)
	{
		sub_context_ty	*scp;
		string_ty	*s;

		scp = sub_context_new();
		sub_var_set_charstar(scp, "Name", lhs->method->name);
		s =
			subst_intl
			(
				scp,
		       i18n("the catch variable must be modifiable (not $name)")
			);
		sub_context_delete(scp);
		rp->status = rpt_stmt_status_error;
		rp->thrown = rpt_value_error(this->e->pos, s);
		str_free(s);
		trace((/*{*/"}\n"));
		return;
	}

	/*
	 * run the try clause
	 */
	assert(this->nchild == 2);
	rpt_stmt_run(this->child[0], rp);
	if (rp->status == rpt_stmt_status_error)
	{
		rpt_value_ty	*vp;

		/*
		 * set the catch variable
		 *	to the <string> of the error
		 *	without the file position
		 */
		vp = rpt_value_string(rpt_value_error_query(rp->thrown));
		rpt_value_free(rp->thrown);
		rpt_value_reference_set(lhs, vp);
		rpt_value_free(vp);

		/*
		 * run the catch clause
		 */
		rpt_stmt_run(this->child[1], rp);
	}
	rpt_value_free(lhs);
	trace((/*{*/"}\n"));
}


static rpt_stmt_method_ty method =
{
	sizeof(rpt_stmt_try_ty),
	"try catch",
	0, /* construct */
	0, /* destruct */
	run
};


rpt_stmt_ty *
rpt_stmt_try(s1, e, s2)
	rpt_stmt_ty	*s1;
	rpt_expr_ty	*e;
	rpt_stmt_ty	*s2;
{
	rpt_stmt_ty	*that;
	rpt_stmt_try_ty	*this;

	that = rpt_stmt_alloc(&method);
	this = (rpt_stmt_try_ty *)that;
	this->e = rpt_expr_copy(e);
	rpt_stmt_append(that, s1);
	rpt_stmt_append(that, s2);
	if (!rpt_expr_lvalue(e))
		rpt_expr_parse_error(e, i18n("the catch variable must be modifiable"));
	return that;
}
