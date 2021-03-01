/*
 *	aegis - project change supervisor
 *	Copyright (C) 1995 Peter Miller;
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
 * MANIFEST: functions to implement the length builtin function
 */

#include <aer/expr.h>
#include <aer/func/length.h>
#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <aer/value/string.h>
#include <str.h>


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
	rpt_value_ty	*arg;
	rpt_value_ty	*result;

	arg = argv[0];
	if (arg->method->type == rpt_value_type_error)
		return arg;
	arg = rpt_value_stringize(arg);
	if (arg->method->type != rpt_value_type_string)
	{
		result =
			rpt_value_error
			(
				"length: string value required (was given %s)",
				arg->method->name
			);
		rpt_value_free(arg);
		return result;
	}

	result = rpt_value_integer(rpt_value_string_query(arg)->str_length);
	rpt_value_free(arg);
	return result;
}


rpt_func_ty rpt_func_length =
{
	"length",
	1, /* optimizable */
	verify,
	run
};
