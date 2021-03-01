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
 * MANIFEST: functions to implement the builtin wrap function
 */

#include <ac/string.h>

#include <aer/expr.h>
#include <aer/func/split.h>
#include <aer/value/error.h>
#include <aer/value/list.h>
#include <aer/value/string.h>
#include <str.h>


static int verify _((rpt_expr_ty *));

static int
verify(ep)
	rpt_expr_ty	*ep;
{
	return (ep->nchild == 2);
}


static rpt_value_ty *run _((rpt_expr_ty *, size_t, rpt_value_ty **));

static rpt_value_ty *
run(ep, argc, argv)
	rpt_expr_ty	*ep;
	size_t		argc;
	rpt_value_ty	**argv;
{
	rpt_value_ty	*a1;
	rpt_value_ty	*a2;
	rpt_value_ty	*result;
	rpt_value_ty	*tmp;
	char		*sp;
	char		*sep;

	a1 = argv[0];
	if (a1->method->type == rpt_value_type_error)
		return a1;
	a1 = rpt_value_stringize(a1);
	if (a1->method->type != rpt_value_type_string)
	{
		tmp =
			rpt_value_error
			(
		      "split: argument 1: string value required (was given %s)",
				a1->method->name
			);
		rpt_value_free(a1);
		return tmp;
	}

	a2 = argv[1];
	if (a2->method->type == rpt_value_type_error)
	{
		rpt_value_free(a1);
		return a2;
	}
	a2 = rpt_value_stringize(a2);
	if (a2->method->type != rpt_value_type_string)
	{
		tmp =
			rpt_value_error
			(
		      "split: argument 2: string value required (was given %s)",
				a2->method->name
			);
		rpt_value_free(a1);
		rpt_value_free(a2);
		return tmp;
	}

	sp = rpt_value_string_query(a1)->str_text;
	sep = rpt_value_string_query(a2)->str_text;
	if (!*sep)
		sep = " \n\r\t\f\b";

	/*
	 * the result is a list
	 * create an empty one se we can start filling it
	 */
	result = rpt_value_list();

	while (*sp)
	{
		char		*end_p;
		string_ty	*os;

		/*
		 * find where the line ends
		 */
		end_p = sp;
		while (*end_p && !strchr(sep, *end_p))
			++end_p;

		/*
		 * append the line to the result
		 */
		os = str_n_from_c(sp, end_p - sp);
		tmp = rpt_value_string(os);
		str_free(os);
		rpt_value_list_append(result, tmp);
		rpt_value_free(tmp);

		/*
		 * skip the separator
		 */
		sp = end_p;
		if (*sp)
			++sp;
	}

	/*
	 * clean up and go home
	 */
	rpt_value_free(a1);
	rpt_value_free(a2);
	return result;
}


rpt_func_ty rpt_func_split =
{
	"split",
	1, /* optimizable */
	verify,
	run
};
