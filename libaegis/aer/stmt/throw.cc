//
//	aegis - project change supervisor
//	Copyright (C) 1996, 1999, 2003-2005 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate throw statements
//

#include <libaegis/aer/stmt/throw.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/string.h>
#include <common/str.h>
#include <libaegis/sub.h>
#include <common/trace.h>


struct rpt_stmt_throw_ty
{
	RPT_STMT
	rpt_expr_ty	*e;
};


static void
run(rpt_stmt_ty *that, rpt_stmt_result_ty *rp)
{
	rpt_stmt_throw_ty *this_thing;
	rpt_value_ty	*vp;
	rpt_value_ty	*vp2;

	trace(("throw::run()\n{\n"));
	this_thing = (rpt_stmt_throw_ty *)that;
	vp = rpt_expr_evaluate(this_thing->e, 0);
	if (vp->method->type == rpt_value_type_error)
	{
		rp->status = rpt_stmt_status_error;
		rp->thrown = vp;
		trace(("}\n"));
		return;
	}

	vp2 = rpt_value_stringize(vp);
	rpt_value_free(vp);
	if (vp2->method->type != rpt_value_type_string)
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
		    i18n("throw statement requires string argument (not $name)")
			);
		sub_context_delete(scp);
		rp->status = rpt_stmt_status_error;
		rp->thrown = rpt_value_error(this_thing->e->pos, s);
		str_free(s);
		trace(("}\n"));
		return;
	}

	rp->status = rpt_stmt_status_error;
	rp->thrown = rpt_value_error(this_thing->e->pos,
                                     rpt_value_string_query(vp2));
	rpt_value_free(vp2);
	trace(("}\n"));
}


static rpt_stmt_method_ty method =
{
	sizeof(rpt_stmt_throw_ty),
	"throw",
	0, // construct
	0, // destruct
	run
};


rpt_stmt_ty *
rpt_stmt_throw(rpt_expr_ty *e)
{
	rpt_stmt_ty	*that;
	rpt_stmt_throw_ty *this_thing;

	that = rpt_stmt_alloc(&method);
	this_thing = (rpt_stmt_throw_ty *)that;
	this_thing->e = rpt_expr_copy(e);
	return that;
}
