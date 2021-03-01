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
 * MANIFEST: functions to implement the builtin print function
 */

#include <aer/expr.h>
#include <aer/func/print.h>
#include <aer/value/void.h>
#include <aer/value/error.h>
#include <aer/value/string.h>
#include <col.h>
#include <str.h>


int		rpt_func_print__ncolumns;
int		*rpt_func_print__column;


static int verify _((rpt_expr_ty *));

static int
verify(ep)
	rpt_expr_ty	*ep;
{
	if (!rpt_func_print__ncolumns)
		return 0;
	
	/*
	 * you may not print with more arguments
	 * than columns defined
	 */
	return (ep->nchild <= rpt_func_print__ncolumns);
}


static rpt_value_ty *run _((rpt_expr_ty *, size_t, rpt_value_ty **));

static rpt_value_ty *
run(ep, argc, argv)
	rpt_expr_ty	*ep;
	size_t		argc;
	rpt_value_ty	**argv;
{
	size_t		j;
	rpt_value_ty	*vp;
	rpt_value_ty	*vp2;

	for (j = 0; j < argc && j < rpt_func_print__ncolumns; ++j)
	{
		vp = rpt_value_stringize(argv[j]);
		switch (vp->method->type)
		{
		case rpt_value_type_string:
			col_puts
			(
				rpt_func_print__column[j],
				rpt_value_string_query(vp)->str_text
			);
			break;

		case rpt_value_type_error:
			col_puts
			(
				rpt_func_print__column[j],
				rpt_value_error_query(vp)->str_text
			);
			break;

		default:
			vp2 =
				rpt_value_error
				(
					"unable to print %s value",
					vp->method->name
				);
			rpt_expr_error(ep, "%S", rpt_value_error_query(vp2));
			col_puts
			(
				rpt_func_print__column[j],
				rpt_value_error_query(vp2)->str_text
			);
			rpt_value_free(vp2);
			break;
		}
		rpt_value_free(vp);
	}
	col_eoln();
	return rpt_value_void();
}


rpt_func_ty rpt_func_print =
{
	"print",
	0, /* not optimizable */
	verify,
	run
};
