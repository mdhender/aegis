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
#include <libaegis/aer/expr/assign.h>
#include <libaegis/aer/expr/bit.h>
#include <libaegis/aer/expr/constant.h>
#include <libaegis/aer/expr/mul.h>
#include <libaegis/aer/expr/plus.h>
#include <libaegis/aer/expr/power.h>
#include <libaegis/aer/expr/shift.h>
#include <libaegis/aer/lex.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/real.h>
#include <libaegis/aer/value/ref.h>
#include <libaegis/sub.h>


rpt_expr_assign::~rpt_expr_assign()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


rpt_expr_assign::rpt_expr_assign(const rpt_expr::pointer &a,
    const rpt_expr::pointer &b)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    append(a);
    append(b);

    if (!a->lvalue())
	a->parse_error(i18n("illegal left hand side of assignment"));
}


rpt_expr::pointer
rpt_expr_assign::create(const rpt_expr::pointer &a, const rpt_expr::pointer &b)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer(new rpt_expr_assign(a, b));
}


rpt_value::pointer
rpt_expr_assign::evaluate()
    const
{
    //
    // evaluate the left hand side
    //
    trace(("rpt_expr_assign::evaluate()\n{\n"));
    assert(get_nchildren() == 2);
    rpt_value::pointer lv = nth_child(0)->evaluate(true, false);
    trace(("lv is a %s\n", lv->name()));
    if (lv->is_an_error())
    {
	trace(("}\n"));
	return lv;
    }

    //
    // if the left hand side is not a reference type,
    // it is an error
    //
    rpt_value_reference *lv_ref = dynamic_cast<rpt_value_reference *>(lv.get());
    if (!lv_ref)
    {
        trace(("oops\n"));
	sub_context_ty sc;
	sc.var_set_charstar("Name", lv->name());
	nstring s
        (
            sc.subst_intl
	    (
		i18n("illegal left hand side of assignment (was given $name)")
	    )
        );
	rpt_value::pointer vp =
            rpt_value_error::create(nth_child(0)->get_pos(), s);
	trace(("}\n"));
	return vp;
    }

    //
    // evaluate the right hand side
    //
    trace(("about to evaluate rhs\n"));
    rpt_value::pointer rv = nth_child(1)->evaluate(true, true);
    trace(("rv is a %s\n", rv->name()));
    if (rv->is_an_error())
    {
	trace(("}\n"));
	return rv;
    }

    //
    // set the value (will take copy)
    //
    trace(("set reference\n"));
    lv_ref->set(rv);

    //
    // cleanup and go home
    //
    trace(("return %08lX;\n", (long)rv.get()));
    trace(("}\n"));
    return rv;
}


static rpt_value::pointer
bin_eval(const rpt_expr::pointer &lhs,
    rpt_expr::pointer (*op)(const rpt_expr::pointer &lhs,
        const rpt_expr::pointer &rhs),
    const rpt_expr::pointer &rhs)
{
    //
    // evaluate the left hand side
    //
    trace(("bin_eval()\n{\n"));
    rpt_value::pointer ptr = lhs->evaluate(true, false);
    if (ptr->is_an_error())
    {
	trace(("}\n"));
	return ptr;
    }

    //
    // if the left hand side is not a reference type,
    // it is an error
    //
    if (!dynamic_cast<rpt_value_reference *>(ptr.get()))
    {
	sub_context_ty sc;
	sc.var_set_charstar("Name", ptr->name());
	nstring s
        (
	    sc.subst_intl
	    (
		i18n("illegal left hand side of assignment (was given $name)")
	    )
        );
        assert(lhs->get_pos());
	rpt_value::pointer result = rpt_value_error::create(lhs->get_pos(), s);

        trace(("return %08lX;\n", (long)result.get()));
        trace(("}\n"));
        return result;
    }

    //
    // construct
    //	lhs = lhs "op" rhs
    //
    // Note:
    //	e1 and e2 are separate because one is an lvalue, and
    //	the other is an rvalue.  They e1 is altered by
    //	rpt_expr_assign, while e2 is not.
    //
    rpt_expr::pointer e1 = rpt_expr_constant::create(ptr);
    e1->pos_from(lhs);
    rpt_expr::pointer e2 = rpt_expr_constant::create(ptr);
    e2->pos_from(lhs);

    //
    // use the constant expression to build a simple assignment expression
    // "op" is the appropriate binary expression builder
    //
    rpt_expr::pointer e3 = op(e2, rhs);
    rpt_expr::pointer e4 = rpt_expr_assign::create(e1, e3);

    //
    // evaluate the simple assignment
    //	and discard the fake expression tree
    //
    rpt_value::pointer result = e4->evaluate(true, true);

    //
    // clean up and go home
    //
    trace(("return %08lX;\n", (long)result.get()));
    trace(("}\n"));
    return result;
}


