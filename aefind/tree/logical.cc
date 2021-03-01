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

#include <common/ac/stdio.h>

#include <common/str.h>
#include <libaegis/aer/value/boolean.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/sub.h>

#include <aefind/function/needs.h>
#include <aefind/tree/constant.h>
#include <aefind/tree/diadic.h>
#include <aefind/tree/list.h>
#include <aefind/tree/logical.h>
#include <aefind/tree/monadic.h>


tree_and::~tree_and()
{
}


tree_and::tree_and(const tree::pointer &a1, const tree::pointer &a2) :
    tree_diadic(a1, a2)
{
}


tree::pointer
tree_and::create(const tree::pointer &a1, const tree::pointer &a2)
{
    return pointer(new tree_and(a1, a2));
}


tree::pointer
tree_and::create_l(const tree_list &args)
{
    function_needs_two("&&", args);
    return create(args[0], args[1]);
}


rpt_value::pointer
tree_and::evaluate(string_ty *path_unres, string_ty *path, string_ty *path_res,
    struct stat *st) const
{
    rpt_value::pointer v1 =
        get_left()->evaluate(path_unres, path, path_res, st);
    if (v1->is_an_error())
	return v1;
    rpt_value::pointer v1b = rpt_value::booleanize(v1);
    rpt_value_boolean *v1bp = dynamic_cast<rpt_value_boolean *>(v1b.get());
    if (!v1bp)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Name", v1->name());
	nstring s
        (
	    sc.subst_intl
	    (
		i18n("boolean value required for logical and (was given $name)")
	    )
        );
	return rpt_value_error::create(s);
    }
    if (!v1bp->query())
	return v1b;

    rpt_value::pointer v2 =
        get_right()->evaluate(path_unres, path, path_res, st);
    if (v2->is_an_error())
	return v2;
    rpt_value::pointer v2b = rpt_value::booleanize(v2);
    rpt_value_boolean *v2bp = dynamic_cast<rpt_value_boolean *>(v2b.get());
    if (!v2bp)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Name", v2->name());
	nstring s
        (
	    sc.subst_intl
	    (
		i18n("boolean value required for logical and (was given $name)")
	    )
        );
	return rpt_value_error::create(s);
    }
    return v2b;
}


tree::pointer
tree_and::optimize()
    const
{
    tree::pointer l2 = get_left()->optimize();
    tree::pointer r2 = get_right()->optimize();
    if (l2->constant())
    {
        //
        // left is constant
        //
        rpt_value::pointer lv = l2->evaluate_constant();
        rpt_value_boolean *lvbp = dynamic_cast<rpt_value_boolean *>(lv.get());
        if (!lvbp)
            return create(l2, r2);
        bool flag = lvbp->query();
        return (flag ? r2 : l2);
    }

    if (!r2->constant())
    {
        //
        // neither left nor right are constant
        //
        return create(l2, r2);
    }

    //
    // right is constant, but left is not
    //
    rpt_value::pointer rv = r2->evaluate_constant();
    rpt_value_boolean *rvbp = dynamic_cast<rpt_value_boolean *>(rv.get());
    if (!rvbp)
    {
        return create(l2, r2);
    }
    bool flag = rvbp->query();
    return (flag ? l2 : r2);
}


const char *
tree_and::name()
    const
{
    return "&&";
}


tree_or::~tree_or()
{
}


tree_or::tree_or(const tree::pointer &a1, const tree::pointer &a2) :
    tree_diadic(a1, a2)
{
}


tree::pointer
tree_or::create(const tree::pointer &a1, const tree::pointer &a2)
{
    return pointer(new tree_or(a1, a2));
}


tree::pointer
tree_or::create_l(const tree_list &args)
{
    function_needs_two("||", args);
    return create(args[0], args[1]);
}


rpt_value::pointer
tree_or::evaluate(string_ty *path_unres, string_ty *path, string_ty *path_res,
    struct stat *st) const
{
    rpt_value::pointer v1 =
        get_left()->evaluate(path_unres, path, path_res, st);
    if (v1->is_an_error())
	return v1;
    rpt_value::pointer v1b = rpt_value::booleanize(v1);
    rpt_value_boolean *v1bp = dynamic_cast<rpt_value_boolean *>(v1b.get());
    if (!v1bp)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Name", v1->name());
	nstring s
        (
	    sc.subst_intl
	    (
		i18n("boolean value required for logical or (was given $name)")
	    )
        );
	return rpt_value_error::create(s);
    }
    if (v1bp->query())
	return v1b;

    rpt_value::pointer v2 =
        get_right()->evaluate(path_unres, path, path_res, st);
    if (v2->is_an_error())
	return v2;
    rpt_value::pointer v2b = rpt_value::booleanize(v2);
    rpt_value_boolean *v2bp = dynamic_cast<rpt_value_boolean *>(v2b.get());
    if (!v2bp)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Name", v2->name());
	nstring s
        (
	    sc.subst_intl
	    (
		i18n("boolean value required for logical or (was given $name)")
	    )
        );
	return rpt_value_error::create(s);
    }
    return v2b;
}


tree::pointer
tree_or::optimize()
    const
{
    tree::pointer l2 = get_left()->optimize();
    tree::pointer r2 = get_right()->optimize();
    if (l2->constant())
    {
        //
        // left is constant
        //
        rpt_value::pointer lv = l2->evaluate_constant();
        rpt_value_boolean *lvbp = dynamic_cast<rpt_value_boolean *>(lv.get());
        if (!lvbp)
        {
            return create(l2, r2);
        }
        bool flag = lvbp->query();
        return (flag ? l2 : r2);
    }

    if (!r2->constant())
    {
        //
        // neither left nor right are constant
        //
        return create(l2, r2);
    }

    //
    // right is constant, but left is not
    //
    rpt_value::pointer rv = r2->evaluate_constant();
    rpt_value_boolean *rvbp = dynamic_cast<rpt_value_boolean *>(rv.get());
    if (!rvbp)
    {
        return create(l2, r2);
    }
    bool flag = rvbp->query();
    return (flag ? r2 : l2);
}


