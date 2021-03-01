//
//      aegis - project change supervisor
//      Copyright (C) 1994-1996, 1999, 2001-2008 Peter Miller
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

#include <common/ac/string.h>

#include <libaegis/aer/expr/rel.h>
#include <libaegis/aer/value/boolean.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/real.h>
#include <libaegis/aer/value/string.h>
#include <common/error.h>
#include <common/fstrcmp.h>
#include <libaegis/sub.h>
#include <common/trace.h>

#define PAIR(a, b)      ((a) * rpt_value_type_MAX + (b))


rpt_expr_lt::~rpt_expr_lt()
{
}


rpt_expr_lt::rpt_expr_lt(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    append(lhs);
    append(rhs);
}


rpt_expr::pointer
rpt_expr_lt::create(const rpt_expr::pointer &lhs, const rpt_expr::pointer &rhs)
{
    return pointer(new rpt_expr_lt(lhs, rhs));
}


rpt_value::pointer
rpt_expr_lt::evaluate()
    const
{
    //
    // evaluate the left hand side
    //
    trace(("lt::evaluate()\n"));
    assert(get_nchildren() == 2);
    rpt_value::pointer lv1 = nth_child(0)->evaluate(true, true);
    if (lv1->is_an_error())
        return lv1;

    //
    // coerce the left hand side to an arithmetic type
    //  (will not give error if can't, will copy instead)
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
    //  (will not give error if can't, will copy instead)
    //
    rpt_value::pointer rv2 = rpt_value::arithmetic(rv1);

    //
    // what to do depends on
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
                    return rpt_value_boolean::create(li < ri);
                }
            }

            {
                rpt_value_integer *rv2rp =
                    dynamic_cast<rpt_value_integer *>(rv2.get());
                if (rv2rp)
                {
                    double rr = rv2rp->query();
                    return rpt_value_boolean::create(li < rr);
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
                    return rpt_value_boolean::create(lr < ri);
                }
            }

            {
                rpt_value_integer *rv2rp =
                    dynamic_cast<rpt_value_integer *>(rv2.get());
                if (rv2rp)
                {
                    double rr = rv2rp->query();
                    return rpt_value_boolean::create(lr < rr);
                }
            }
        }
    }

    //
    // No numerical comparisons worked, try for a string comparison.
    //
    rpt_value::pointer lv5 = rpt_value::stringize(lv1);
    rpt_value_string *lv5sp = dynamic_cast<rpt_value_string *>(lv5.get());
    rpt_value::pointer rv5 = rpt_value::stringize(rv1);
    rpt_value_string *rv5sp = dynamic_cast<rpt_value_string *>(rv5.get());
    if (lv5sp && rv5sp)
    {
        return rpt_value_boolean::create(lv5sp->query() < rv5sp->query());
    }

    //
    // nothing worked.  complain.
    //
    sub_context_ty sc;
    sc.var_set_charstar("Name1", lv1->name());
    sc.var_set_charstar("Name2", rv1->name());
    nstring s(sc.subst_intl(i18n("illegal comparison ($name1 < $name2)")));
    return rpt_value_error::create(get_pos(), s);
}


rpt_expr_le::~rpt_expr_le()
{
}


rpt_expr_le::rpt_expr_le(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    append(lhs);
    append(rhs);
}


rpt_expr::pointer
rpt_expr_le::create(const rpt_expr::pointer &lhs, const rpt_expr::pointer &rhs)
{
    return pointer(new rpt_expr_le(lhs, rhs));
}


