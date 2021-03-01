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
 * MANIFEST: functions to implement the substr builtin function
 */

#include <aer/expr.h>
#include <aer/func/substr.h>
#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <aer/value/string.h>
#include <mem.h>
#include <str.h>


static int verify _((rpt_expr_ty *));

static int
verify(ep)
	rpt_expr_ty	*ep;
{
	return (ep->nchild == 3);
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
	string_ty	*s;
	long		start;
	long		length;
	long		end;
	string_ty	*s2;

	arg = argv[0];
	if (arg->method->type == rpt_value_type_error)
		return arg;
	arg = rpt_value_stringize(arg);
	if (arg->method->type != rpt_value_type_string)
	{
		result =
			rpt_value_error
			(
		       "wrap: argument 1: string value required (was given %s)",
				arg->method->name
			);
		rpt_value_free(arg);
		return result;
	}
	s = str_copy(rpt_value_string_query(arg));
	rpt_value_free(arg);

	arg = argv[1];
	if (arg->method->type == rpt_value_type_error)
	{
		str_free(s);
		return arg;
	}
	arg = rpt_value_integerize(arg);
	if (arg->method->type != rpt_value_type_integer)
	{
		result =
			rpt_value_error
			(
		      "wrap: argument 2: integer value required (was given %s)",
				arg->method->name
			);
		rpt_value_free(arg);
		str_free(s);
		return result;
	}
	start = rpt_value_integer_query(arg);
	rpt_value_free(arg);

	arg = argv[2];
	if (arg->method->type == rpt_value_type_error)
	{
		str_free(s);
		return arg;
	}
	arg = rpt_value_integerize(arg);
	if (arg->method->type != rpt_value_type_integer)
	{
		result =
			rpt_value_error
			(
		      "wrap: argument 3: integer value required (was given %s)",
				arg->method->name
			);
		rpt_value_free(arg);
		str_free(s);
		return result;
	}
	length = rpt_value_integer_query(arg);
	rpt_value_free(arg);

	/*
	 * clip the start end and to conform to the string
	 */
	end = start + length;
	if (start < 0)
		start = 0;
	if (end < 0)
		end = 0;
	if (start > s->str_length)
		start = s->str_length;
	if (end > s->str_length)
		end = s->str_length;
	if (end < start)
	{
		start = 0;
		end = 0;
	}

	/*
	 * build the result
	 */
	s2 = str_n_from_c(s->str_text + start, end - start);
	str_free(s);
	result = rpt_value_string(s2);
	str_free(s2);
	return result;
}


rpt_func_ty rpt_func_substr =
{
	"substr",
	1, /* optimizable */
	verify,
	run
};
