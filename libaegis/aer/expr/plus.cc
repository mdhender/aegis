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

#include <common/trace.h>
#include <libaegis/aer/expr/plus.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/list.h>
#include <libaegis/aer/value/real.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/sub.h>

#define PAIR(a, b)      ((a) * rpt_value_type_MAX + (b))


rpt_expr_plus::~rpt_expr_plus()
{
}


rpt_expr_plus::rpt_expr_plus(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    append(lhs);
    append(rhs);
}


rpt_expr::pointer
rpt_expr_plus::create(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    return pointer(new rpt_expr_plus(lhs, rhs));
}


rpt_value::pointer
rpt_expr_plus::evaluate()
    const
{
    //
    // evaluate the left hand side
    //
    trace(("plus::evaluate()\n{\n"));
    assert(get_nchildren() == 2);
    rpt_value::pointer lv1 = nth_child(0)->evaluate(true, true);
    if (lv1->is_an_error())
    {
        trace(("}\n"));
        return lv1;
    }

    //
    // coerce the left hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    rpt_value::pointer lv2 = rpt_value::arithmetic(lv1);

    //
    // evaluate the right hand side
    //
    rpt_value::pointer rv1 = nth_child(1)->evaluate(true, true);
    if (rv1->is_an_error())
    {
        trace(("}\n"));
        return rv1;
    }

    //
    // coerce the right hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    rpt_value::pointer rv2 = rpt_value::arithmetic(rv1);

    //
    // the type of the result depends on
    // the types of the operands
    //
    {
        rpt_value_integer *lv2ip = dynamic_cast<rpt_value_integer *>(lv2.get());
        if (lv2ip)
        {
            long li = lv2ip->query();

            {
                rpt_value_integer *rv2ip =
                    dynamic_cast<rpt_value_integer *>(rv2.get());
                if (rv2ip)
                {
                    long ri = rv2ip->query();
                    rpt_value::pointer result =
                        rpt_value_integer::create(li + ri);
                    trace(("}\n"));
                    return result;
                }
            }

            {
                rpt_value_real *rv2rp =
                    dynamic_cast<rpt_value_real *>(rv2.get());
                if (rv2rp)
                {
                    double rr = rv2rp->query();
                    rpt_value::pointer result = rpt_value_real::create(li + rr);
                    trace(("}\n"));
                    return result;
                }
            }
        }
    }

    {
        rpt_value_real *lv2rp = dynamic_cast<rpt_value_real *>(lv2.get());
        if (lv2rp)
        {
            double lr = lv2rp->query();

            {
                rpt_value_integer *rv2ip =
                    dynamic_cast<rpt_value_integer *>(rv2.get());
                if (rv2ip)
                {
                    long ri = rv2ip->query();
                    rpt_value::pointer result = rpt_value_real::create(lr + ri);
                    trace(("}\n"));
                    return result;
                }
            }

            {
                rpt_value_real *rv2rp =
                    dynamic_cast<rpt_value_real *>(rv2.get());
                if (rv2rp)
                {
                    double rr = rv2rp->query();
                    rpt_value::pointer result = rpt_value_real::create(lr + rr);
                    trace(("}\n"));
                    return result;
                }
            }
        }
    }

    sub_context_ty sc;
    sc.var_set_charstar("Name1", lv2->name());
    sc.var_set_charstar("Name2", rv2->name());
    nstring s(sc.subst_intl(i18n("illegal addition ($name1 + $name2)")));
    rpt_value::pointer vp = rpt_value_error::create(get_pos(), s);
    trace(("}\n"));
    return vp;
}


rpt_expr_minus::~rpt_expr_minus()
{
}


rpt_expr_minus::rpt_expr_minus(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    append(lhs);
    append(rhs);
}


rpt_expr::pointer
rpt_expr_minus::create(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    return pointer(new rpt_expr_minus(lhs, rhs));
}