rpt_value::pointer
rpt_expr_le::evaluate()
    const
{
    //
    // evaluate the left hand side
    //
    trace(("le::evaluate()\n"));
    assert(get_nchildren() == 2);
    rpt_value::pointer lv1 = nth_child(0)->evaluate(true, true);
    if (lv1->is_an_error())
        return lv1;

    //
    // coerce the left hand side to an arithmetic type
    //  (will not give error if can't, will copy instead)
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
    //  (will not give error if can't, will copy instead)
    //
    rpt_value::pointer rv2 = rpt_value::arithmetic(rv1);

    //
    // what to do depends on
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
                    return rpt_value_boolean::create(li <= ri);
                }
            }

            {
                rpt_value_integer *rv2rp =
                    dynamic_cast<rpt_value_integer *>(rv2.get());
                if (rv2rp)
                {
                    double rr = rv2rp->query();
                    return rpt_value_boolean::create(li <= rr);
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
                    return rpt_value_boolean::create(lr <= ri);
                }
            }

            {
                rpt_value_integer *rv2rp =
                    dynamic_cast<rpt_value_integer *>(rv2.get());
                if (rv2rp)
                {
                    double rr = rv2rp->query();
                    return rpt_value_boolean::create(lr <= rr);
                }
            }
        }
    }

    //
    // No numerical comparisons worked, try for a string comparison.
    //
    rpt_value::pointer lv5 = rpt_value::stringize(lv1);
    rpt_value_string *lv5sp = dynamic_cast<rpt_value_string *>(lv5.get());
    rpt_value::pointer rv5 = rpt_value::stringize(rv1);
    rpt_value_string *rv5sp = dynamic_cast<rpt_value_string *>(rv5.get());
    if (lv5sp && rv5sp)
    {
        return rpt_value_boolean::create(lv5sp->query() <= rv5sp->query());
    }

    //
    // nothing worked.  complain.
    //
    sub_context_ty sc;
    sc.var_set_charstar("Name1", lv1->name());
    sc.var_set_charstar("Name2", rv1->name());
    nstring s(sc.subst_intl(i18n("illegal comparison ($name1 <= $name2)")));
    return rpt_value_error::create(get_pos(), s);
}


rpt_expr_gt::~rpt_expr_gt()
{
}


rpt_expr_gt::rpt_expr_gt(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    append(lhs);
    append(rhs);
}


rpt_expr::pointer
rpt_expr_gt::create(const rpt_expr::pointer &lhs, const rpt_expr::pointer &rhs)
{
    return pointer(new rpt_expr_gt(lhs, rhs));
}


rpt_value::pointer
rpt_expr_gt::evaluate()
    const
{
    //
    // evaluate the left hand side
    //
    trace(("gt::evaluate()\n"));
    assert(get_nchildren() == 2);
    rpt_value::pointer lv1 = nth_child(0)->evaluate(true, true);
    if (lv1->is_an_error())
        return lv1;

    //
    // coerce the left hand side to an arithmetic type
    //  (will not give error if can't, will copy instead)
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
    //  (will not give error if can't, will copy instead)
    //
    rpt_value::pointer rv2 = rpt_value::arithmetic(rv1);

    //
    // what to do depends on
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
                    return rpt_value_boolean::create(li > ri);
                }
            }

            {
                rpt_value_integer *rv2rp =
                    dynamic_cast<rpt_value_integer *>(rv2.get());
                if (rv2rp)
                {
                    double rr = rv2rp->query();
                    return rpt_value_boolean::create(li > rr);
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
                    return rpt_value_boolean::create(lr > ri);
                }
            }

            {
                rpt_value_integer *rv2rp =
                    dynamic_cast<rpt_value_integer *>(rv2.get());
                if (rv2rp)
                {
                    double rr = rv2rp->query();
                    return rpt_value_boolean::create(lr > rr);
                }
            }
        }
    }

    //
    // No numerical comparisons worked, try for a string comparison.
    //
    rpt_value::pointer lv5 = rpt_value::stringize(lv1);
    rpt_value_string *lv5sp = dynamic_cast<rpt_value_string *>(lv5.get());
    rpt_value::pointer rv5 = rpt_value::stringize(rv1);
    rpt_value_string *rv5sp = dynamic_cast<rpt_value_string *>(rv5.get());
    if (lv5sp && rv5sp)
    {
        return rpt_value_boolean::create(lv5sp->query() > rv5sp->query());
    }

    //
    // nothing worked.  complain.
    //
    sub_context_ty sc;
    sc.var_set_charstar("Name1", lv1->name());
    sc.var_set_charstar("Name2", rv1->name());
    nstring s(sc.subst_intl(i18n("illegal comparison ($name1 > $name2)")));
    return rpt_value_error::create(get_pos(), s);
}


