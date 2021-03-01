//
//      aegis - project change supervisor
//      Copyright (C) 1994, 2000, 2003-2008 Peter Miller
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
#include <libaegis/aer/expr.h>
#include <libaegis/aer/func/change.h>
#include <libaegis/aer/func/project.h>
#include <libaegis/aer/value/boolean.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/change/identifier.h>
#include <libaegis/project.h>
#include <libaegis/user.h>
#include <libaegis/zero.h>

static change_identifier *cidp;


rpt_func_change_number::~rpt_func_change_number()
{
}


rpt_func_change_number::rpt_func_change_number()
{
}


rpt_func::pointer
rpt_func_change_number::create()
{
    return pointer(new rpt_func_change_number());
}


const char *
rpt_func_change_number::name()
    const
{
    return "change_number";
}


bool
rpt_func_change_number::optimizable()
    const
{
    return false;
}


bool
rpt_func_change_number::verify(const rpt_expr::pointer &ep)
    const
{
    return (ep->get_nchildren() == 0);
}


rpt_value::pointer
rpt_func_change_number::run(const rpt_expr::pointer &, size_t,
    rpt_value::pointer *) const
{
    assert(cidp);
    return rpt_value_integer::create(magic_zero_decode(cidp->get_cp()->number));
}


rpt_func_change_number_set::~rpt_func_change_number_set()
{
}


rpt_func_change_number_set::rpt_func_change_number_set()
{
}


rpt_func::pointer
rpt_func_change_number_set::create()
{
    return pointer(new rpt_func_change_number_set());
}


const char *
rpt_func_change_number_set::name()
    const
{
    return "change_number_set";
}


bool
rpt_func_change_number_set::optimizable()
    const
{
    return false;
}


bool
rpt_func_change_number_set::verify(const rpt_expr::pointer &ep)
    const
{
    return (ep->get_nchildren() == 0);
}


rpt_value::pointer
rpt_func_change_number_set::run(const rpt_expr::pointer &, size_t,
    rpt_value::pointer *) const
{
    assert(cidp);
    return rpt_value_boolean::create(cidp->set());
}


void
report_parse_change_set(change_identifier &cid)
{
    cidp = &cid;
}
