//
//	aegis - project change supervisor
//	Copyright (C) 1997, 1999, 2002-2008 Peter Miller
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

#include <common/ac/math.h>

#include <common/error.h> // for assert
#include <common/str.h>
#include <common/trace.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/real.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/sub.h>

#include <aefind/function/needs.h>
#include <aefind/tree/arithmetic.h>
#include <aefind/tree/list.h>


#define PAIR(a, b)	((a) * rpt_value_type_MAX + (b))


tree_mul::~tree_mul()
{
}


tree_mul::tree_mul(const tree::pointer &a1, const tree::pointer &a2) :
    tree_diadic(a1, a2)
{
}


tree::pointer
tree_mul::create(const tree::pointer &a1, const tree::pointer &a2)
{
    return pointer(new tree_mul(a1, a2));
}


tree::pointer
tree_mul::create_l(const tree_list &args)
{
    function_needs_two("*", args);
    return create(args[0], args[1]);
}


rpt_value::pointer
tree_mul::evaluate(string_ty *path_unres, string_ty *path, string_ty *path_res,
    struct stat *st) const
{
    //
    // evaluate the left hand side
    //
    trace(("tree_mul::evaluate()\n"));
    rpt_value::pointer v1 =
        get_left()->evaluate(path_unres, path, path_res, st);
    if (v1->is_an_error())
    {
	trace(("}\n"));
	return v1;
    }

    //
    // coerce the left hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    rpt_value::pointer v1a = rpt_value::arithmetic(v1);

    //
    // evaluate the right hand side
    //
    rpt_value::pointer v2 =
        get_right()->evaluate(path_unres, path, path_res, st);
    if (v2->is_an_error())
    {
	trace(("}\n"));
	return v2;
    }

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
        rpt_value_integer *v1a_ip =
            dynamic_cast<rpt_value_integer *>(v1a.get());
        if (v1a_ip)
        {
            long v1n = v1a_ip->query();

            {
                rpt_value_integer *v2a_ip =
                    dynamic_cast<rpt_value_integer *>(v2a.get());
                if (v2a_ip)
                {
                    long v2n = v2a_ip->query();
                    return rpt_value_integer::create(v1n * v2n);
                }
            }

            {
                rpt_value_real *v2a_rp =
                    dynamic_cast<rpt_value_real *>(v2a.get());
                if (v2a_rp)
                {
                    double v2n = v2a_rp->query();
                    return rpt_value_real::create(v1n * v2n);
                }
            }
        }
    }

    {
        rpt_value_real *v1a_rp = dynamic_cast<rpt_value_real *>(v1a.get());
        if (v1a_rp)
        {
            double v1n = v1a_rp->query();

            {
                rpt_value_integer *v2a_ip =
                    dynamic_cast<rpt_value_integer *>(v2a.get());
                if (v2a_ip)
                {
                    long v2n = v2a_ip->query();
                    return rpt_value_real::create(v1n * v2n);
                }
            }

            {
                rpt_value_real *v2a_rp =
                    dynamic_cast<rpt_value_real *>(v2a.get());
                if (v2a_rp)
                {
                    double v2n = v2a_rp->query();
                    return rpt_value_real::create(v1n * v2n);
                }
            }
        }
    }

    sub_context_ty sc;
    sc.var_set_charstar("Name1", v1->name());
    sc.var_set_charstar("Name2", v2->name());
    nstring s(sc.subst_intl(i18n("illegal multiplication ($name1 * $name2)")));
    return rpt_value_error::create(s);
}


