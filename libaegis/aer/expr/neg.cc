//
//	aegis - project change supervisor
//	Copyright (C) 1994-1996, 1999, 2002-2008 Peter Miller
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
#include <common/trace.h>
#include <libaegis/aer/expr/neg.h>
#include <libaegis/aer/value/boolean.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/real.h>
#include <libaegis/sub.h>


rpt_expr_neg::~rpt_expr_neg()
{
}


rpt_expr_neg::rpt_expr_neg(const rpt_expr::pointer &arg)
{
    append(arg);
}


rpt_expr::pointer
rpt_expr_neg::create(const rpt_expr::pointer &arg)
{
    return pointer(new rpt_expr_neg(arg));
}


rpt_value::pointer
rpt_expr_neg::evaluate()
    const
{
    //
    // evaluate the argument
    //
    trace(("rpt_expr_neg::evaluate()\n"));
    assert(get_nchildren() == 1);
    rpt_value::pointer v1 = nth_child(0)->evaluate(true, true);
    if (v1->is_an_error())
	return v1;

    //
    // coerce the argument to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    rpt_value::pointer v2 = rpt_value::arithmetic(v1);

    //
    // the type of the result depends on
    // the types of the argument
    //
    rpt_value_integer *v2ip = dynamic_cast<rpt_value_integer *>(v2.get());
    if (v2ip)
        return rpt_value_integer::create(-v2ip->query());

    rpt_value_real *v2rp = dynamic_cast<rpt_value_real *>(v2.get());
    if (v2rp)
        return rpt_value_real::create(-v2rp->query());

    sub_context_ty sc;
    sc.var_set_charstar("Name", v2->name());
    nstring s(sc.subst_intl(i18n("illegal negative ($name)")));
    return rpt_value_error::create(nth_child(0)->get_pos(), s);
}


rpt_expr_pos::~rpt_expr_pos()
{
}


rpt_expr_pos::rpt_expr_pos(const rpt_expr::pointer &arg)
{
    append(arg);
}


rpt_expr::pointer
rpt_expr_pos::create(const rpt_expr::pointer &arg)
{
    return pointer(new rpt_expr_pos(arg));
}


rpt_value::pointer
rpt_expr_pos::evaluate()
    const
{
    //
    // evaluate the argument
    //
    trace(("pos::evaluate()\n"));
    assert(get_nchildren() == 1);
    rpt_value::pointer v1 = nth_child(0)->evaluate(true, true);
    if (v1->is_an_error())
	return v1;

    //
    // coerce the argument to an arithmetic type
    //	(will not give error if can't, will copy instead)
    //
    rpt_value::pointer v2 = rpt_value::arithmetic(v1);

    //
    // it's an error if it isn't an integer or a real
    //
    rpt_value_integer *v2ip = dynamic_cast<rpt_value_integer *>(v2.get());
    if (v2ip)
        return v2;

    rpt_value_real *v2rp = dynamic_cast<rpt_value_real *>(v2.get());
    if (v2rp)
        return v2;

    sub_context_ty sc;
    sc.var_set_charstar("Name", v2->name());
    nstring s(sc.subst_intl(i18n("illegal positive ($name)")));
    return rpt_value_error::create(nth_child(0)->get_pos(), s);
}
