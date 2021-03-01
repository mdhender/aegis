//
//	aegis - project change supervisor
//	Copyright (C) 1997, 1999, 2001-2008 Peter Miller
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

#include <common/ac/string.h>

#include <libaegis/aer/value/boolean.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/real.h>
#include <libaegis/aer/value/string.h>
#include <common/error.h> // for assert
#include <common/str.h>
#include <libaegis/sub.h>
#include <common/trace.h>
#include <aefind/tree/diadic.h>
#include <aefind/tree/relative.h>



#define PAIR(a, b)	((a) * rpt_value_type_MAX + (b))


tree_lt::~tree_lt()
{
}


tree_lt::tree_lt(const tree::pointer &a1, const tree::pointer &a2) :
    tree_diadic(a1, a2)
{
}


tree::pointer
tree_lt::create(const tree::pointer &a1, const tree::pointer &a2)
{
    return pointer(new tree_lt(a1, a2));
}


rpt_value::pointer
tree_lt::evaluate(string_ty *path_unres, string_ty *path, string_ty *path_res,
    struct stat *st) const
{
    //
    // evaluate the left hand side
    //
    trace(("lt::evaluate()\n"));
    rpt_value::pointer lv1 =
        get_left()->evaluate(path_unres, path, path_res, st);
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
    rpt_value::pointer rv1 =
        get_right()->evaluate(path_unres, path, path_res, st);
    if (rv1->is_an_error())
	return rv1;

    //
    // coerce the right hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
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
            long lv = lv2ip->query();

            {
                rpt_value_integer *rv2ip =
                    dynamic_cast<rpt_value_integer *>(rv2.get());
                if (rv2ip)
                {
                    long rv = rv2ip->query();
                    return rpt_value_boolean::create(lv < rv);
                }
            }

            {
                rpt_value_real *rv2rp =
                    dynamic_cast<rpt_value_real *>(rv2.get());
                if (rv2rp)
                {
                    double rv = rv2rp->query();
                    return rpt_value_boolean::create(lv < rv);
                }
            }
        }
    }

    {
        rpt_value_real *lv2rp = dynamic_cast<rpt_value_real *>(lv2.get());
        if (lv2rp)
        {
            double lv = lv2rp->query();

            {
                rpt_value_integer *rv2ip =
                    dynamic_cast<rpt_value_integer *>(rv2.get());
                if (rv2ip)
                {
                    long rv = rv2ip->query();
                    return rpt_value_boolean::create(lv < rv);
                }
            }

            {
                rpt_value_real *rv2rp =
                    dynamic_cast<rpt_value_real *>(rv2.get());
                if (rv2rp)
                {
                    double rv = rv2rp->query();
                    return rpt_value_boolean::create(lv < rv);
                }
            }
        }
    }

    lv2 = rpt_value::stringize(lv1);
    rpt_value_string *lv2sp = dynamic_cast<rpt_value_string *>(lv2.get());
    rv2 = rpt_value::stringize(rv1);
    rpt_value_string *rv2sp = dynamic_cast<rpt_value_string *>(rv2.get());

    if (!lv2sp || !rv2sp)
    {
        sub_context_ty sc;
        sc.var_set_charstar("Name1", lv1->name());
        sc.var_set_charstar("Name2", rv1->name());
        nstring s(sc.subst_intl(i18n("illegal comparison ($name1 < $name2)")));
        return rpt_value_error::create(s);
    }

    return rpt_value_boolean::create(lv2sp->query() < rv2sp->query());
}


