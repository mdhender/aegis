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
#include <libaegis/aer/expr/bit.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/sub.h>


rpt_expr_and_bit::~rpt_expr_and_bit()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


rpt_expr_and_bit::rpt_expr_and_bit(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    append(lhs);
    append(rhs);
}


rpt_expr::pointer
rpt_expr_and_bit::create(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer(new rpt_expr_and_bit(lhs, rhs));
}


rpt_value::pointer
rpt_expr_and_bit::evaluate()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    assert(get_nchildren() == 2);
    rpt_value::pointer v1 = nth_child(0)->evaluate(true, true);
    trace(("v1 is %s\n", v1->name()));
    if (v1->is_an_error())
	return v1;
    rpt_value::pointer v1i = rpt_value::integerize(v1);
    trace(("v1i is %s\n", v1i->name()));
    rpt_value_integer *v1ip = dynamic_cast<rpt_value_integer *>(v1i.get());
    if (!v1ip)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Name", v1->name());
	nstring s
        (
	    sc.subst_intl
	    (
		i18n("integer value required for bit and (was given $name)")
	    )
        );
	assert(nth_child(0)->get_pos());
	rpt_value::pointer result =
            rpt_value_error::create(nth_child(0)->get_pos(), s);
	return result;
    }
    trace(("v1ip is %ld\n", v1ip->query()));

    rpt_value::pointer v2 = nth_child(1)->evaluate(true, true);
    trace(("v2 is %s\n", v2->name()));
    if (v2->is_an_error())
	return v2;
    rpt_value::pointer v2i = rpt_value::integerize(v2);
    trace(("v2i is %s\n", v2i->name()));
    rpt_value_integer *v2ip = dynamic_cast<rpt_value_integer *>(v2i.get());
    if (!v2ip)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Name", v2->name());
	nstring s
        (
	    sc.subst_intl
	    (
		i18n("integer value required for bit and (was given $name)")
	    )
        );
        rpt_value::pointer result =
            rpt_value_error::create(nth_child(1)->get_pos(), s);
	return result;
    }
    trace(("v2ip is %ld\n", v2ip->query()));

    return rpt_value_integer::create(v1ip->query() & v2ip->query());
}


rpt_expr_xor_bit::~rpt_expr_xor_bit()
{
}


rpt_expr_xor_bit::rpt_expr_xor_bit(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    append(lhs);
    append(rhs);
}


rpt_expr::pointer
rpt_expr_xor_bit::create(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    return pointer(new rpt_expr_xor_bit(lhs, rhs));
}


rpt_value::pointer
rpt_expr_xor_bit::evaluate()
    const
{
    assert(get_nchildren() == 2);
    rpt_value::pointer v1 = nth_child(0)->evaluate(true, true);
    if (v1->is_an_error())
	return v1;
    rpt_value::pointer v1i = rpt_value::integerize(v1);
    rpt_value_integer *v1ip = dynamic_cast<rpt_value_integer *>(v1i.get());
    if (!v1ip)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Name", v1->name());
	nstring s
        (
	    sc.subst_intl
	    (
		i18n("integer value required for bit xor (was given $name)")
	    )
        );
	rpt_value::pointer result =
            rpt_value_error::create(nth_child(0)->get_pos(), s);
	return result;
    }

    rpt_value::pointer v2 = nth_child(1)->evaluate(true, true);
    if (v2->is_an_error())
	return v2;
    rpt_value::pointer v2i = rpt_value::integerize(v2);
    rpt_value_integer *v2ip = dynamic_cast<rpt_value_integer *>(v2i.get());
    if (!v2ip)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Name", v2->name());
	nstring s
        (
	    sc.subst_intl
	    (
		i18n("integer value required for bit xor (was given $name)")
	    )
        );
	rpt_value::pointer result =
            rpt_value_error::create(nth_child(1)->get_pos(), s);
	return result;
    }

    return rpt_value_integer::create(v1ip->query() ^ v2ip->query());
}


rpt_expr_or_bit::~rpt_expr_or_bit()
{
}


rpt_expr_or_bit::rpt_expr_or_bit(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    append(lhs);
    append(rhs);
}


rpt_expr::pointer
rpt_expr_or_bit::create(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    return pointer(new rpt_expr_or_bit(lhs, rhs));
}


rpt_value::pointer
rpt_expr_or_bit::evaluate()
    const
{
    assert(get_nchildren() == 2);
    rpt_value::pointer v1 = nth_child(0)->evaluate(true, true);
    if (v1->is_an_error())
	return v1;
    rpt_value::pointer v1i = rpt_value::integerize(v1);
    rpt_value_integer *v1ip = dynamic_cast<rpt_value_integer *>(v1i.get());
    if (!v1ip)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Name", v1->name());
	nstring s
        (
	    sc.subst_intl
	    (
		i18n("integer value required for bit or (was given $name)")
	    )
        );
        rpt_value::pointer result =
            rpt_value_error::create(nth_child(0)->get_pos(), s);
	return result;
    }

    rpt_value::pointer v2 = nth_child(1)->evaluate(true, true);
    if (v2->is_an_error())
	return v2;
    rpt_value::pointer v2i = rpt_value::integerize(v2);
    rpt_value_integer *v2ip = dynamic_cast<rpt_value_integer *>(v2i.get());
    if (!v2ip)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Name", v2->name());
	nstring s
        (
	    sc.subst_intl
	    (
		i18n("integer value required for bit or (was given $name)")
	    )
        );
        rpt_value::pointer result =
            rpt_value_error::create(nth_child(1)->get_pos(), s);
	return result;
    }

    return rpt_value_integer::create(v1ip->query() | v2ip->query());
}


rpt_expr_not_bit::~rpt_expr_not_bit()
{
}


rpt_expr_not_bit::rpt_expr_not_bit(const rpt_expr::pointer &arg)
{
    append(arg);
}


rpt_expr::pointer
rpt_expr_not_bit::create(const rpt_expr::pointer &arg)
{
    return pointer(new rpt_expr_not_bit(arg));
}


rpt_value::pointer
rpt_expr_not_bit::evaluate()
    const
{
    //
    // evaluate the argument
    //
    trace(("not::evaluate()\n{\n"));
    assert(get_nchildren() == 1);
    rpt_value::pointer v1 = nth_child(0)->evaluate(true, true);
    if (v1->is_an_error())
    {
	trace(("}\n"));
	return v1;
    }

    //
    // coerce the argument to an arithmetic type
    //	(will not give error if can't, will copy instead)
    //
    rpt_value::pointer v2 = rpt_value::integerize(v1);

    //
    // the type of the result depends on
    // the types of the argument
    //
    rpt_value_integer *vip = dynamic_cast<rpt_value_integer *>(v2.get());
    if (!vip)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Name", v2->name());
	nstring s(sc.subst_intl(i18n("illegal bit not ($name)")));
	rpt_value::pointer vp =
            rpt_value_error::create(nth_child(0)->get_pos(), s);
        return vp;
    }

    return rpt_value_integer::create(~vip->query());
}