rpt_expr_assign_power::~rpt_expr_assign_power()
{
}


rpt_expr_assign_power::rpt_expr_assign_power(const rpt_expr::pointer &a,
    const rpt_expr::pointer &b)
{
    append(a);
    append(b);

    if (!a->lvalue())
	a->parse_error(i18n("illegal left hand side of assignment"));
}


rpt_expr::pointer
rpt_expr_assign_power::create(const rpt_expr::pointer &a,
    const rpt_expr::pointer &b)
{
    return pointer(new rpt_expr_assign_power(a, b));
}


rpt_value::pointer
rpt_expr_assign_power::evaluate()
    const
{
    trace(("power_assign::evaluate()\n{\n"));
    rpt_value::pointer result =
        bin_eval(nth_child(0), &rpt_expr_power::create, nth_child(1));
    trace(("return %08lX;\n", (long)result.get()));
    trace(("}\n"));
    return result;
}


rpt_expr_assign_mul::~rpt_expr_assign_mul()
{
}


rpt_expr_assign_mul::rpt_expr_assign_mul(const rpt_expr::pointer &a,
    const rpt_expr::pointer &b)
{
    append(a);
    append(b);

    if (!a->lvalue())
	a->parse_error(i18n("illegal left hand side of assignment"));
}


rpt_expr::pointer
rpt_expr_assign_mul::create(const rpt_expr::pointer &a,
    const rpt_expr::pointer &b)
{
    return pointer(new rpt_expr_assign_mul(a, b));
}


rpt_value::pointer
rpt_expr_assign_mul::evaluate()
    const
{
    trace(("mul_assign::evaluate()\n{\n"));
    rpt_value::pointer result =
        bin_eval(nth_child(0), &rpt_expr_mul::create, nth_child(1));
    trace(("return %08lX;\n", (long)result.get()));
    trace(("}\n"));
    return result;
}


rpt_expr_assign_div::~rpt_expr_assign_div()
{
}


rpt_expr_assign_div::rpt_expr_assign_div(const rpt_expr::pointer &a,
    const rpt_expr::pointer &b)
{
    append(a);
    append(b);

    if (!a->lvalue())
	a->parse_error(i18n("illegal left hand side of assignment"));
}


rpt_expr::pointer
rpt_expr_assign_div::create(const rpt_expr::pointer &a,
    const rpt_expr::pointer &b)
{
    return pointer(new rpt_expr_assign_div(a, b));
}


rpt_value::pointer
rpt_expr_assign_div::evaluate()
    const
{
    trace(("div_assign::evaluate()\n{\n"));
    rpt_value::pointer result =
        bin_eval(nth_child(0), &rpt_expr_div::create, nth_child(1));
    trace(("return %08lX;\n", (long)result.get()));
    trace(("}\n"));
    return result;
}


rpt_expr_assign_mod::~rpt_expr_assign_mod()
{
}


rpt_expr_assign_mod::rpt_expr_assign_mod(const rpt_expr::pointer &a,
    const rpt_expr::pointer &b)
{
    append(a);
    append(b);

    if (!a->lvalue())
	a->parse_error(i18n("illegal left hand side of assignment"));
}


rpt_expr::pointer
rpt_expr_assign_mod::create(const rpt_expr::pointer &a,
    const rpt_expr::pointer &b)
{
    return pointer(new rpt_expr_assign_mod(a, b));
}


