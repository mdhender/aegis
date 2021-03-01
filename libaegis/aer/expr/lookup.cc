//
//	aegis - project change supervisor
//	Copyright (C) 1994-1996, 2002-2008 Peter Miller
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

#include <libaegis/aer/expr/constant.h>
#include <libaegis/aer/expr/lookup.h>
#include <libaegis/aer/lex.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/list.h>
#include <libaegis/aer/value/ref.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/aer/value/struct.h>
#include <common/error.h>
#include <common/str.h>
#include <libaegis/sub.h>
#include <common/trace.h>


rpt_expr_lookup::~rpt_expr_lookup()
{
}


rpt_expr_lookup::rpt_expr_lookup(const rpt_expr::pointer &e1,
    const rpt_expr::pointer &e2)
{
    append(e1);
    append(e2);
}


rpt_expr::pointer
rpt_expr_lookup::create(const rpt_expr::pointer &e1,
    const rpt_expr::pointer &e2)
{
    return pointer(new rpt_expr_lookup(e1, e2));
}


rpt_expr::pointer
rpt_expr_lookup::create(const rpt_expr::pointer &e1, const nstring &name)
{
    rpt_value::pointer vp = rpt_value_string::create(name);
    rpt_expr::pointer e2 = rpt_expr_constant::create(vp);
    e2->pos_from_lex();

    return pointer(new rpt_expr_lookup(e1, e2));
}


rpt_value::pointer
rpt_expr_lookup::evaluate()
    const
{
    trace(("expr_lookup::evaluate(this = %08lX)\n{\n", (long)this));
    if (get_pos())
    {
        trace(("pos is %s\n", get_pos()->representation().c_str()));
    }
    assert(get_nchildren() == 2);
    trace(("evaluate lhs\n"));
    rpt_value::pointer lhs = nth_child(0)->evaluate(true, false);
    trace(("lhs is a %s\n", lhs->name()));
    if (lhs->is_an_error())
    {
	trace(("}\n"));
	return lhs;
    }

    trace(("evaluate rhs\n"));
    rpt_value::pointer rhs = nth_child(1)->evaluate(true, true);
    trace(("rhs is a %s\n", rhs->name()));
    if (rhs->is_an_error())
    {
	trace(("}\n"));
	return rhs;
    }
    rpt_value_reference *lhs_r = dynamic_cast<rpt_value_reference *>(lhs.get());
    if (lhs_r)
    {
	trace(("make sure lvalue is a struct\n"));
	rpt_value::pointer vp1 = lhs_r->get();
        trace(("vp1 is a %s\n", vp1->name()));
        if (dynamic_cast<rpt_value_list *>(vp1.get()))
            lhs_r = 0;
        else if (!vp1->is_a_struct())
	{
	    trace(("create and assign empty struct\n"));
	    rpt_value::pointer vp2 = rpt_value_struct::create();
	    lhs_r->set(vp2);
	}
    }

    //
    // do the lookup
    //
    trace(("do the lookup\n"));
    rpt_value::pointer result = lhs->lookup(rhs, !!lhs_r);
    rpt_value_error *result_e = dynamic_cast<rpt_value_error *>(result.get());
    if (result_e)
    {
	assert(get_pos());
	result_e->setpos(get_pos());
    }

    trace(("}\n"));
    return result;
}


bool
rpt_expr_lookup::lvalue()
    const
{
    assert(get_nchildren() == 2);
    return nth_child(0)->lvalue();
}
