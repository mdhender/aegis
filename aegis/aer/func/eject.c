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
 * MANIFEST: functions to impliment the builtin eject function
 */

#include <aer/expr.h>
#include <aer/func/eject.h>
#include <aer/value/void.h>
#include <col.h>

static int valid _((rpt_expr_ty *));

static int
valid(ep)
	rpt_expr_ty	*ep;
{
	return (ep->nchild == 0);
}


static rpt_value_ty *run _((rpt_expr_ty *, size_t, rpt_value_ty **));

static rpt_value_ty *
run(ep, argc, argv)
	rpt_expr_ty	*ep;
	size_t		argc;
	rpt_value_ty	**argv;
{
	col_eject();
	return rpt_value_void();
}


rpt_func_ty rpt_func_eject =
{
	"eject",
	0, /* not optimizable */
	valid,
	run
};