const char *
tree_or::name()
    const
{
    return "||";
}


tree_not::~tree_not()
{
}


tree_not::tree_not(const tree::pointer &a_arg) :
    tree_monadic(a_arg)
{
}


tree::pointer
tree_not::create(const tree::pointer &a_arg)
{
    return pointer(new tree_not(a_arg));
}


tree::pointer
tree_not::create_l(const tree_list &args)
{
    function_needs_one("!", args);
    return create(args[0]);
}


rpt_value::pointer
tree_not::evaluate(string_ty *path_unres, string_ty *path, string_ty *path_res,
    struct stat *st) const
{
    rpt_value::pointer v1 = get_arg()->evaluate(path_unres, path, path_res, st);
    if (v1->is_an_error())
	return v1;
    rpt_value::pointer v2 = rpt_value::booleanize(v1);
    rpt_value_boolean *v2bp = dynamic_cast<rpt_value_boolean *>(v2.get());
    if (!v2bp)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Name", v1->name());
	nstring s(sc.subst_intl(i18n("illegal logical not ($name)")));
	return rpt_value_error::create(s);
    }
    return rpt_value_boolean::create(!v2bp->query());
}


tree::pointer
tree_not::optimize()
    const
{
    tree::pointer tp = create(get_arg()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_not::name()
    const
{
    return "!";
}


tree_comma::~tree_comma()
{
}


tree_comma::tree_comma(const tree::pointer &a1, const tree::pointer &a2) :
    tree_diadic(a1, a2)
{
}


tree::pointer
tree_comma::create(const tree::pointer &a1, const tree::pointer &a2)
{
    return pointer(new tree_comma(a1, a2));
}


tree::pointer
tree_comma::create_l(const tree_list &args)
{
    function_needs_two(",", args);
    return create(args[0], args[1]);
}


rpt_value::pointer
tree_comma::evaluate(string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st) const
{
    rpt_value::pointer vp =
        get_left()->evaluate(path_unres, path, path_res, st);
    if (vp->is_an_error())
	return vp;

    return get_right()->evaluate(path_unres, path, path_res, st);
}


tree::pointer
tree_comma::optimize()
    const
{
    tree::pointer tp = create(get_left()->optimize(), get_right()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_comma::name()
    const
{
    return ",";
}


tree_triadic::~tree_triadic()
{
}


tree_triadic::tree_triadic(const tree::pointer &a_a1, const tree::pointer &a_a2,
        const tree::pointer &a_a3) :
    a1(a_a1),
    a2(a_a2),
    a3(a_a3)
{
}


tree::pointer
tree_triadic::create(const tree::pointer &a_a1, const tree::pointer &a_a2,
    const tree::pointer &a_a3)
{
    return pointer(new tree_triadic(a_a1, a_a2, a_a3));
}


void
tree_triadic::print()
    const
{
    printf("( ( ");
    a1->print();
    printf(" ) ? ( ");
    a2->print();
    printf(" ) : ( ");
    a3->print();
    printf(" ) )");
}


rpt_value::pointer
tree_triadic::evaluate(string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st) const
{
    rpt_value::pointer v1 = a1->evaluate(path_unres, path, path_res, st);
    if (v1->is_an_error())
	return v1;
    rpt_value::pointer v1b = rpt_value::booleanize(v1);
    rpt_value_boolean *v1bp = dynamic_cast<rpt_value_boolean *>(v1b.get());
    if (!v1bp)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Name", v1->name());
	nstring s
        (
	    sc.subst_intl
	    (
		i18n("boolean value required for if (was given $name)")
	    )
        );
	return rpt_value_error::create(s);
    }
    bool which = v1bp->query();

    return (which ? a2 : a3)->evaluate(path_unres, path, path_res, st);
}


bool
tree_triadic::useful()
    const
{
    return (a1->useful() || a2->useful() || a3->useful());
}


bool
tree_triadic::constant()
    const
{
    if (!a1->constant())
	return false;
    rpt_value::pointer v1 = a1->evaluate_constant();
    if (v1->is_an_error())
    {
	return false;
    }
    rpt_value::pointer v1b = rpt_value::booleanize(v1);
    rpt_value_boolean *v1bp = dynamic_cast<rpt_value_boolean *>(v1b.get());
    if (!v1bp)
    {
	return false;
    }
    bool which = v1bp->query();

    return (which ? a2 : a3)->constant();
}


tree::pointer
tree_triadic::optimize()
    const
{
    if (!a1->constant())
    {
	tree::pointer arg1 = a1->optimize();
	tree::pointer arg2 = a2->optimize();
	tree::pointer arg3 = a3->optimize();
	return tree_triadic::create(arg1, arg2, arg3);
    }
    rpt_value::pointer v1 = a1->evaluate_constant();
    if (v1->is_an_error())
    {
	tree::pointer result = tree_constant::create(v1);
	return result;
    }
    rpt_value::pointer v1b = rpt_value::booleanize(v1);
    rpt_value_boolean *v1bp = dynamic_cast<rpt_value_boolean *>(v1b.get());
    if (!v1bp)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Name", v1->name());
	nstring s
        (
	    sc.subst_intl
	    (
		i18n("boolean value required for if (was given $name)")
	    )
        );
	return tree_constant::create(rpt_value_error::create(s));
    }
    bool which = v1bp->query();

    return (which ? a2 : a3)->optimize();
}


const char *
tree_triadic::name()
    const
{
    return "?:";
}
