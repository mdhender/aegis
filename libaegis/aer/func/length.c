/*
 *	aegis - project change supervisor
 *	Copyright (C) 1995, 1996, 1999, 2003 Peter Miller;
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
 * MANIFEST: functions to implement the length builtin function
 */

#include <aer/expr.h>
#include <aer/func/length.h>
#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <aer/value/string.h>
#include <sub.h>


static int
verify(rpt_expr_ty *ep)
{
	return (ep->nchild == 1);
}


static rpt_value_ty *
run(rpt_expr_ty *ep, size_t argc, rpt_value_ty **argv)
{
	rpt_value_ty	*arg;
	rpt_value_ty	*result;

	arg = argv[0];
	if (arg->method->type == rpt_value_type_error)
		return arg;
	arg = rpt_value_stringize(arg);
	if (arg->method->type != rpt_value_type_string)
	{
		sub_context_ty	*scp;
		string_ty	*s;

		scp = sub_context_new();
		sub_var_set_charstar(scp, "Function", "length");
		sub_var_set_charstar(scp, "Number", "1");
		sub_var_set_charstar(scp, "Name", arg->method->name);
		rpt_value_free(arg);
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
