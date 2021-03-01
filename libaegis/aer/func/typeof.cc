//
//      aegis - project change supervisor
//      Copyright (C) 1994, 1996, 2003-2008 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/error.h>
#include <common/str.h>
#include <libaegis/aer/expr.h>
#include <libaegis/aer/func/typeof.h>
#include <libaegis/aer/value/string.h>


rpt_func_typeof::~rpt_func_typeof()
{
}


rpt_func_typeof::rpt_func_typeof()
{
}


rpt_func::pointer
rpt_func_typeof::create()
{
    return pointer(new rpt_func_typeof());
}


const char *
rpt_func_typeof::name()
    const
{
    return "typeof";
}


bool
rpt_func_typeof::optimizable()
    const
{
    return true;
}

bool
rpt_func_typeof::verify(const rpt_expr::pointer &ep)
    const
{
    return (ep->get_nchildren() == 1);
}


rpt_value::pointer
rpt_func_typeof::run(const rpt_expr::pointer &, size_t,
    rpt_value::pointer *argv) const
{
    rpt_value::pointer arg = rpt_value::undefer(argv[0]);
    return rpt_value_string::create(arg->type_of());
}