rpt_value::pointer
rpt_expr_minus::evaluate()
    const
{
    //
    // evaluate the left hand side
    //
    trace(("minus::evaluate()\n"));
    assert(get_nchildren() == 2);
    rpt_value::pointer lv1 = nth_child(0)->evaluate(true, true);
    if (lv1->is_an_error())
        return lv1;

    //
    // coerce the left hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    rpt_value::pointer lv2 = rpt_value::arithmetic(lv1);

    //
    // evaluate the right hand side
    //
    rpt_value::pointer rv1 = nth_child(1)->evaluate(true, true);
    if (rv1->is_an_error())
        return rv1;

    //
    // coerce the right hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    rpt_value::pointer rv2 = rpt_value::arithmetic(rv1);

    //
    // the type of the result depends on
    // the types of the operands
    //
    {
        rpt_value_integer *lv2ip = dynamic_cast<rpt_value_integer *>(lv2.get());
        if (lv2ip)
        {
            long li = lv2ip->query();

            {
                rpt_value_integer *rv2ip =
                    dynamic_cast<rpt_value_integer *>(rv2.get());
                if (rv2ip)
                {
                    long ri = rv2ip->query();
                    return rpt_value_integer::create(li - ri);
                }
            }

            {
                rpt_value_real *rv2rp =
                    dynamic_cast<rpt_value_real *>(rv2.get());
                if (rv2rp)
                {
                    double rr = rv2rp->query();
                    return rpt_value_real::create(li - rr);
                }
            }
        }
    }

    {
        rpt_value_real *lv2rp = dynamic_cast<rpt_value_real *>(lv2.get());
        if (lv2rp)
        {
            double lr = lv2rp->query();

            {
                rpt_value_integer *rv2ip =
                    dynamic_cast<rpt_value_integer *>(rv2.get());
                if (rv2ip)
                {
                    long ri = rv2ip->query();
                    return rpt_value_real::create(lr - ri);
                }
            }

            {
                rpt_value_real *rv2rp =
                    dynamic_cast<rpt_value_real *>(rv2.get());
                if (rv2rp)
                {
                    double rr = rv2rp->query();
                    return rpt_value_real::create(lr - rr);
                }
            }
        }
    }

    sub_context_ty sc;
    sc.var_set_charstar("Name1", lv2->name());
    sc.var_set_charstar("Name2", rv2->name());
    nstring s(sc.subst_intl(i18n("illegal subtraction ($name1 - $name2)")));
    return rpt_value_error::create(get_pos(), s);
}


rpt_expr_join::~rpt_expr_join()
{
}


rpt_expr_join::rpt_expr_join(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    append(lhs);
    append(rhs);
}


rpt_expr::pointer
rpt_expr_join::create(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    return pointer(new rpt_expr_join(lhs, rhs));
}


rpt_value::pointer
rpt_expr_join::evaluate()
    const
{
    //
    // evaluate the left hand side
    //
    trace(("join::evaluate()\n"));
    assert(get_nchildren() == 2);
    rpt_value::pointer v1 = nth_child(0)->evaluate(true, true);
    if (v1->is_an_error())
        return v1;

    //
    // evaluate the right hand side
    //
    rpt_value::pointer v2 = nth_child(1)->evaluate(true, true);
    if (v2->is_an_error())
        return v2;

    //
    // you can join almost anything to a list
    //
    rpt_value_list *v1lp = dynamic_cast<rpt_value_list *>(v1.get());
    if (v1lp)
    {
        rpt_value_list *rlp = new rpt_value_list();
        rpt_value::pointer result(rlp);

        size_t n = v1lp->size();
        for (size_t j = 0; j < n; ++j)
        {
            rlp->append(v1lp->nth(j));
        }

        rpt_value_list *v2lp = dynamic_cast<rpt_value_list *>(v2.get());
        if (v2lp)
        {
            n = v2lp->size();
            for (size_t j = 0; j < n; ++j)
            {
                rlp->append(v2lp->nth(j));
            }
        }
        else
            rlp->append(v2);
        return result;
    }

    rpt_value_list *v2lp = dynamic_cast<rpt_value_list *>(v2.get());
    if (v2lp)
    {
        rpt_value_list *rlp = new rpt_value_list();
        rpt_value::pointer result(rlp);

        rlp->append(v1);
        size_t n = v2lp->size();
        for (size_t j = 0; j < n; ++j)
        {
            rlp->append(v2lp->nth(j));
        }
        return result;
    }

    //
    // must be a string join
    //
    rpt_value::pointer v1s = rpt_value::stringize(v1);
    rpt_value_string *v1sp = dynamic_cast<rpt_value_string *>(v1s.get());
    rpt_value::pointer v2s = rpt_value::stringize(v2);
    rpt_value_string *v2sp = dynamic_cast<rpt_value_string *>(v2s.get());
    if (v1sp && v2sp)
    {
        nstring s = v1sp->query() + v2sp->query();
        rpt_value::pointer result = rpt_value_string::create(s);
        return result;
    }

    //
    // anything else is an error
    //
    sub_context_ty sc;
    sc.var_set_charstar("Name1", v1s->name());
    sc.var_set_charstar("Name2", v2s->name());
    nstring s(sc.subst_intl(i18n("illegal join ($name1 ## $name2)")));
    return rpt_value_error::create(get_pos(), s);
}


// vim: set ts=8 sw=4 et :
