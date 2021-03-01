/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1996, 2003 Peter Miller;
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
 * MANIFEST: functions to impliment the typeof builtin function
 */

#include <aer/expr.h>
#include <aer/func/typeof.h>
#include <aer/value/string.h>
#include <error.h>
#include <str.h>


static int
validate(rpt_expr_ty *ep)
{
	return (ep->nchild == 1);
}


static rpt_value_ty *
run(rpt_expr_ty *ep, size_t argc, rpt_value_ty **argv)
{
	string_ty	*s;
	rpt_value_ty	*arg;
	rpt_value_ty	*result;

	assert(argc == 1);
	arg = rpt_value_undefer(argv[0]);
	s = str_from_c(rpt_value_typeof(arg));
	rpt_value_free(arg);
	result = rpt_value_string(s);
	str_free(s);
	return result;
}


rpt_func_ty rpt_func_typeof =
{
	"typeof",
	1, /* optimizable */
	validate,
	run
};
