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
#include <libaegis/aer/func/need.h>
#include <libaegis/aer/func/print.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/void.h>
#include <libaegis/col.h>


rpt_func_need::~rpt_func_need()
{
}


rpt_func_need::rpt_func_need()
{
}


rpt_func::pointer
rpt_func_need::create()
{
    return pointer(new rpt_func_need());
}


const char *
rpt_func_need::name()
    const
{
    return "need";
}


bool
rpt_func_need::optimizable()
    const
{
    return false;
}


bool
rpt_func_need::verify(const rpt_expr::pointer &ep)
    const
{
    return (ep->get_nchildren() == 1);
}


rpt_value::pointer
rpt_func_need::run(const rpt_expr::pointer &, size_t, rpt_value::pointer *argv)
    const
{
    assert(rpt_func_print__colp);
    rpt_value::pointer vp = rpt_value::integerize(argv[0]);
    rpt_value_integer *rvi = dynamic_cast<rpt_value_integer *>(vp.get());
    if (rvi)
    {
        long n = rvi->query();
        if (n > 0)
            rpt_func_print__colp->need(n);
    }
    return rpt_value_void::create();
}
