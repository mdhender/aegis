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
#include <libaegis/aer/expr/shift.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/sub.h>


rpt_expr_shift_left::~rpt_expr_shift_left()
{
}


rpt_expr_shift_left::rpt_expr_shift_left(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    append(lhs);
    append(rhs);
}


rpt_expr::pointer
rpt_expr_shift_left::create(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    return pointer(new rpt_expr_shift_left(lhs, rhs));
}


rpt_value::pointer
rpt_expr_shift_left::evaluate()
    const
{
    assert(get_nchildren() == 2);
    rpt_value::pointer v1 = nth_child(0)->evaluate(true, true);
    if (v1->is_an_error())
	return v1;
    rpt_value::pointer v1i = rpt_value::integerize(v1);

    rpt_value::pointer v2 = nth_child(1)->evaluate(true, true);
    if (v2->is_an_error())
	return v2;
    rpt_value::pointer v2i = rpt_value::integerize(v2);

    rpt_value_integer *v1ip = dynamic_cast<rpt_value_integer *>(v1i.get());
    rpt_value_integer *v2ip = dynamic_cast<rpt_value_integer *>(v2i.get());
    if (!v1ip || !v2ip)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Name1", v1->name());
	sc.var_set_charstar("Name2", v2->name());
	nstring s(sc.subst_intl(i18n("illegal shift ($name1 << $name2)")));
	return rpt_value_error::create(get_pos(), s);
    }

    long v1n = v1ip->query();
    long v2n = v2ip->query();
    return rpt_value_integer::create((unsigned long)v1n << v2n);
}


rpt_expr_shift_right::~rpt_expr_shift_right()
{
}


rpt_expr_shift_right::rpt_expr_shift_right(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    append(lhs);
    append(rhs);
}


rpt_expr::pointer
rpt_expr_shift_right::create(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    return pointer(new rpt_expr_shift_right(lhs, rhs));
}


rpt_value::pointer
rpt_expr_shift_right::evaluate()
    const
{
    assert(get_nchildren() == 2);
    rpt_value::pointer v1 = nth_child(0)->evaluate(true, true);
    if (v1->is_an_error())
	return v1;
    rpt_value::pointer v1i = rpt_value::integerize(v1);

    rpt_value::pointer v2 = nth_child(1)->evaluate(true, true);
    if (v2->is_an_error())
	return v2;
    rpt_value::pointer v2i = rpt_value::integerize(v2);

    rpt_value_integer *v1ip = dynamic_cast<rpt_value_integer *>(v1i.get());
    rpt_value_integer *v2ip = dynamic_cast<rpt_value_integer *>(v2i.get());
    if (!v1ip || !v2ip)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Name1", v1->name());
	sc.var_set_charstar("Name2", v2->name());
	nstring s(sc.subst_intl(i18n("illegal shift ($name1 << $name2)")));
	return rpt_value_error::create(get_pos(), s);
    }

    long v1n = v1ip->query();
    long v2n = v2ip->query();
    return rpt_value_integer::create((unsigned long)v1n >> v2n);
}
