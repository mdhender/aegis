//
//	aegis - project change supervisor
//	Copyright (C) 1994, 2004, 2005 Peter Miller.
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
// MANIFEST: functions to impliment the builtin eject function
//

#include <libaegis/aer/expr.h>
#include <libaegis/aer/func/eject.h>
#include <libaegis/aer/func/print.h>
#include <libaegis/aer/value/void.h>
#include <libaegis/col.h>
#include <common/error.h> // for assert

static int
valid(rpt_expr_ty *ep)
{
    return (ep->nchild == 0);
}


static rpt_value_ty *
run(rpt_expr_ty *ep, size_t argc, rpt_value_ty **argv)
{
    assert(rpt_func_print__colp);
    col_eject(rpt_func_print__colp);
    return rpt_value_void();
}


rpt_func_ty rpt_func_eject =
{
    "eject",
    0, // not optimizable
    valid,
    run
};
