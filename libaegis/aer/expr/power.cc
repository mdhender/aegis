//
//      aegis - project change supervisor
//      Copyright (C) 1994-1996, 1999, 2002-2008, 2012 Peter Miller
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

#include <common/ac/assert.h>
#include <common/ac/errno.h>
#include <common/ac/string.h>
#include <common/ac/math.h>

#include <common/trace.h>
#include <libaegis/aer/expr/power.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/real.h>
#include <libaegis/sub.h>


rpt_expr_power::~rpt_expr_power()
{
}


rpt_expr_power::rpt_expr_power(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    append(lhs);
    append(rhs);
}


rpt_expr::pointer
rpt_expr_power::create(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    return pointer(new rpt_expr_power(lhs, rhs));
}


rpt_value::pointer
rpt_expr_power::evaluate()
    const
{
    //
    // evaluate the left hand side
    //
    trace(("power::evaluate()\n"));
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

    double v1d = 0;
    rpt_value_real *v1arp = dynamic_cast<rpt_value_real *>(v1a.get());
    if (v1arp)
    {
        v1d = v1arp->query();
    }
    else
    {
        rpt_value_integer *v1aip = dynamic_cast<rpt_value_integer *>(v1a.get());
        if (v1aip)
        {
            v1d = v1aip->query();
        }
        else
        {
            sub_context_ty sc;
            sc.var_set_charstar("Name1", v1->name());
            sc.var_set_charstar("Name2", v2->name());
            nstring s(sc.subst_intl(i18n("illegal power ($name1 ** $name2)")));
            return rpt_value_error::create(get_pos(), s);
        }
    }

    double v2d = 0;
    rpt_value_real *v2arp = dynamic_cast<rpt_value_real *>(v2a.get());
    if (v2arp)
    {
        v2d = v2arp->query();
    }
    else
    {
        rpt_value_integer *v2aip = dynamic_cast<rpt_value_integer *>(v2a.get());
        if (v2aip)
        {
            v2d = v2aip->query();
        }
        else
        {
            sub_context_ty sc;
            sc.var_set_charstar("Name1", v1->name());
            sc.var_set_charstar("Name2", v2->name());
            nstring s(sc.subst_intl(i18n("illegal power ($name1 ** $name2)")));
            return rpt_value_error::create(get_pos(), s);
        }
    }

    errno = 0;
    double n = pow(v1d, v2d);
    if (errno != 0)
    {
        int errno_old = errno;
        sub_context_ty sc;
        sc.errno_setx(errno_old);
        sc.var_set_format("Value1", "%g", v1d);
        sc.var_set_format("Value2", "%g", v2d);
        nstring s(sc.subst_intl(i18n("$value1 ** $value2: $errno")));
        return rpt_value_error::create(get_pos(), s);
    }

    return rpt_value_real::create(n);
}


// vim: set ts=8 sw=4 et :
