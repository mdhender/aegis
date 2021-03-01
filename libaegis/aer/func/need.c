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
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to impliment the builtin need function
 */

#include <aer/expr.h>
#include <aer/func/need.h>
#include <aer/func/print.h>
#include <aer/value/integer.h>
#include <aer/value/void.h>
#include <col.h>
#include <error.h>


static int valid _((rpt_expr_ty *));

static int
valid(ep)
	rpt_expr_ty	*ep;
{
	return (ep->nchild == 1);
}


static rpt_value_ty *run _((rpt_expr_ty *, size_t, rpt_value_ty **));

static rpt_value_ty *
run(ep, argc, argv)
	rpt_expr_ty	*ep;
	size_t		argc;
	rpt_value_ty	**argv;
{
	rpt_value_ty	*vp;
	long		n;

	assert(rpt_func_print__colp);
	vp = rpt_value_integerize(argv[0]);
	if (vp->method->type == rpt_value_type_integer)
	{
		n = rpt_value_integer_query(vp);
		if (n > 0)
			col_need(rpt_func_print__colp, n);
	}
	return rpt_value_void();
}


rpt_func_ty rpt_func_need =
{
	"need",
	0, /* not optimizable */
	valid,
	run,
};
