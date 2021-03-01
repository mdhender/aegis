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

#include <ctype.h>

#include <aer/expr.h>
#include <aer/func/wrap.h>
#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <aer/value/list.h>
#include <aer/value/string.h>
#include <mem.h>
#include <str.h>


static int verify _((rpt_expr_ty *));

static int
verify(ep)
	rpt_expr_ty	*ep;
{
	return (ep->nchild == 2);
}


static string_ty *trim _((string_ty *));

static string_ty *
trim(s)
	string_ty	*s;
{
	static size_t	buflen;
	static char	*buf;
	char		*bp;
	char		*sp;

	if (buflen < s->str_length)
	{
		buflen = s->str_length;
		buf = mem_change_size(buf, buflen);
	}
	bp = buf;
	sp = s->str_text;
	while (*sp && *sp != '\n' && isspace(*sp))
		++sp;
	while (*sp)
	{
		if (!*sp)
			break;
		if (*sp == '\n')
		{
			*bp++ = *sp++;
			while (*sp && *sp != '\n' && isspace(*sp))
				++sp;
		}
		else if (isspace(*sp))
		{
			*bp++ = ' ';
			do
				++sp;
			while
				(*sp && *sp != '\n' && isspace(*sp));
			if (!*sp || *sp == '\n')
				--bp;
		}
		else
			*bp++ = *sp++;
	}
	return str_n_from_c(buf, bp - buf);
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
	string_ty	*s;
	long		width;
	char		*sp;

	a1 = argv[0];
	if (a1->method->type == rpt_value_type_error)
		return a1;
	a1 = rpt_value_stringize(a1);
	if (a1->method->type != rpt_value_type_string)
	{
		tmp =
			rpt_value_error
			(
		       "wrap: argument 1: string value required (was given %s)",
				a1->method->name
			);
		rpt_value_free(a1);
		return tmp;
	}
	s = trim(rpt_value_string_query(a1));
	rpt_value_free(a1);

	a2 = argv[1];
	if (a2->method->type == rpt_value_type_error)
	{
		str_free(s);
		return a2;
	}
	a2 = rpt_value_integerize(a2);
	if (a2->method->type != rpt_value_type_integer)
	{
		tmp =
			rpt_value_error
			(
		      "wrap: argument 2: integer value required (was given %s)",
				a2->method->name
			);
		rpt_value_free(a2);
		str_free(s);
		return tmp;
	}
	width = rpt_value_integer_query(a2);
	rpt_value_free(a2);
	if (width < 1)
	{
		tmp =
			rpt_value_error
			(
			     "wrap: argument 2: a width of %ld is out of range",
				a2->method->name
			);
		str_free(s);
		return tmp;
	}

	/*
	 * the result is a list
	 * create an empty one se we can start filling it
	 */
	result = rpt_value_list();

	sp = s->str_text;
	while (*sp)
	{
		char		*end_p;
		string_ty	*os;

		/*
		 * find where the line ends
		 */
		end_p = sp;
		while (end_p - sp < width && *end_p && *end_p != '\n')
			++end_p;
		if (*end_p && *end_p != '\n')
		{
			char		*w;

			/*
			 * see if there is a better place to wrap
			 */
			w = end_p;
			while (w > sp && w[-1] != ' ')
				--w;
			if (w > sp + 1)
				end_p = w - 1;
		}

		/*
		 * append the line to the result
		 */
		os = str_n_from_c(sp, end_p - sp);
		tmp = rpt_value_string(os);
		str_free(os);
		rpt_value_list_append(result, tmp);
		rpt_value_free(tmp);

		/*
		 * skip line terminator and spaces
		 */
		sp = end_p;
		if (*sp == '\n')
			++sp;
		while (*sp == ' ')
			++sp;
	}

	/*
	 * clean up and go home
	 */
	str_free(s);
	return result;
}


rpt_func_ty rpt_func_wrap =
{
	"wrap",
	1, /* optimizable */
	verify,
	run
};
