/*
 *	aegis - project change supervisor
 *	Copyright (C) 1995, 1996, 1999, 2002, 2003 Peter Miller;
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
 * MANIFEST: functions to implement the substr builtin function
 */

#include <aer/expr.h>
#include <aer/func/substr.h>
#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <aer/value/string.h>
#include <error.h>
#include <mem.h>
#include <sub.h>


static int
verify(rpt_expr_ty *ep)
{
	return (ep->nchild == 3);
}


static rpt_value_ty *
run(rpt_expr_ty *ep, size_t argc, rpt_value_ty **argv)
{
	rpt_value_ty	*arg;
	rpt_value_ty	*result;
	string_ty	*subject;
	long		start;
	long		length;
	long		end;
	string_ty	*s;

	arg = argv[0];
	assert(arg->method->type != rpt_value_type_error);
	arg = rpt_value_stringize(arg);
	if (arg->method->type != rpt_value_type_string)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		rpt_value_free(arg);
		sub_var_set_charstar(scp, "Function", "substr");
		sub_var_set_charstar(scp, "Number", "1");
		sub_var_set_charstar(scp, "Name", argv[0]->method->name);
		s =
			subst_intl
			(
				scp,
    i18n("$function: argument $number: string value required (was given $name)")
			);
		sub_context_delete(scp);
		result = rpt_value_error(ep->pos, s);
		str_free(s);
		return result;
	}
	subject = str_copy(rpt_value_string_query(arg));
	rpt_value_free(arg);

	arg = argv[1];
	assert(arg->method->type != rpt_value_type_error);
	arg = rpt_value_integerize(arg);
	if (arg->method->type != rpt_value_type_integer)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		rpt_value_free(arg);
		sub_var_set_charstar(scp, "Function", "substr");
		sub_var_set_charstar(scp, "Number", "2");
		sub_var_set_charstar(scp, "Name", argv[1]->method->name);
		s =
			subst_intl
			(
				scp,
   i18n("$function: argument $number: integer value required (was given $name)")
			);
		sub_context_delete(scp);
		result = rpt_value_error(ep->pos, s);
		str_free(s);
		return result;
	}
	start = rpt_value_integer_query(arg);
	rpt_value_free(arg);

	arg = argv[2];
	assert(arg->method->type != rpt_value_type_error);
	arg = rpt_value_integerize(arg);
	if (arg->method->type != rpt_value_type_integer)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		rpt_value_free(arg);
		sub_var_set_charstar(scp, "Function", "substr");
		sub_var_set_charstar(scp, "Number", "3");
		sub_var_set_charstar(scp, "Name", argv[2]->method->name);
		s =
			subst_intl
			(
				scp,
   i18n("$function: argument $number: integer value required (was given $name)")
			);
		sub_context_delete(scp);
		result = rpt_value_error(ep->pos, s);
		str_free(s);
		return result;
	}
	length = rpt_value_integer_query(arg);
	rpt_value_free(arg);

	/*
	 * clip the start and end to conform to the string
	 */
	end = start + length;
	if (start < 0)
		start = 0;
	if (end < 0)
		end = 0;
	if (start > (long)subject->str_length)
		start = subject->str_length;
	if (end > (long)subject->str_length)
		end = subject->str_length;
	if (end < start)
	{
		start = 0;
		end = 0;
	}

	/*
	 * build the result
	 */
	s = str_n_from_c(subject->str_text + start, end - start);
	str_free(subject);
	result = rpt_value_string(s);
	str_free(s);
	return result;
}


rpt_func_ty rpt_func_substr =
{
	"substr",
	1, /* optimizable */
	verify,
	run
};
