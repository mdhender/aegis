//
//	aegis - project change supervisor
//	Copyright (C) 1994, 1999, 2002-2004 Peter Miller;
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
// MANIFEST: functions to impliment the builtin page_width function
//

#include <aer/expr.h>
#include <aer/func/page_width.h>
#include <aer/value/integer.h>
#include <error.h>
#include <page.h>


static int
page_width_valid(rpt_expr_ty *ep)
{
    return (ep->nchild == 0);
}


static rpt_value_ty *
page_width_run(rpt_expr_ty *ep, size_t argc, rpt_value_ty **argv)
{
    assert(argc == 0);
    return rpt_value_integer(page_width_get(-1));
}


rpt_func_ty rpt_func_page_width =
{
    "page_width",
    0, // not constant
    page_width_valid,
    page_width_run,
};


static int
page_length_valid(rpt_expr_ty *ep)
{
    return (ep->nchild == 0);
}


static rpt_value_ty *
page_length_run(rpt_expr_ty *ep, size_t argc, rpt_value_ty **argv)
{
    assert(argc == 0);
    return rpt_value_integer(page_length_get(-1));
}


rpt_func_ty rpt_func_page_length =
{
    "page_length",
    0, // not constant
    page_length_valid,
    page_length_run,
};