tree::pointer
tree_lt::optimize()
    const
{
    tree::pointer tp = create(get_left()->optimize(), get_right()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_lt::name()
    const
{
    return "<";
}


tree_le::~tree_le()
{
}


tree_le::tree_le(const tree::pointer &a1, const tree::pointer &a2) :
    tree_diadic(a1, a2)
{
}


tree::pointer
tree_le::create(const tree::pointer &a1, const tree::pointer &a2)
{
    return pointer(new tree_le(a1, a2));
}


rpt_value::pointer
tree_le::evaluate(string_ty *path_unres, string_ty *path, string_ty *path_res,
    struct stat *st) const
{
    //
    // evaluate the left hand side
    //
    trace(("le::evaluate()\n"));
    rpt_value::pointer lv1 =
        get_left()->evaluate(path_unres, path, path_res, st);
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
    rpt_value::pointer rv1 =
        get_right()->evaluate(path_unres, path, path_res, st);
    if (rv1->is_an_error())
	return rv1;

    //
    // coerce the right hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
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
            long lv = lv2ip->query();

            {
                rpt_value_integer *rv2ip =
                    dynamic_cast<rpt_value_integer *>(rv2.get());
                if (rv2ip)
                {
                    long rv = rv2ip->query();
                    return rpt_value_boolean::create(lv <= rv);
                }
            }

            {
                rpt_value_real *rv2rp =
                    dynamic_cast<rpt_value_real *>(rv2.get());
                if (rv2rp)
                {
                    double rv = rv2rp->query();
                    return rpt_value_boolean::create(lv <= rv);
                }
            }
        }
    }

    {
        rpt_value_real *lv2rp = dynamic_cast<rpt_value_real *>(lv2.get());
        if (lv2rp)
        {
            double lv = lv2rp->query();

            {
                rpt_value_integer *rv2ip =
                    dynamic_cast<rpt_value_integer *>(rv2.get());
                if (rv2ip)
                {
                    long rv = rv2ip->query();
                    return rpt_value_boolean::create(lv <= rv);
                }
            }

            {
                rpt_value_real *rv2rp =
                    dynamic_cast<rpt_value_real *>(rv2.get());
                if (rv2rp)
                {
                    double rv = rv2rp->query();
                    return rpt_value_boolean::create(lv <= rv);
                }
            }
        }
    }

    lv2 = rpt_value::stringize(lv1);
    rpt_value_string *lv2sp = dynamic_cast<rpt_value_string *>(lv2.get());
    rv2 = rpt_value::stringize(rv1);
    rpt_value_string *rv2sp = dynamic_cast<rpt_value_string *>(rv2.get());

    if (!lv2sp || !rv2sp)
    {
        sub_context_ty sc;
        sc.var_set_charstar("Name1", lv1->name());
        sc.var_set_charstar("Name2", rv1->name());
        nstring s(sc.subst_intl(i18n("illegal comparison ($name1 <= $name2)")));
        return rpt_value_error::create(s);
    }

    return rpt_value_boolean::create(lv2sp->query() <= rv2sp->query());
}


tree::pointer
tree_le::optimize()
    const
{
    tree::pointer tp = create(get_left()->optimize(), get_right()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_le::name()
    const
{
    return "<=";
}


tree_gt::~tree_gt()
{
}


tree_gt::tree_gt(const tree::pointer &a1, const tree::pointer &a2) :
    tree_diadic(a1, a2)
{
}


tree::pointer
tree_gt::create(const tree::pointer &a1, const tree::pointer &a2)
{
    return pointer(new tree_gt(a1, a2));
}


rpt_value::pointer
tree_gt::evaluate(string_ty *path_unres, string_ty *path, string_ty *path_res,
    struct stat *st) const
{
    //
    // evaluate the left hand side
    //
    trace(("gt::evaluate()\n"));
    rpt_value::pointer lv1 =
        get_left()->evaluate(path_unres, path, path_res, st);
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
    rpt_value::pointer rv1 =
        get_right()->evaluate(path_unres, path, path_res, st);
    if (rv1->is_an_error())
	return rv1;

    //
    // coerce the right hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
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
            long lv = lv2ip->query();

            {
                rpt_value_integer *rv2ip =
                    dynamic_cast<rpt_value_integer *>(rv2.get());
                if (rv2ip)
                {
                    long rv = rv2ip->query();
                    return rpt_value_boolean::create(lv > rv);
                }
            }

            {
                rpt_value_real *rv2rp =
                    dynamic_cast<rpt_value_real *>(rv2.get());
                if (rv2rp)
                {
                    double rv = rv2rp->query();
                    return rpt_value_boolean::create(lv > rv);
                }
            }
        }
    }

    {
        rpt_value_real *lv2rp = dynamic_cast<rpt_value_real *>(lv2.get());
        if (lv2rp)
        {
            double lv = lv2rp->query();

            {
                rpt_value_integer *rv2ip =
                    dynamic_cast<rpt_value_integer *>(rv2.get());
                if (rv2ip)
                {
                    long rv = rv2ip->query();
                    return rpt_value_boolean::create(lv > rv);
                }
            }

            {
                rpt_value_real *rv2rp =
                    dynamic_cast<rpt_value_real *>(rv2.get());
                if (rv2rp)
                {
                    double rv = rv2rp->query();
                    return rpt_value_boolean::create(lv > rv);
                }
            }
        }
    }

    lv2 = rpt_value::stringize(lv1);
    rpt_value_string *lv2sp = dynamic_cast<rpt_value_string *>(lv2.get());
    rv2 = rpt_value::stringize(rv1);
    rpt_value_string *rv2sp = dynamic_cast<rpt_value_string *>(rv2.get());

    if (!lv2sp || !rv2sp)
    {
        sub_context_ty sc;
        sc.var_set_charstar("Name1", lv1->name());
        sc.var_set_charstar("Name2", rv1->name());
        nstring s(sc.subst_intl(i18n("illegal comparison ($name1 > $name2)")));
        return rpt_value_error::create(s);
    }

    return rpt_value_boolean::create(lv2sp->query() > rv2sp->query());
}