rpt_expr_ge::~rpt_expr_ge()
{
}


rpt_expr_ge::rpt_expr_ge(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    append(lhs);
    append(rhs);
}


rpt_expr::pointer
rpt_expr_ge::create(const rpt_expr::pointer &lhs, const rpt_expr::pointer &rhs)
{
    return pointer(new rpt_expr_ge(lhs, rhs));
}


rpt_value::pointer
rpt_expr_ge::evaluate()
    const
{
    //
    // evaluate the left hand side
    //
    trace(("ge::evaluate()\n"));
    assert(get_nchildren() == 2);
    rpt_value::pointer lv1 = nth_child(0)->evaluate(true, true);
    if (lv1->is_an_error())
        return lv1;

    //
    // coerce the left hand side to an arithmetic type
    //  (will not give error if can't, will copy instead)
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
    //  (will not give error if can't, will copy instead)
    //
    rpt_value::pointer rv2 = rpt_value::arithmetic(rv1);

    //
    // what to do depends on
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
                    return rpt_value_boolean::create(li >= ri);
                }
            }

            {
                rpt_value_integer *rv2rp =
                    dynamic_cast<rpt_value_integer *>(rv2.get());
                if (rv2rp)
                {
                    double rr = rv2rp->query();
                    return rpt_value_boolean::create(li >= rr);
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
                    return rpt_value_boolean::create(lr >= ri);
                }
            }

            {
                rpt_value_integer *rv2rp =
                    dynamic_cast<rpt_value_integer *>(rv2.get());
                if (rv2rp)
                {
                    double rr = rv2rp->query();
                    return rpt_value_boolean::create(lr >= rr);
                }
            }
        }
    }

    //
    // No numerical comparisons worked, try for a string comparison.
    //
    rpt_value::pointer lv5 = rpt_value::stringize(lv1);
    rpt_value_string *lv5sp = dynamic_cast<rpt_value_string *>(lv5.get());
    rpt_value::pointer rv5 = rpt_value::stringize(rv1);
    rpt_value_string *rv5sp = dynamic_cast<rpt_value_string *>(rv5.get());
    if (lv5sp && rv5sp)
    {
        return rpt_value_boolean::create(lv5sp->query() >= rv5sp->query());
    }

    //
    // nothing worked.  complain.
    //
    sub_context_ty sc;
    sc.var_set_charstar("Name1", lv1->name());
    sc.var_set_charstar("Name2", rv1->name());
    nstring s(sc.subst_intl(i18n("illegal comparison ($name1 >= $name2)")));
    return rpt_value_error::create(get_pos(), s);
}


rpt_expr_eq::~rpt_expr_eq()
{
}


rpt_expr_eq::rpt_expr_eq(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    append(lhs);
    append(rhs);
}


rpt_expr::pointer
rpt_expr_eq::create(const rpt_expr::pointer &lhs, const rpt_expr::pointer &rhs)
{
    return pointer(new rpt_expr_eq(lhs, rhs));
}


