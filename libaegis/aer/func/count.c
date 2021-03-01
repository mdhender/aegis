/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1996 Peter Miller;
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
 * MANIFEST: functions to implement the builtin count function
 */

#include <aer/expr.h>
#include <aer/func/count.h>
#include <aer/value.h>
#include <aer/value/error.h>
#include <error.h>


static int verify _((rpt_expr_ty *));

static int
verify(ep)
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
	rpt_value_ty	*result;

	assert(argc == 1);
	result = rpt_value_count(argv[0]);
	if (result->method->type == rpt_value_type_error)
	{
		assert(ep->pos);
		rpt_value_error_setpos(result, ep->pos);
	}
	return result;
}


rpt_func_ty rpt_func_count =
{
	"count",
	1, /* optimizable */
	verify,
	run,
};
