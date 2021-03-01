//
// aegis - project change supervisor
// Copyright (C) 1994-1996, 1999, 2002-2008, 2012 Peter Miller
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
#include <common/ac/math.h>

#include <common/trace.h>
#include <libaegis/aer/expr/mul.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/real.h>
#include <libaegis/sub.h>

#define PAIR(a, b)      ((a) * rpt_value_type_MAX + (b))


rpt_expr_mul::~rpt_expr_mul()
{
}


rpt_expr_mul::rpt_expr_mul(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    append(lhs);
    append(rhs);
}


rpt_expr::pointer
rpt_expr_mul::create(const rpt_expr::pointer &lhs, const rpt_expr::pointer &rhs)
{
    return pointer(new rpt_expr_mul(lhs, rhs));
}


rpt_value::pointer
rpt_expr_mul::evaluate()
    const
{
    //
    // evaluate the left hand side
    //
    trace(("mul::evaluate()\n"));
    assert(get_nchildren() == 2);
    rpt_value::pointer v1 = nth_child(0)->evaluate(true, true);
    if (v1->is_an_error())
        return v1;

    //
    // coerce the left hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    rpt_value::pointer v1a = rpt_value::arithmetic(v1);

    //
    // evaluate the right hand side
    //
    rpt_value::pointer v2 = nth_child(1)->evaluate(true, true);
    if (v2->is_an_error())
        return v2;

    //
    // coerce the right hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    rpt_value::pointer v2a = rpt_value::arithmetic(v2);

    //
    // the type of the result depends on
    // the types of the operands
    //
    {
        rpt_value_integer *v1aip = dynamic_cast<rpt_value_integer *>(v1a.get());
        if (v1aip)
        {
            long v1n = v1aip->query();

            {
                rpt_value_integer *v2aip =
                    dynamic_cast<rpt_value_integer *>(v2a.get());
                if (v2aip)
                {
                    return rpt_value_integer::create(v1n * v2aip->query());
                }
            }

            {
                rpt_value_real *v2arp =
                    dynamic_cast<rpt_value_real *>(v2a.get());
                if (v2arp)
                {
                    return rpt_value_real::create(v1n * v2arp->query());
                }
            }
        }
    }

    {
        rpt_value_real *v1arp = dynamic_cast<rpt_value_real *>(v1a.get());
        if (v1arp)
        {
            double v1n = v1arp->query();

            {
                rpt_value_integer *v2aip =
                    dynamic_cast<rpt_value_integer *>(v2a.get());
                if (v2aip)
                {
                    return rpt_value_real::create(v1n * v2aip->query());
                }
            }

            {
                rpt_value_real *v2arp =
                    dynamic_cast<rpt_value_real *>(v2a.get());
                if (v2arp)
                {
                    return rpt_value_real::create(v1n * v2arp->query());
                }
            }
        }
    }

    sub_context_ty sc;
    sc.var_set_charstar("Name1", v1a->name());
    sc.var_set_charstar("Name2", v2a->name());
    nstring s(sc.subst_intl(i18n("illegal multiplication ($name1 * $name2)")));
    return rpt_value_error::create(get_pos(), s);
}


rpt_expr_div::~rpt_expr_div()
{
}


rpt_expr_div::rpt_expr_div(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    append(lhs);
    append(rhs);
}


rpt_expr::pointer
rpt_expr_div::create(const rpt_expr::pointer &lhs, const rpt_expr::pointer &rhs)
{
    return pointer(new rpt_expr_div(lhs, rhs));
}


static rpt_value::pointer
div_by_zero_error(const rpt_expr::pointer &ep)
{
    sub_context_ty sc;
    nstring s(sc.subst_intl(i18n("division by zero")));
    return rpt_value_error::create(ep->get_pos(), s);
}


