//
//	aegis - project change supervisor
//	Copyright (C) 1994, 2003, 2004 Peter Miller.
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to impliment the builtin terse function
//

#include <aer/expr.h>
#include <aer/func/terse.h>
#include <aer/value/boolean.h>
#include <error.h>
#include <option.h>


static int
valid(rpt_expr_ty *ep)
{
	return (ep->nchild == 0);
}


static rpt_value_ty *
run(rpt_expr_ty *ep, size_t argc, rpt_value_ty **argv)
{
	assert(argc == 0);
	return rpt_value_boolean(option_terse_get());
}


rpt_func_ty rpt_func_terse =
{
	"terse",
	0, // not constant
	valid,
	run,
};