rpt_value::pointer
rpt_expr_eq::evaluate()
    const
{
    //
    // evaluate the left hand side
    //
    trace(("eq::evaluate()\n"));
    assert(get_nchildren() == 2);
    rpt_value::pointer lv1 = nth_child(0)->evaluate(true, true);
    trace(("lv1 is a %s\n", lv1->name()));
    if (lv1->is_an_error())
        return lv1;

    //
    // coerce the left hand side to an arithmetic type
    //  (will not give error if can't, will copy instead)
    //
    rpt_value::pointer lv2 = rpt_value::arithmetic(lv1);
    trace(("lv2 is a %s\n", lv2->name()));

    //
    // evaluate the right hand side
    //
    rpt_value::pointer rv1 = nth_child(1)->evaluate(true, true);
    trace(("rv1 is a %s\n", rv1->name()));
    if (rv1->is_an_error())
        return rv1;

    //
    // coerce the right hand side to an arithmetic type
    //  (will not give error if can't, will copy instead)
    //
    rpt_value::pointer rv2 = rpt_value::arithmetic(rv1);
    trace(("rv2 is a %s\n", rv2->name()));

    //
    // what to do depends on
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
                    return rpt_value_boolean::create(li == ri);
                }
            }

            {
                rpt_value_integer *rv2rp =
                    dynamic_cast<rpt_value_integer *>(rv2.get());
                if (rv2rp)
                {
                    double rr = rv2rp->query();
                    return rpt_value_boolean::create(li == rr);
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
                    return rpt_value_boolean::create(lr == ri);
                }
            }

            {
                rpt_value_integer *rv2rp =
                    dynamic_cast<rpt_value_integer *>(rv2.get());
                if (rv2rp)
                {
                    double rr = rv2rp->query();
                    return rpt_value_boolean::create(lr == rr);
                }
            }
        }
    }

    //
    // No numerical comparisons worked, try for a string comparison.
    //
    rpt_value::pointer lv5 = rpt_value::stringize(lv1);
    rpt_value_string *lv5sp = dynamic_cast<rpt_value_string *>(lv5.get());
    rpt_value::pointer rv5 = rpt_value::stringize(rv1);
    rpt_value_string *rv5sp = dynamic_cast<rpt_value_string *>(rv5.get());
    if (lv5sp && rv5sp)
    {
        return rpt_value_boolean::create(lv5sp->query() == rv5sp->query());
    }

    //
    // nothing worked.  complain.
    //
    sub_context_ty sc;
    sc.var_set_charstar("Name1", lv1->name());
    sc.var_set_charstar("Name2", rv1->name());
    nstring s(sc.subst_intl(i18n("illegal comparison ($name1 == $name2)")));
    return rpt_value_error::create(get_pos(), s);
}


rpt_expr_ne::~rpt_expr_ne()
{
}


rpt_expr_ne::rpt_expr_ne(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    append(lhs);
    append(rhs);
}


rpt_expr::pointer
rpt_expr_ne::create(const rpt_expr::pointer &lhs, const rpt_expr::pointer &rhs)
{
    return pointer(new rpt_expr_ne(lhs, rhs));
}


rpt_value::pointer
rpt_expr_ne::evaluate()
    const
{
    //
    // evaluate the left hand side
    //
    trace(("ne::evaluate()\n"));
    assert(get_nchildren() == 2);
    rpt_value::pointer lv1 = nth_child(0)->evaluate(true, true);
    if (lv1->is_an_error())
        return lv1;

    //
    // coerce the left hand side to an arithmetic type
    //  (will not give error if can't, will copy instead)
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
    //  (will not give error if can't, will copy instead)
    //
    rpt_value::pointer rv2 = rpt_value::arithmetic(rv1);

    //
    // what to do depends on
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
                    return rpt_value_boolean::create(li != ri);
                }
            }

            {
                rpt_value_integer *rv2rp =
                    dynamic_cast<rpt_value_integer *>(rv2.get());
                if (rv2rp)
                {
                    double rr = rv2rp->query();
                    return rpt_value_boolean::create(li != rr);
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
                    return rpt_value_boolean::create(lr != ri);
                }
            }

            {
                rpt_value_integer *rv2rp =
                    dynamic_cast<rpt_value_integer *>(rv2.get());
                if (rv2rp)
                {
                    double rr = rv2rp->query();
                    return rpt_value_boolean::create(lr != rr);
                }
            }
        }
    }

    //
    // No numerical comparisons worked, try for a string comparison.
    //
    rpt_value::pointer lv5 = rpt_value::stringize(lv1);
    rpt_value_string *lv5sp = dynamic_cast<rpt_value_string *>(lv5.get());
    rpt_value::pointer rv5 = rpt_value::stringize(rv1);
    rpt_value_string *rv5sp = dynamic_cast<rpt_value_string *>(rv5.get());
    if (lv5sp && rv5sp)
    {
        return rpt_value_boolean::create(lv5sp->query() != rv5sp->query());
    }

    //
    // nothing worked.  complain.
    //
    sub_context_ty sc;
    sc.var_set_charstar("Name1", lv1->name());
    sc.var_set_charstar("Name2", rv1->name());
    nstring s(sc.subst_intl(i18n("illegal comparison ($name1 != $name2)")));
    return rpt_value_error::create(get_pos(), s);
}