rpt_value::pointer
rpt_expr_div::evaluate()
    const
{
    //
    // evaluate the left hand side
    //
    trace(("div::evaluate()\n"));
    assert(get_nchildren() == 2);
    rpt_value::pointer v1 = nth_child(0)->evaluate(true, true);
    if (v1->is_an_error())
        return v1;

    //
    // coerce the left hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    rpt_value::pointer v1a = rpt_value::arithmetic(v1);

    //
    // evaluate the right hand side
    //
    rpt_value::pointer v2 = nth_child(1)->evaluate(true, true);
    if (v2->is_an_error())
        return v2;

    //
    // coerce the right hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    rpt_value::pointer v2a = rpt_value::arithmetic(v2);

    //
    // the type of the result depends on
    // the types of the operands
    //
    {
        rpt_value_integer *v1aip = dynamic_cast<rpt_value_integer *>(v1a.get());
        if (v1aip)
        {
            long v1n = v1aip->query();

            {
                rpt_value_integer *v2aip =
                    dynamic_cast<rpt_value_integer *>(v2a.get());
                if (v2aip)
                {
                    long den = v2aip->query();
                    if (den == 0)
                        return div_by_zero_error(nth_child(1));
                    return rpt_value_integer::create(v1n / den);
                }
            }

            {
                rpt_value_real *v2arp =
                    dynamic_cast<rpt_value_real *>(v2a.get());
                if (v2arp)
                {
                    double den = v2arp->query();
                    if (den == 0)
                        return div_by_zero_error(nth_child(1));
                    return rpt_value_real::create(v1n / den);
                }
            }
        }
    }

    {
        rpt_value_real *v1arp = dynamic_cast<rpt_value_real *>(v1a.get());
        if (v1arp)
        {
            double v1n = v1arp->query();

            {
                rpt_value_integer *v2aip =
                    dynamic_cast<rpt_value_integer *>(v2a.get());
                if (v2aip)
                {
                    long den = v2aip->query();
                    if (den == 0)
                        return div_by_zero_error(nth_child(1));
                    return rpt_value_real::create(v1n / den);
                }
            }

            {
                rpt_value_real *v2arp =
                    dynamic_cast<rpt_value_real *>(v2a.get());
                if (v2arp)
                {
                    double den = v2arp->query();
                    if (den == 0)
                        return div_by_zero_error(nth_child(1));
                    return rpt_value_real::create(v1n / den);
                }
            }
        }
    }

    sub_context_ty sc;
    sc.var_set_charstar("Name1", v1a->name());
    sc.var_set_charstar("Name2", v2a->name());
    nstring s(sc.subst_intl(i18n("illegal division ($name1 / $name2)")));
    return rpt_value_error::create(get_pos(), s);
}


rpt_expr_mod::~rpt_expr_mod()
{
}


rpt_expr_mod::rpt_expr_mod(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    append(lhs);
    append(rhs);
}


rpt_expr::pointer
rpt_expr_mod::create(const rpt_expr::pointer &lhs, const rpt_expr::pointer &rhs)
{
    return pointer(new rpt_expr_mod(lhs, rhs));
}


static rpt_value::pointer
mod_by_zero_error(const rpt_expr::pointer &ep)
{
    sub_context_ty sc;
    nstring s(sc.subst_intl(i18n("modulo by zero")));
    return rpt_value_error::create(ep->get_pos(), s);
}


rpt_value::pointer
rpt_expr_mod::evaluate()
    const
{
    //
    // evaluate the left hand side
    //
    trace(("mod::evaluate()\n"));
    assert(get_nchildren() == 2);
    rpt_value::pointer v1 = nth_child(0)->evaluate(true, true);
    if (v1->is_an_error())
        return v1;

    //
    // coerce the left hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    rpt_value::pointer v1a = rpt_value::arithmetic(v1);

    //
    // evaluate the right hand side
    //
    rpt_value::pointer v2 = nth_child(1)->evaluate(true, true);
    if (v2->is_an_error())
        return v2;

    //
    // coerce the right hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    rpt_value::pointer v2a = rpt_value::arithmetic(v2);

    //
    // the type of the result depends on
    // the types of the operands
    //
    {
        rpt_value_integer *v1aip = dynamic_cast<rpt_value_integer *>(v1a.get());
        if (v1aip)
        {
            long num = v1aip->query();

            {
                rpt_value_integer *v2aip =
                    dynamic_cast<rpt_value_integer *>(v2a.get());
                if (v2aip)
                {
                    long den = v2aip->query();
                    if (den == 0)
                        return mod_by_zero_error(nth_child(1));
                    return rpt_value_integer::create(num % den);
                }
            }

            {
                rpt_value_real *v2arp =
                    dynamic_cast<rpt_value_real *>(v2a.get());
                if (v2arp)
                {
                    double den = v2arp->query();
                    if (den == 0)
                        return mod_by_zero_error(nth_child(1));
                    return rpt_value_real::create(fmod(num, den));
                }
            }
        }
    }

    {
        rpt_value_real *v1arp = dynamic_cast<rpt_value_real *>(v1a.get());
        if (v1arp)
        {
            double num = v1arp->query();

            {
                rpt_value_integer *v2aip =
                    dynamic_cast<rpt_value_integer *>(v2a.get());
                if (v2aip)
                {
                    long den = v2aip->query();
                    if (den == 0)
                        return mod_by_zero_error(nth_child(1));
                    return rpt_value_real::create(fmod(num, den));
                }
            }

            {
                rpt_value_real *v2arp =
                    dynamic_cast<rpt_value_real *>(v2a.get());
                if (v2arp)
                {
                    double den = v2arp->query();
                    if (den == 0)
                        return mod_by_zero_error(nth_child(1));
                    return rpt_value_real::create(fmod(num, den));
                }
            }
        }
    }

    sub_context_ty sc;
    sc.var_set_charstar("Name1", v1a->name());
    sc.var_set_charstar("Name2", v2a->name());
    // xgettext:no-c-format
    nstring s(sc.subst_intl(i18n("illegal modulo ($name1 % $name2)")));
    return rpt_value_error::create(get_pos(), s);
}


// vim: set ts=8 sw=4 et :
