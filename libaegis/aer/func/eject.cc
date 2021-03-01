//
//	aegis - project change supervisor
//	Copyright (C) 1994, 2004-2008 Peter Miller.
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

#include <common/error.h> // for assert
#include <libaegis/aer/expr.h>
#include <libaegis/aer/func/eject.h>
#include <libaegis/aer/func/print.h>
#include <libaegis/aer/value/void.h>
#include <libaegis/col.h>


rpt_func_eject::~rpt_func_eject()
{
}


rpt_func_eject::rpt_func_eject()
{
}


rpt_func::pointer
rpt_func_eject::create()
{
    return pointer(new rpt_func_eject());
}


const char *
rpt_func_eject::name()
    const
{
    return "eject";
}


bool
rpt_func_eject::optimizable()
    const
{
    return false;
}


bool
rpt_func_eject::verify(const rpt_expr::pointer &ep)
    const
{
    return (ep->get_nchildren() == 0);
}


rpt_value::pointer
rpt_func_eject::run(const rpt_expr::pointer &, size_t, rpt_value::pointer *)
    const
{
    assert(rpt_func_print__colp);
    rpt_func_print__colp->eject();
    return rpt_value_void::create();
}