tree::pointer
tree_mul::optimize()
    const
{
    tree::pointer tp = create(get_left()->optimize(), get_right()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_mul::name()
    const
{
    return "*";
}


tree_divide::~tree_divide()
{
}


tree_divide::tree_divide(const tree::pointer &a1, const tree::pointer &a2) :
    tree_diadic(a1, a2)
{
}


tree::pointer
tree_divide::create(const tree::pointer &a1, const tree::pointer &a2)
{
    return pointer(new tree_divide(a1, a2));
}


tree::pointer
tree_divide::create_l(const tree_list &args)
{
    function_needs_two("/", args);
    return create(args[0], args[1]);
}


static rpt_value::pointer
divide_by_zero_error()
{
    sub_context_ty sc;
    nstring s(sc.subst_intl(i18n("division by zero")));
    return rpt_value_error::create(s);
}


rpt_value::pointer
tree_divide::evaluate(string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st) const
{
    //
    // evaluate the left hand side
    //
    trace(("divide::evaluate()\n"));
    rpt_value::pointer v1 =
        get_left()->evaluate(path_unres, path, path_res, st);
    if (v1->is_an_error())
    {
	trace(("}\n"));
	return v1;
    }

    //
    // coerce the left hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    rpt_value::pointer v1a = rpt_value::arithmetic(v1);

    //
    // evaluate the right hand side
    //
    rpt_value::pointer v2 =
        get_right()->evaluate(path_unres, path, path_res, st);
    if (v2->is_an_error())
    {
	trace(("}\n"));
	return v2;
    }

    //
    // coerce the right hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    rpt_value::pointer v2a = rpt_value::arithmetic(v2);

    //
    // the type of the result depends on
    // the types of the operands
    //
    rpt_value_integer *v1a_ip = dynamic_cast<rpt_value_integer *>(v1a.get());
    if (v1a_ip)
    {
        long num = v1a_ip->query();

        rpt_value_integer *v2a_ip =
            dynamic_cast<rpt_value_integer *>(v2a.get());
        if (v2a_ip)
        {
            long den = v2a_ip->query();
            if (den == 0)
                return divide_by_zero_error();
            return rpt_value_integer::create(num / den);
        }

        rpt_value_real *v2a_rp = dynamic_cast<rpt_value_real *>(v2a.get());
        if (v2a_rp)
        {
            double den = v2a_rp->query();
            if (den == 0)
                return divide_by_zero_error();
            return rpt_value_real::create(num / den);
        }
    }

    rpt_value_real *v1a_rp = dynamic_cast<rpt_value_real *>(v1a.get());
    if (v1a_rp)
    {
        double num = v1a_rp->query();

        rpt_value_integer *v2a_ip =
            dynamic_cast<rpt_value_integer *>(v2a.get());
        if (v2a_ip)
        {
            long den = v2a_ip->query();
            if (den == 0)
                return divide_by_zero_error();
            return rpt_value_real::create(num / den);
        }

        rpt_value_real *v2a_rp = dynamic_cast<rpt_value_real *>(v2a.get());
        if (v2a_rp)
        {
            double den = v2a_rp->query();
            if (den == 0)
                return divide_by_zero_error();
            return rpt_value_real::create(num / den);
        }
    }

    sub_context_ty sc;
    sc.var_set_charstar("Name1", v1a->name());
    sc.var_set_charstar("Name2", v2a->name());
    nstring s(sc.subst_intl(i18n("illegal division ($name1 / $name2)")));
    return rpt_value_error::create(s);
}


tree::pointer
tree_divide::optimize()
    const
{
    tree::pointer tp = create(get_left()->optimize(), get_right()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_divide::name()
    const
{
    return "/";
}


tree_mod::~tree_mod()
{
}


tree_mod::tree_mod(const tree::pointer &a1, const tree::pointer &a2) :
    tree_diadic(a1, a2)
{
}


tree::pointer
tree_mod::create(const tree::pointer &a1, const tree::pointer &a2)
{
    return pointer(new tree_mod(a1, a2));
}


tree::pointer
tree_mod::create_l(const tree_list &args)
{
    function_needs_two("%", args);
    return create(args[0], args[1]);
}


static rpt_value::pointer
modulo_by_zero_error()
{
    sub_context_ty sc;
    nstring s(sc.subst_intl(i18n("modulo by zero")));
    return rpt_value_error::create(s);
}


rpt_value::pointer
tree_mod::evaluate(string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st) const
{
    //
    // evaluate the left hand side
    //
    trace(("mod::evaluate()\n"));
    rpt_value::pointer v1 =
        get_left()->evaluate(path_unres, path, path_res, st);
    if (v1->is_an_error())
    {
	trace(("}\n"));
	return v1;
    }

    //
    // coerce the left hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    rpt_value::pointer v1a = rpt_value::arithmetic(v1);

    //
    // evaluate the right hand side
    //
    rpt_value::pointer v2 =
        get_right()->evaluate(path_unres, path, path_res, st);
    if (v2->is_an_error())
    {
	trace(("}\n"));
	return v2;
    }

    //
    // coerce the right hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    rpt_value::pointer v2a = rpt_value::arithmetic(v2);

    //
    // the type of the result depends on
    // the types of the operands
    //
    rpt_value_integer *v1a_ip = dynamic_cast<rpt_value_integer *>(v1a.get());
    if (v1a_ip)
    {
        long num = v1a_ip->query();

        rpt_value_integer *v2a_ip =
            dynamic_cast<rpt_value_integer *>(v2a.get());
        if (v2a_ip)
        {
            long den = v2a_ip->query();
            if (den == 0)
                return modulo_by_zero_error();
            return rpt_value_integer::create(num % den);
        }

        rpt_value_real *v2a_rp = dynamic_cast<rpt_value_real *>(v2a.get());
        if (v2a_rp)
        {
            double den = v2a_rp->query();
            if (den == 0)
                return modulo_by_zero_error();
            return rpt_value_real::create(fmod(num, den));
        }
    }

    rpt_value_real *v1a_rp = dynamic_cast<rpt_value_real *>(v1a.get());
    if (v1a_rp)
    {
        double num = v1a_rp->query();

        rpt_value_integer *v2a_ip =
            dynamic_cast<rpt_value_integer *>(v2a.get());
        if (v2a_ip)
        {
            long den = v2a_ip->query();
            if (den == 0)
                return modulo_by_zero_error();
            return rpt_value_real::create(fmod(num, den));
        }

        rpt_value_real *v2a_rp = dynamic_cast<rpt_value_real *>(v2a.get());
        if (v2a_rp)
        {
            double den = v2a_rp->query();
            if (den == 0)
                return modulo_by_zero_error();
            return rpt_value_real::create(fmod(num, den));
        }
    }

    sub_context_ty sc;
    sc.var_set_charstar("Name1", v1a->name());
    sc.var_set_charstar("Name2", v2a->name());
    // xgettext:no-c-format
    nstring s(sc.subst_intl(i18n("illegal modulo ($name1 % $name2)")));
    return rpt_value_error::create(s);
}


tree::pointer
tree_mod::optimize()
    const
{
    tree::pointer tp = create(get_left()->optimize(), get_right()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_mod::name()
    const
{
    return "%";
}


tree_neg::~tree_neg()
{
}


tree_neg::tree_neg(const tree::pointer &a_arg) :
    tree_monadic(a_arg)
{
}


tree::pointer
tree_neg::create(const tree::pointer &a_arg)
{
    return pointer(new tree_neg(a_arg));
}


tree::pointer
tree_neg::create_l(const tree_list &args)
{
    function_needs_one("-", args);
    return create(args[0]);
}


rpt_value::pointer
tree_neg::evaluate(string_ty *path_unres, string_ty *path, string_ty *path_res,
    struct stat *st) const
{
    //
    // evaluate the argument
    //
    trace(("neg::evaluate()\n"));
    rpt_value::pointer v1 = get_arg()->evaluate(path_unres, path, path_res, st);
    if (v1->is_an_error())
    {
	trace(("}\n"));
	return v1;
    }

    //
    // coerce the argument to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    rpt_value::pointer v2 = rpt_value::arithmetic(v1);

    //
    // the type of the result depends on
    // the types of the argument
    //
    rpt_value_integer *v2_ip = dynamic_cast<rpt_value_integer *>(v2.get());
    if (v2_ip)
    {
        long n = v2_ip->query();
        return rpt_value_integer::create(-n);
    }

    rpt_value_real *v2_rp = dynamic_cast<rpt_value_real *>(v2.get());
    if (v2_rp)
    {
        double n = v2_rp->query();
        return rpt_value_real::create(-n);
    }

    sub_context_ty sc;
    sc.var_set_charstar("Name", v2->name());
    nstring s(sc.subst_intl(i18n("illegal negative ($name)")));
    return rpt_value_error::create(s);
}


tree::pointer
tree_neg::optimize()
    const
{
    tree::pointer tp = create(get_arg()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_neg::name()
    const
{
    return "-";
}


tree_pos::~tree_pos()
{
}


tree_pos::tree_pos(const tree::pointer &a_arg) :
    tree_monadic(a_arg)
{
}


tree::pointer
tree_pos::create(const tree::pointer &a_arg)
{
    return pointer(new tree_pos(a_arg));
}


tree::pointer
tree_pos::create_l(const tree_list &args)
{
    function_needs_one("+", args);
    return create(args[0]);
}


rpt_value::pointer
tree_pos::evaluate(string_ty *path_unres, string_ty *path, string_ty *path_res,
    struct stat *st) const
{
    //
    // evaluate the argument
    //
    trace(("pos::evaluate()\n"));
    rpt_value::pointer v1 = get_arg()->evaluate(path_unres, path, path_res, st);
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
    if
    (
        !dynamic_cast<rpt_value_integer *>(v2.get())
    &&
        !dynamic_cast<rpt_value_real *>(v2.get())
    )
    {
        sub_context_ty sc;
        sc.var_set_charstar("Name", v2->name());
        nstring s(sc.subst_intl(i18n("illegal positive ($name)")));
        return rpt_value_error::create(s);
    }

    return v2;
}


tree::pointer
tree_pos::optimize()
    const
{
    tree::pointer tp = create(get_arg()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_pos::name()
    const
{
    return "+";
}


tree_plus::~tree_plus()
{
}


tree_plus::tree_plus(const tree::pointer &a1, const tree::pointer &a2) :
    tree_diadic(a1, a2)
{
}


tree::pointer
tree_plus::create(const tree::pointer &a1, const tree::pointer &a2)
{
    return pointer(new tree_plus(a1, a2));
}


tree::pointer
tree_plus::create_l(const tree_list &args)
{
    function_needs_two("+", args);
    return create(args[0], args[1]);
}


rpt_value::pointer
tree_plus::evaluate(string_ty *path_unres, string_ty *path, string_ty *path_res,
    struct stat *st) const
{
    //
    // evaluate the left hand side
    //
    trace(("plus::evaluate()\n"));
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
    // the type of the result depends on
    // the types of the operands
    //
    {
        rpt_value_integer *lv2_ip =
            dynamic_cast<rpt_value_integer *>(lv2.get());
        if (lv2_ip)
        {
            long v1n = lv2_ip->query();

            {
                rpt_value_integer *rv2_ip =
                    dynamic_cast<rpt_value_integer *>(rv2.get());
                if (rv2_ip)
                {
                    long v2n = rv2_ip->query();
                    return rpt_value_integer::create(v1n + v2n);
                }
            }

            {
                rpt_value_real *rv2_rp =
                    dynamic_cast<rpt_value_real *>(rv2.get());
                if (rv2_rp)
                {
                    double v2n = rv2_rp->query();
                    return rpt_value_real::create(v1n + v2n);
                }
            }
        }
    }

    {
        rpt_value_real *lv2_rp = dynamic_cast<rpt_value_real *>(lv2.get());
        if (lv2_rp)
        {
            double v1n = lv2_rp->query();

            {
                rpt_value_integer *rv2_ip =
                    dynamic_cast<rpt_value_integer *>(rv2.get());
                if (rv2_ip)
                {
                    long v2n = rv2_ip->query();
                    return rpt_value_real::create(v1n + v2n);
                }
            }

            {
                rpt_value_real *rv2_rp =
                    dynamic_cast<rpt_value_real *>(rv2.get());
                if (rv2_rp)
                {
                    double v2n = rv2_rp->query();
                    return rpt_value_real::create(v1n + v2n);
                }
            }
        }
    }

    sub_context_ty sc;
    sc.var_set_charstar("Name1", lv2->name());
    sc.var_set_charstar("Name2", rv2->name());
    nstring s(sc.subst_intl(i18n("illegal addition ($name1 + $name2)")));
    return rpt_value_error::create(s);
}


tree::pointer
tree_plus::optimize()
    const
{
    tree::pointer tp = create(get_left()->optimize(), get_right()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_plus::name()
    const
{
    return "+";
}


tree_subtract::~tree_subtract()
{
}


tree_subtract::tree_subtract(const tree::pointer &a1, const tree::pointer &a2) :
    tree_diadic(a1, a2)
{
}


tree::pointer
tree_subtract::create(const tree::pointer &a1, const tree::pointer &a2)
{
    return pointer(new tree_subtract(a1, a2));
}


tree::pointer
tree_subtract::create_l(const tree_list &args)
{
    function_needs_two("-", args);
    return create(args[0], args[1]);
}


rpt_value::pointer
tree_subtract::evaluate(string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st) const
{
    //
    // evaluate the left hand side
    //
    trace(("subtract::evaluate()\n"));
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
    // the type of the result depends on
    // the types of the operands
    //
    {
        rpt_value_integer *lv2_ip =
            dynamic_cast<rpt_value_integer *>(lv2.get());
        if (lv2_ip)
        {
            long v1n = lv2_ip->query();

            {
                rpt_value_integer *rv2_ip =
                    dynamic_cast<rpt_value_integer *>(rv2.get());
                if (rv2_ip)
                {
                    long v2n = rv2_ip->query();
                    return rpt_value_integer::create(v1n - v2n);
                }
            }

            {
                rpt_value_real *rv2_rp =
                    dynamic_cast<rpt_value_real *>(rv2.get());
                if (rv2_rp)
                {
                    double v2n = rv2_rp->query();
                    return rpt_value_real::create(v1n - v2n);
                }
            }
        }
    }

    {
        rpt_value_real *lv2_rp = dynamic_cast<rpt_value_real *>(lv2.get());
        if (lv2_rp)
        {
            double v1n = lv2_rp->query();

            {
                rpt_value_integer *rv2_ip =
                    dynamic_cast<rpt_value_integer *>(rv2.get());
                if (rv2_ip)
                {
                    long v2n = rv2_ip->query();
                    return rpt_value_real::create(v1n - v2n);
                }
            }

            {
                rpt_value_real *rv2_rp =
                    dynamic_cast<rpt_value_real *>(rv2.get());
                if (rv2_rp)
                {
                    double v2n = rv2_rp->query();
                    return rpt_value_real::create(v1n - v2n);
                }
            }
        }
    }

    sub_context_ty sc;
    sc.var_set_charstar("Name1", lv2->name());
    sc.var_set_charstar("Name2", rv2->name());
    nstring s(sc.subst_intl(i18n("illegal subtraction ($name1 - $name2)")));
    return rpt_value_error::create(s);
}


tree::pointer
tree_subtract::optimize()
    const
{
    tree::pointer tp = create(get_left()->optimize(), get_right()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_subtract::name()
    const
{
    return "-";
}


tree_join::~tree_join()
{
}


tree_join::tree_join(const tree::pointer &a1, const tree::pointer &a2) :
    tree_diadic(a1, a2)
{
}


tree::pointer
tree_join::create(const tree::pointer &a1, const tree::pointer &a2)
{
    return pointer(new tree_join(a1, a2));
}


tree::pointer
tree_join::create_l(const tree_list &args)
{
    function_needs_two(":", args);
    return create(args[0], args[1]);
}


rpt_value::pointer
tree_join::evaluate(string_ty *path_unres, string_ty *path, string_ty *path_res,
    struct stat *st) const
{
    //
    // evaluate the left hand side
    //
    trace(("join::evaluate()\n"));
    rpt_value::pointer v1 =
        get_left()->evaluate(path_unres, path, path_res, st);
    if (v1->is_an_error())
    {
	trace(("}\n"));
	return v1;
    }

    //
    // evaluate the right hand side
    //
    rpt_value::pointer v2 =
        get_right()->evaluate(path_unres, path, path_res, st);
    if (v2->is_an_error())
    {
	trace(("}\n"));
	return v2;
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
	return rpt_value_string::create(s);
    }

    sub_context_ty sc;
    sc.var_set_charstar("Name1", v1s->name());
    sc.var_set_charstar("Name2", v2s->name());
    nstring s(sc.subst_intl(i18n("illegal join ($name1 ## $name2)")));
    return rpt_value_error::create(s);
}


tree::pointer
tree_join::optimize()
    const
{
    tree::pointer tp = create(get_left()->optimize(), get_right()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_join::name()
    const
{
    return ":";
}
