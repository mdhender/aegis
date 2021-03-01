/*
 *	aegis - project change supervisor
 *	Copyright (C) 1998, 1999, 2001 Peter Miller;
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
 * MANIFEST: functions to implement the subst builtin function
 */

#include <aer/expr.h>
#include <aer/func/substitute.h>
#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <aer/value/string.h>
#include <error.h>
#include <mem.h>
#include <sub.h>


static int verify _((rpt_expr_ty *));

static int
verify(ep)
	rpt_expr_ty	*ep;
{
	return (ep->nchild == 3 || ep->nchild == 4);
}


static const char *error_value;


static void error_callback _((const char *));

static void
error_callback(s)
	const char	*s;
{
	error_value = s;
}


static rpt_value_ty *run _((rpt_expr_ty *, size_t, rpt_value_ty **));

static rpt_value_ty *
run(ep, argc, argv)
	rpt_expr_ty	*ep;
	size_t		argc;
	rpt_value_ty	**argv;
{
	rpt_value_ty	*arg;
	string_ty	*lhs;
	string_ty	*rhs;
	string_ty	*input;
	long		count;
	string_ty	*s;
	rpt_value_ty	*result;

	/*
	 * Get the match pattern.
	 */
	arg = argv[0];
	assert(arg->method->type != rpt_value_type_error);
	arg = rpt_value_stringize(arg);
	if (arg->method->type != rpt_value_type_string)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		rpt_value_free(arg);
		sub_var_set_charstar(scp, "Function", "subst");
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
	lhs = str_copy(rpt_value_string_query(arg));
	rpt_value_free(arg);

	/*
	 * Get the replacement pattern.
	 */
	arg = argv[1];
	assert(arg->method->type != rpt_value_type_error);
	arg = rpt_value_stringize(arg);
	if (arg->method->type != rpt_value_type_string)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		rpt_value_free(arg);
		sub_var_set_charstar(scp, "Function", "subst");
		sub_var_set_charstar(scp, "Number", "2");
		sub_var_set_charstar(scp, "Name", argv[1]->method->name);
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
	rhs = str_copy(rpt_value_string_query(arg));
	rpt_value_free(arg);

	/*
	 * Get the string to be worked over.
	 */
	arg = argv[2];
	assert(arg->method->type != rpt_value_type_error);
	arg = rpt_value_stringize(arg);
	if (arg->method->type != rpt_value_type_string)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		rpt_value_free(arg);
		sub_var_set_charstar(scp, "Function", "subst");
		sub_var_set_charstar(scp, "Number", "3");
		sub_var_set_charstar(scp, "Name", argv[2]->method->name);
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
	input = str_copy(rpt_value_string_query(arg));
	rpt_value_free(arg);

	/*
	 * Get the count of how many times to match.
	 */
	if (argc < 4)
		count = 0;
	else
	{
		arg = argv[3];
		assert(arg->method->type != rpt_value_type_error);
		arg = rpt_value_integerize(arg);
		if (arg->method->type != rpt_value_type_integer)
		{
			sub_context_ty	*scp;
	
			scp = sub_context_new();
			rpt_value_free(arg);
			sub_var_set_charstar(scp, "Function", "subst");
			sub_var_set_charstar(scp, "Number", "2");
			sub_var_set_charstar(scp, "Name", argv[3]->method->name);
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
		count = rpt_value_integer_query(arg);
		rpt_value_free(arg);
	}

	/*
	 * perform the substitution
	 */
	error_value = 0;
	s = str_re_substitute(lhs, rhs, input, error_callback, count);
	if (!s)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		rpt_value_free(arg);
		sub_var_set_charstar(scp, "Function", "subst");
		sub_var_set_charstar(scp, "Number", "1");
		sub_var_set_charstar
		(
			scp,
			"Message",
			(error_value ? error_value : "unknown")
		);
		s =
			subst_intl
			(
				scp,
				i18n("$function: argument $number: $message")
			);
		sub_context_delete(scp);
		result = rpt_value_error(ep->pos, s);
		str_free(s);
		return result;
	}

	/*
	 * build the result
	 */
	str_free(lhs);
	str_free(rhs);
	str_free(input);
	result = rpt_value_string(s);
	str_free(s);
	return result;
}


rpt_func_ty rpt_func_substitute =
{
	"subst",
	1, /* optimizable */
	verify,
	run
};
