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
 * MANIFEST: functions to implement the builtin sort function
 */

#include <ac/stdlib.h>
#include <ac/string.h>

#include <aer/expr.h>
#include <aer/func/sort.h>
#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <aer/value/list.h>
#include <aer/value/real.h>
#include <aer/value/string.h>
#include <mem.h>
#include <str.h>


#define PAIR(a, b) ((a) * rpt_value_type_MAX + (b))


static int verify _((rpt_expr_ty *));

static int
verify(ep)
	rpt_expr_ty	*ep;
{
	return (ep->nchild == 1);
}


static int cmp _((const void *, const void *));

static int
cmp(va, vb)
	const void	*va;
	const void	*vb;
{
	rpt_value_ty	*a;
	rpt_value_ty	*b;
	int		result;

	a = *(rpt_value_ty **)va;
	b = *(rpt_value_ty **)vb;
	switch (PAIR(a->method->type, b->method->type))
	{
	default:
		{
			int na = a->method->type;
			int nb = a->method->type;
			if (na < nb)
				result = -1;
			else if (na > nb)
				result = 1;
			else
				result = 0;
		}
		break;

	case PAIR(rpt_value_type_string, rpt_value_type_string):
		result =
			strcmp
			(
				rpt_value_string_query(a)->str_text,
				rpt_value_string_query(b)->str_text
			);
		break;

	case PAIR(rpt_value_type_integer, rpt_value_type_integer):
		{
			long na = rpt_value_integer_query(a);
			long nb = rpt_value_integer_query(b);
			if (na < nb)
				result = -1;
			else if (na > nb)
				result = 1;
			else
				result = 0;
		}
		break;

	case PAIR(rpt_value_type_integer, rpt_value_type_real):
		{
			long na = rpt_value_integer_query(a);
			double nb = rpt_value_real_query(b);
			if (na < nb)
				result = -1;
			else if (na > nb)
				result = 1;
			else
				result = 0;
		}
		break;

	case PAIR(rpt_value_type_real, rpt_value_type_integer):
		{
			double na = rpt_value_real_query(a);
			long nb = rpt_value_integer_query(b);
			if (na < nb)
				result = -1;
			else if (na > nb)
				result = 1;
			else
				result = 0;
		}
		break;

	case PAIR(rpt_value_type_real, rpt_value_type_real):
		{
			double na = rpt_value_real_query(a);
			double nb = rpt_value_real_query(b);
			if (na < nb)
				result = -1;
			else if (na > nb)
				result = 1;
			else
				result = 0;
		}
		break;
	}
	return result;
}


static rpt_value_ty *run _((rpt_expr_ty *, size_t, rpt_value_ty **));

static rpt_value_ty *
run(ep, argc, argv)
	rpt_expr_ty	*ep;
	size_t		argc;
	rpt_value_ty	**argv;
{
	rpt_value_ty	*result;
	rpt_value_ty	*vp;
	long		length;
	rpt_value_ty	**item;
	long		j;

	if (argc != 1)
	{
		result =
			rpt_value_error
			(
			     "sort requires exactly 1 argument (was given %ld)",
				(long)argc
			);
		goto done;
	}
	vp = argv[0];
	if (vp->method->type != rpt_value_type_list)
	{
		result =
			rpt_value_error
			(
				"sort requires a list argument (was given %s)",
				vp->method->name
			);
		goto done;
	}

	/*
	 * sort the list items
	 */
	length = rpt_value_list_length(vp);
	item = mem_alloc(length * sizeof(rpt_value_ty *));
	for (j = 0; j < length; ++j)
		item[j] = rpt_value_arithmetic(rpt_value_list_nth(vp, j));
	qsort(item, length, sizeof(item[0]), cmp);
	result = rpt_value_list();
	for (j = 0; j < length; ++j)
	{
		rpt_value_list_append(result, item[j]);
		rpt_value_free(item[j]);
	}
	mem_free(item);

	/*
	 * here for all exits
	 */
	done:
	if (result->method->type == rpt_value_type_error)
		rpt_expr_error(ep, "%S", rpt_value_error_query(result));
	return result;
}


rpt_func_ty rpt_func_sort =
{
	"sort",
	1, /* optimizable */
	verify,
	run,
};