rpt_value::pointer
rpt_expr_assign_mod::evaluate()
    const
{
    trace(("mod_assign::evaluate()\n{\n"));
    rpt_value::pointer result =
        bin_eval(nth_child(0), &rpt_expr_mod::create, nth_child(1));
    trace(("return %08lX;\n", (long)result.get()));
    trace(("}\n"));
    return result;
}


rpt_expr_assign_plus::~rpt_expr_assign_plus()
{
}


rpt_expr_assign_plus::rpt_expr_assign_plus(const rpt_expr::pointer &a,
    const rpt_expr::pointer &b)
{
    append(a);
    append(b);

    if (!a->lvalue())
	a->parse_error(i18n("illegal left hand side of assignment"));
}


rpt_expr::pointer
rpt_expr_assign_plus::create(const rpt_expr::pointer &a,
    const rpt_expr::pointer &b)
{
    return pointer(new rpt_expr_assign_plus(a, b));
}


rpt_value::pointer
rpt_expr_assign_plus::evaluate()
    const
{
    trace(("plus_assign::evaluate()\n{\n"));
    rpt_value::pointer result =
        bin_eval(nth_child(0), &rpt_expr_plus::create, nth_child(1));
    trace(("return %08lX;\n", (long)result.get()));
    trace(("}\n"));
    return result;
}


rpt_expr_assign_minus::~rpt_expr_assign_minus()
{
}


rpt_expr_assign_minus::rpt_expr_assign_minus(const rpt_expr::pointer &a,
    const rpt_expr::pointer &b)
{
    append(a);
    append(b);

    if (!a->lvalue())
	a->parse_error(i18n("illegal left hand side of assignment"));
}


rpt_expr::pointer
rpt_expr_assign_minus::create(const rpt_expr::pointer &a,
    const rpt_expr::pointer &b)
{
    return pointer(new rpt_expr_assign_minus(a, b));
}


rpt_value::pointer
rpt_expr_assign_minus::evaluate()
    const
{
    trace(("minus_assign::evaluate()\n{\n"));
    rpt_value::pointer result =
        bin_eval(nth_child(0), &rpt_expr_minus::create, nth_child(1));
    trace(("return %08lX;\n", (long)result.get()));
    trace(("}\n"));
    return result;
}


rpt_expr_assign_and_bit::~rpt_expr_assign_and_bit()
{
}


rpt_expr_assign_and_bit::rpt_expr_assign_and_bit(const rpt_expr::pointer &a,
    const rpt_expr::pointer &b)
{
    append(a);
    append(b);

    if (!a->lvalue())
	a->parse_error(i18n("illegal left hand side of assignment"));
}


rpt_expr::pointer
rpt_expr_assign_and_bit::create(const rpt_expr::pointer &a,
    const rpt_expr::pointer &b)
{
    return pointer(new rpt_expr_assign_and_bit(a, b));
}


rpt_value::pointer
rpt_expr_assign_and_bit::evaluate()
    const
{
    trace(("and_bit_assign::evaluate()\n{\n"));
    rpt_value::pointer result =
        bin_eval(nth_child(0), &rpt_expr_and_bit::create, nth_child(1));
    trace(("return %08lX;\n", (long)result.get()));
    trace(("}\n"));
    return result;
}


rpt_expr_assign_xor_bit::~rpt_expr_assign_xor_bit()
{
}


rpt_expr_assign_xor_bit::rpt_expr_assign_xor_bit(const rpt_expr::pointer &a,
    const rpt_expr::pointer &b)
{
    append(a);
    append(b);

    if (!a->lvalue())
	a->parse_error(i18n("illegal left hand side of assignment"));
}


rpt_expr::pointer
rpt_expr_assign_xor_bit::create(const rpt_expr::pointer &a,
    const rpt_expr::pointer &b)
{
    return pointer(new rpt_expr_assign_xor_bit(a, b));
}


rpt_value::pointer
rpt_expr_assign_xor_bit::evaluate()
    const
{
    trace(("xor_bit_assign::evaluate()\n{\n"));
    rpt_value::pointer result =
        bin_eval(nth_child(0), &rpt_expr_xor_bit::create, nth_child(1));
    trace(("return %08lX;\n", (long)result.get()));
    trace(("}\n"));
    return result;
}


