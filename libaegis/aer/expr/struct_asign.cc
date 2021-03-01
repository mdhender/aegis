//
// aegis - project change supervisor
// Copyright (C) 1994, 1996, 2002-2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/symtab.h>
#include <libaegis/aer/expr/constant.h>
#include <libaegis/aer/expr/struct.h>
#include <libaegis/aer/expr/struct_asign.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/aer/value/struct.h>
#include <libaegis/aer/value/ref.h>
#include <libaegis/aer/value/void.h>


rpt_expr_struct_assign::~rpt_expr_struct_assign()
{
}


rpt_expr_struct_assign::rpt_expr_struct_assign(const nstring &a_name,
    const rpt_expr::pointer &a_value)
{
    rpt_expr::pointer lhs =
        rpt_expr_constant::create(rpt_value_string::create(a_name));
    lhs->pos_from(a_value);
    append(lhs);
    append(a_value);
}


rpt_expr::pointer
rpt_expr_struct_assign::create(const nstring &a_name,
    const rpt_expr::pointer &a_value)
{
    return pointer(new rpt_expr_struct_assign(a_name, a_value));
}


rpt_value::pointer
rpt_expr_struct_assign::evaluate()
    const
{
    assert(nth_child(0));
    rpt_value::pointer nvp = nth_child(0)->evaluate(true, true);
    rpt_value_string *p = dynamic_cast<rpt_value_string *>(nvp.get());
    assert(p);
    nstring nsp(p->query());

    assert(nth_child(1));
    rpt_value::pointer vp = nth_child(1)->evaluate(true, true);
    if (vp->is_an_error())
        return vp;

    rpt_value::pointer rvp = rpt_value_reference::create(vp);

    rpt_expr_struct::symtab_query()->assign(nsp, rvp);
    return rpt_value_void::create();
}


// vim: set ts=8 sw=4 et :
