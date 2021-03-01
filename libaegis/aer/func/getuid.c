/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001 Peter Miller;
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
 * MANIFEST: functions to implement the builtin getuid function
 */

#include <aer/expr.h>
#include <aer/func/getuid.h>
#include <aer/value/integer.h>
#include <os.h>
#include <error.h>


static int
getuid_verify(rpt_expr_ty *ep)
{
    return (ep->nchild == 0);
}


static rpt_value_ty *
getuid_run(rpt_expr_ty *ep, size_t argc, rpt_value_ty **argv)
{
    int             uid;

    assert(argc == 0);
    os_become_orig_query(&uid, (int *)0, (int *)0);
    return rpt_value_integer(uid);
}


rpt_func_ty rpt_func_getuid =
{
    "getuid",
    0, /* optimizable */
    getuid_verify,
    getuid_run,
};