tree::pointer
tree_gt::optimize()
    const
{
    tree::pointer tp = create(get_left()->optimize(), get_right()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_gt::name()
    const
{
    return ">";
}


tree_ge::~tree_ge()
{
}


tree_ge::tree_ge(const tree::pointer &a1, const tree::pointer &a2) :
    tree_diadic(a1, a2)
{
}


tree::pointer
tree_ge::create(const tree::pointer &a1, const tree::pointer &a2)
{
    return pointer(new tree_ge(a1, a2));
}


rpt_value::pointer
tree_ge::evaluate(string_ty *path_unres, string_ty *path, string_ty *path_res,
    struct stat *st) const
{
    //
    // evaluate the left hand side
    //
    trace(("ge::evaluate()\n"));
    rpt_value::pointer lv1 =
        get_left()->evaluate(path_unres, path, path_res, st);
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
    rpt_value::pointer rv1 =
        get_right()->evaluate(path_unres, path, path_res, st);
    if (rv1->is_an_error())
	return rv1;

    //
    // coerce the right hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
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
            long lv = lv2ip->query();

            {
                rpt_value_integer *rv2ip =
                    dynamic_cast<rpt_value_integer *>(rv2.get());
                if (rv2ip)
                {
                    long rv = rv2ip->query();
                    return rpt_value_boolean::create(lv >= rv);
                }
            }

            {
                rpt_value_real *rv2rp =
                    dynamic_cast<rpt_value_real *>(rv2.get());
                if (rv2rp)
                {
                    double rv = rv2rp->query();
                    return rpt_value_boolean::create(lv >= rv);
                }
            }
        }
    }

    {
        rpt_value_real *lv2rp = dynamic_cast<rpt_value_real *>(lv2.get());
        if (lv2rp)
        {
            double lv = lv2rp->query();

            {
                rpt_value_integer *rv2ip =
                    dynamic_cast<rpt_value_integer *>(rv2.get());
                if (rv2ip)
                {
                    long rv = rv2ip->query();
                    return rpt_value_boolean::create(lv >= rv);
                }
            }

            {
                rpt_value_real *rv2rp =
                    dynamic_cast<rpt_value_real *>(rv2.get());
                if (rv2rp)
                {
                    double rv = rv2rp->query();
                    return rpt_value_boolean::create(lv >= rv);
                }
            }
        }
    }

    lv2 = rpt_value::stringize(lv1);
    rpt_value_string *lv2sp = dynamic_cast<rpt_value_string *>(lv2.get());
    rv2 = rpt_value::stringize(rv1);
    rpt_value_string *rv2sp = dynamic_cast<rpt_value_string *>(rv2.get());

    if (!lv2sp || !rv2sp)
    {
        sub_context_ty sc;
        sc.var_set_charstar("Name1", lv1->name());
        sc.var_set_charstar("Name2", rv1->name());
        nstring s(sc.subst_intl(i18n("illegal comparison ($name1 >= $name2)")));
        return rpt_value_error::create(s);
    }

    return rpt_value_boolean::create(lv2sp->query() >= rv2sp->query());
}


tree::pointer
tree_ge::optimize()
    const
{
    tree::pointer tp = create(get_left()->optimize(), get_right()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_ge::name()
    const
{
    return ">=";
}


tree_eq::~tree_eq()
{
}


tree_eq::tree_eq(const tree::pointer &a1, const tree::pointer &a2) :
    tree_diadic(a1, a2)
{
}


tree::pointer
tree_eq::create(const tree::pointer &a1, const tree::pointer &a2)
{
    return pointer(new tree_eq(a1, a2));
}


rpt_value::pointer
tree_eq::evaluate(string_ty *path_unres, string_ty *path, string_ty *path_res,
    struct stat *st) const
{
    //
    // evaluate the left hand side
    //
    trace(("eq::evaluate()\n"));
    rpt_value::pointer lv1 =
        get_left()->evaluate(path_unres, path, path_res, st);
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
    rpt_value::pointer rv1 =
        get_right()->evaluate(path_unres, path, path_res, st);
    if (rv1->is_an_error())
	return rv1;

    //
    // coerce the right hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
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
            long lv = lv2ip->query();

            {
                rpt_value_integer *rv2ip =
                    dynamic_cast<rpt_value_integer *>(rv2.get());
                if (rv2ip)
                {
                    long rv = rv2ip->query();
                    return rpt_value_boolean::create(lv == rv);
                }
            }

            {
                rpt_value_real *rv2rp =
                    dynamic_cast<rpt_value_real *>(rv2.get());
                if (rv2rp)
                {
                    double rv = rv2rp->query();
                    return rpt_value_boolean::create(lv == rv);
                }
            }
        }
    }

    {
        rpt_value_real *lv2rp = dynamic_cast<rpt_value_real *>(lv2.get());
        if (lv2rp)
        {
            double lv = lv2rp->query();

            {
                rpt_value_integer *rv2ip =
                    dynamic_cast<rpt_value_integer *>(rv2.get());
                if (rv2ip)
                {
                    long rv = rv2ip->query();
                    return rpt_value_boolean::create(lv == rv);
                }
            }

            {
                rpt_value_real *rv2rp =
                    dynamic_cast<rpt_value_real *>(rv2.get());
                if (rv2rp)
                {
                    double rv = rv2rp->query();
                    return rpt_value_boolean::create(lv == rv);
                }
            }
        }
    }

    lv2 = rpt_value::stringize(lv1);
    rpt_value_string *lv2sp = dynamic_cast<rpt_value_string *>(lv2.get());
    rv2 = rpt_value::stringize(rv1);
    rpt_value_string *rv2sp = dynamic_cast<rpt_value_string *>(rv2.get());

    if (!lv2sp || !rv2sp)
    {
        sub_context_ty sc;
        sc.var_set_charstar("Name1", lv1->name());
        sc.var_set_charstar("Name2", rv1->name());
        nstring s(sc.subst_intl(i18n("illegal comparison ($name1 == $name2)")));
        return rpt_value_error::create(s);
    }

    return rpt_value_boolean::create(lv2sp->query() == rv2sp->query());
}


