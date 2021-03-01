//
//	aegis - project change supervisor
//	Copyright (C) 1994, 2003-2008 Peter Miller.
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
#include <libaegis/aer/expr.h>
#include <libaegis/aer/func/terse.h>
#include <libaegis/aer/value/boolean.h>
#include <libaegis/option.h>


rpt_func_terse::~rpt_func_terse()
{
}


rpt_func_terse::rpt_func_terse()
{
}


rpt_func::pointer
rpt_func_terse::create()
{
    return pointer(new rpt_func_terse());
}


const char *
rpt_func_terse::name()
    const
{
    return "terse";
}


bool
rpt_func_terse::optimizable()
    const
{
    return true;
}


bool
rpt_func_terse::verify(const rpt_expr::pointer &ep)
    const
{
    return (ep->get_nchildren() == 0);
}


rpt_value::pointer
rpt_func_terse::run(const rpt_expr::pointer &, size_t, rpt_value::pointer *)
    const
{
    return rpt_value_boolean::create(option_terse_get());
}
