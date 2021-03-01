//
//	aegis - project change supervisor
//	Copyright (C) 1994, 1999, 2002-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/error.h>
#include <common/page.h>
#include <libaegis/aer/expr.h>
#include <libaegis/aer/func/page_width.h>
#include <libaegis/aer/value/integer.h>


rpt_func_page_width::~rpt_func_page_width()
{
}


rpt_func_page_width::rpt_func_page_width()
{
}


rpt_func::pointer
rpt_func_page_width::create()
{
    return pointer(new rpt_func_page_width());
}


const char *
rpt_func_page_width::name()
    const
{
    return "page_width";
}


bool
rpt_func_page_width::optimizable()
    const
{
    return false;
}


bool
rpt_func_page_width::verify(const rpt_expr::pointer &ep)
    const
{
    return (ep->get_nchildren() == 0);
}


rpt_value::pointer
rpt_func_page_width::run(const rpt_expr::pointer &,
    size_t, rpt_value::pointer *) const
{
    return rpt_value_integer::create(page_width_get(-1));
}


rpt_func_page_length::~rpt_func_page_length()
{
}


rpt_func_page_length::rpt_func_page_length()
{
}


rpt_func::pointer
rpt_func_page_length::create()
{
    return pointer(new rpt_func_page_length());
}


const char *
rpt_func_page_length::name()
    const
{
    return "page_length";
}


bool
rpt_func_page_length::optimizable()
    const
{
    return false;
}


bool
rpt_func_page_length::verify(const rpt_expr::pointer &ep)
    const
{
    return (ep->get_nchildren() == 0);
}


rpt_value::pointer
rpt_func_page_length::run(const rpt_expr::pointer &, size_t,
    rpt_value::pointer *) const
{
    return rpt_value_integer::create(page_length_get(-1));
}