rpt_expr_match::~rpt_expr_match()
{
}


rpt_expr_match::rpt_expr_match(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    append(lhs);
    append(rhs);
}


rpt_expr::pointer
rpt_expr_match::create(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    return pointer(new rpt_expr_match(lhs, rhs));
}


rpt_value::pointer
rpt_expr_match::evaluate()
    const
{
    //
    // evaluate the left hand side
    //
    trace(("match::evaluate()\n"));
    assert(get_nchildren() == 2);
    rpt_value::pointer lv1 = nth_child(0)->evaluate(true, true);
    if (lv1->is_an_error())
        return lv1;

    //
    // coerce the left hand side to a string
    // (will not give error if can't, will copy instead)
    //
    rpt_value::pointer lv2 = rpt_value::stringize(lv1);

    //
    // evaluate the right hand side
    //
    rpt_value::pointer rv1 = nth_child(1)->evaluate(true, true);
    if (rv1->is_an_error())
        return rv1;

    //
    // coerce the right hand side to a string
    // (will not give error if can't, will copy instead)
    //
    rpt_value::pointer rv2 = rpt_value::stringize(rv1);

    //
    // what to do depends on
    // the types of the operands
    //
    rpt_value_string *lsp = dynamic_cast<rpt_value_string *>(lv2.get());
    rpt_value_string *rsp = dynamic_cast<rpt_value_string *>(rv2.get());
    if (lsp && rsp)
    {
        return
            rpt_value_real::create
            (
                fstrcmp(lsp->query().c_str(), rsp->query().c_str())
            );
    }

    sub_context_ty sc;
    sc.var_set_charstar("Name1", lv1->name());
    sc.var_set_charstar("Name2", rv1->name());
    nstring s(sc.subst_intl(i18n("illegal comparison ($name1 ~~ $name2)")));
    return rpt_value_error::create(get_pos(), s);
}


rpt_expr_nmatch::~rpt_expr_nmatch()
{
}


rpt_expr_nmatch::rpt_expr_nmatch(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    append(lhs);
    append(rhs);
}


rpt_expr::pointer
rpt_expr_nmatch::create(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    return pointer(new rpt_expr_nmatch(lhs, rhs));
}


rpt_value::pointer
rpt_expr_nmatch::evaluate()
    const
{
    //
    // evaluate the left hand side
    //
    trace(("nmatch::evaluate()\n"));
    assert(get_nchildren() == 2);
    rpt_value::pointer lv1 = nth_child(0)->evaluate(true, true);
    if (lv1->is_an_error())
        return lv1;

    //
    // coerce the left hand side to a string
    // (will not give error if can't, will copy instead)
    //
    rpt_value::pointer lv2 = rpt_value::stringize(lv1);

    //
    // evaluate the right hand side
    //
    rpt_value::pointer rv1 = nth_child(1)->evaluate(true, true);
    if (rv1->is_an_error())
        return rv1;

    //
    // coerce the right hand side to a string
    // (will not give error if can't, will copy instead)
    //
    rpt_value::pointer rv2 = rpt_value::stringize(rv1);

    //
    // what to do depends on
    // the types of the operands
    //
    rpt_value_string *lsp = dynamic_cast<rpt_value_string *>(lv2.get());
    rpt_value_string *rsp = dynamic_cast<rpt_value_string *>(rv2.get());
    if (lsp && rsp)
    {
        return
            rpt_value_real::create
            (
                1 - fstrcmp(lsp->query().c_str(), rsp->query().c_str())
            );
    }

    sub_context_ty sc;
    sc.var_set_charstar("Name1", lv1->name());
    sc.var_set_charstar("Name2", rv1->name());
    nstring s(sc.subst_intl(i18n("illegal comparison ($name1 !~ $name2)")));
    return rpt_value_error::create(get_pos(), s);
}
