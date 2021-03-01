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
 * MANIFEST: functions to implement the builtin title function
 */

#include <aer/expr.h>
#include <aer/func/title.h>
#include <aer/value/error.h>
#include <aer/value/string.h>
#include <aer/value/void.h>
#include <col.h>
#include <str.h>


static int verify _((rpt_expr_ty *));

static int
verify(ep)
	rpt_expr_ty	*ep;
{
	return (ep->nchild <= 2);
}


static rpt_value_ty *run _((rpt_expr_ty *, size_t, rpt_value_ty **));

static rpt_value_ty *
run(ep, argc, argv)
	rpt_expr_ty	*ep;
	size_t		argc;
	rpt_value_ty	**argv;
{
	string_ty	*t1;
	string_ty	*t2;
	rpt_value_ty	*vp;
	rpt_value_ty	*vp2;

	if (argc >= 1)
	{
		vp = rpt_value_stringize(argv[0]);
		switch (vp->method->type)
		{
		case rpt_value_type_string:
			t1 = str_copy(rpt_value_string_query(vp));
			break;

		case rpt_value_type_error:
			t1 = str_copy(rpt_value_error_query(vp));
			break;

		default:
			vp2 =
				rpt_value_error
				(
					"unable to print %s value",
					vp->method->name
				);
			rpt_expr_error(ep, "%S", rpt_value_error_query(vp2));
			t1 = str_copy(rpt_value_error_query(vp2));
			rpt_value_free(vp2);
			break;
		}
		rpt_value_free(vp);
	}
	else
		t1 = str_from_c("");

	if (argc >= 2)
	{
		vp = rpt_value_stringize(argv[1]);
		switch (vp->method->type)
		{
		case rpt_value_type_string:
			t2 = str_copy(rpt_value_string_query(vp));
			break;

		case rpt_value_type_error:
			t2 = str_copy(rpt_value_error_query(vp));
			break;

		default:
			vp2 =
				rpt_value_error
				(
					"unable to print %s value",
					vp->method->name
				);
			rpt_expr_error(ep, "%S", rpt_value_error_query(vp2));
			t2 = str_copy(rpt_value_error_query(vp2));
			rpt_value_free(vp2);
			break;
		}
		rpt_value_free(vp);
	}
	else
		t2 = str_from_c("");

	col_title(t1->str_text, t2->str_text);
	str_free(t1);
	str_free(t2);
	return rpt_value_void();
}


rpt_func_ty rpt_func_title =
{
	"title",
	0, /* not optimizable */
	verify,
	run
};