tree::pointer
tree_eq::optimize()
    const
{
    tree::pointer tp = create(get_left()->optimize(), get_right()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_eq::name()
    const
{
    return "==";
}


tree_ne::~tree_ne()
{
}


tree_ne::tree_ne(const tree::pointer &a1, const tree::pointer &a2) :
    tree_diadic(a1, a2)
{
}


tree::pointer
tree_ne::create(const tree::pointer &a1, const tree::pointer &a2)
{
    return pointer(new tree_ne(a1, a2));
}


rpt_value::pointer
tree_ne::evaluate(string_ty *path_unres, string_ty *path, string_ty *path_res,
    struct stat *st) const
{
    //
    // evaluate the left hand side
    //
    trace(("ne::evaluate()\n"));
    rpt_value::pointer lv1 =
        get_left()->evaluate(path_unres, path, path_res, st);
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
    rpt_value::pointer rv1 =
        get_right()->evaluate(path_unres, path, path_res, st);
    if (rv1->is_an_error())
	return rv1;

    //
    // coerce the right hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
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
            long lv = lv2ip->query();

            {
                rpt_value_integer *rv2ip =
                    dynamic_cast<rpt_value_integer *>(rv2.get());
                if (rv2ip)
                {
                    long rv = rv2ip->query();
                    return rpt_value_boolean::create(lv != rv);
                }
            }

            {
                rpt_value_real *rv2rp =
                    dynamic_cast<rpt_value_real *>(rv2.get());
                if (rv2rp)
                {
                    double rv = rv2rp->query();
                    return rpt_value_boolean::create(lv != rv);
                }
            }
        }
    }

    {
        rpt_value_real *lv2rp = dynamic_cast<rpt_value_real *>(lv2.get());
        if (lv2rp)
        {
            double lv = lv2rp->query();

            {
                rpt_value_integer *rv2ip =
                    dynamic_cast<rpt_value_integer *>(rv2.get());
                if (rv2ip)
                {
                    long rv = rv2ip->query();
                    return rpt_value_boolean::create(lv != rv);
                }
            }

            {
                rpt_value_real *rv2rp =
                    dynamic_cast<rpt_value_real *>(rv2.get());
                if (rv2rp)
                {
                    double rv = rv2rp->query();
                    return rpt_value_boolean::create(lv != rv);
                }
            }
        }
    }

    lv2 = rpt_value::stringize(lv1);
    rpt_value_string *lv2sp = dynamic_cast<rpt_value_string *>(lv2.get());
    rv2 = rpt_value::stringize(rv1);
    rpt_value_string *rv2sp = dynamic_cast<rpt_value_string *>(rv2.get());

    if (!lv2sp || !rv2sp)
    {
        sub_context_ty sc;
        sc.var_set_charstar("Name1", lv1->name());
        sc.var_set_charstar("Name2", rv1->name());
        nstring s(sc.subst_intl(i18n("illegal comparison ($name1 != $name2)")));
        return rpt_value_error::create(s);
    }

    return rpt_value_boolean::create(lv2sp->query() != rv2sp->query());
}


tree::pointer
tree_ne::optimize()
    const
{
    tree::pointer tp = create(get_left()->optimize(), get_right()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_ne::name()
    const
{
    return "!=";
}