rpt_expr_assign_or_bit::~rpt_expr_assign_or_bit()
{
}


rpt_expr_assign_or_bit::rpt_expr_assign_or_bit(const rpt_expr::pointer &a,
    const rpt_expr::pointer &b)
{
    append(a);
    append(b);

    if (!a->lvalue())
	a->parse_error(i18n("illegal left hand side of assignment"));
}


rpt_expr::pointer
rpt_expr_assign_or_bit::create(const rpt_expr::pointer &a,
    const rpt_expr::pointer &b)
{
    return pointer(new rpt_expr_assign_or_bit(a, b));
}


rpt_value::pointer
rpt_expr_assign_or_bit::evaluate()
    const
{
    trace(("or_bit_assign::evaluate()\n{\n"));
    rpt_value::pointer result =
        bin_eval(nth_child(0), &rpt_expr_or_bit::create, nth_child(1));
    trace(("return %08lX;\n", (long)result.get()));
    trace(("}\n"));
    return result;
}


rpt_expr_assign_shift_left::~rpt_expr_assign_shift_left()
{
}


rpt_expr_assign_shift_left::rpt_expr_assign_shift_left(
    const rpt_expr::pointer &a, const rpt_expr::pointer &b)
{
    append(a);
    append(b);

    if (!a->lvalue())
	a->parse_error(i18n("illegal left hand side of assignment"));
}


rpt_expr::pointer
rpt_expr_assign_shift_left::create(const rpt_expr::pointer &a,
    const rpt_expr::pointer &b)
{
    return pointer(new rpt_expr_assign_shift_left(a, b));
}


rpt_value::pointer
rpt_expr_assign_shift_left::evaluate()
    const
{
    trace(("shift_left_assign::evaluate()\n{\n"));
    rpt_value::pointer result =
        bin_eval(nth_child(0), &rpt_expr_shift_left::create, nth_child(1));
    trace(("return %08lX;\n", (long)result.get()));
    trace(("}\n"));
    return result;
}


rpt_expr_assign_shift_right::~rpt_expr_assign_shift_right()
{
}


rpt_expr_assign_shift_right::rpt_expr_assign_shift_right(
    const rpt_expr::pointer &a, const rpt_expr::pointer &b)
{
    append(a);
    append(b);

    if (!a->lvalue())
	a->parse_error(i18n("illegal left hand side of assignment"));
}


rpt_expr::pointer
rpt_expr_assign_shift_right::create(const rpt_expr::pointer &a,
    const rpt_expr::pointer &b)
{
    return pointer(new rpt_expr_assign_shift_right(a, b));
}


rpt_value::pointer
rpt_expr_assign_shift_right::evaluate()
    const
{
    trace(("shift_right_assign::evaluate()\n{\n"));
    rpt_value::pointer result =
        bin_eval(nth_child(0), &rpt_expr_shift_right::create, nth_child(1));
    trace(("return %08lX;\n", (long)result.get()));
    trace(("}\n"));
    return result;
}


rpt_expr_assign_join::~rpt_expr_assign_join()
{
}


rpt_expr_assign_join::rpt_expr_assign_join(const rpt_expr::pointer &a,
    const rpt_expr::pointer &b)
{
    append(a);
    append(b);

    if (!a->lvalue())
	a->parse_error(i18n("illegal left hand side of assignment"));
}


rpt_expr::pointer
rpt_expr_assign_join::create(const rpt_expr::pointer &a,
    const rpt_expr::pointer &b)
{
    return pointer(new rpt_expr_assign_join(a, b));
}


rpt_value::pointer
rpt_expr_assign_join::evaluate()
    const
{
    trace(("join_assign::evaluate()\n{\n"));
    rpt_value::pointer result =
        bin_eval(nth_child(0), &rpt_expr_join::create, nth_child(1));
    trace(("return %08lX;\n", (long)result.get()));
    trace(("}\n"));
    return result;
}
