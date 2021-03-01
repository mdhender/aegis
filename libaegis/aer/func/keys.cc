//
//	aegis - project change supervisor
//	Copyright (C) 1994, 1996, 2004-2008 Peter Miller
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
#include <libaegis/aer/func/keys.h>
#include <libaegis/aer/value.h>
#include <libaegis/aer/value/error.h>


rpt_func_keys::~rpt_func_keys()
{
}


rpt_func_keys::rpt_func_keys()
{
}


rpt_func::pointer
rpt_func_keys::create()
{
    return pointer(new rpt_func_keys());
}


const char *
rpt_func_keys::name()
    const
{
    return "keys";
}


bool
rpt_func_keys::optimizable()
    const
{
    return true;
}


bool
rpt_func_keys::verify(const rpt_expr::pointer &ep)
    const
{
    return (ep->get_nchildren() == 1);
}


rpt_value::pointer
rpt_func_keys::run(const rpt_expr::pointer &ep, size_t,
    rpt_value::pointer *argv) const
{
    rpt_value::pointer result = argv[0]->keys();
    rpt_value_error *rve = dynamic_cast<rpt_value_error *>(result.get());
    if (rve)
    {
	assert(ep->get_pos());
	rve->setpos(ep->get_pos());
    }
    return result;
}
