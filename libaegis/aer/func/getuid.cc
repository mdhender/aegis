//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2004-2008 Peter Miller
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
#include <libaegis/aer/func/getuid.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/os.h>


rpt_func_getuid::~rpt_func_getuid()
{
}


rpt_func_getuid::rpt_func_getuid()
{
}


rpt_func::pointer
rpt_func_getuid::create()
{
    return pointer(new rpt_func_getuid());
}


const char *
rpt_func_getuid::name()
    const
{
    return "getuid";
}


bool
rpt_func_getuid::optimizable()
    const
{
    return true;
}


bool
rpt_func_getuid::verify(const rpt_expr::pointer &ep)
    const
{
    return (ep->get_nchildren() == 0);
}


rpt_value::pointer
rpt_func_getuid::run(const rpt_expr::pointer &, size_t, rpt_value::pointer *)
    const
{
    int uid = -1;
    os_become_orig_query(&uid, (int *)0, (int *)0);
    return rpt_value_integer::create(uid);
}
