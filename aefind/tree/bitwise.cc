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

#include <common/error.h>
#include <common/trace.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/sub.h>

#include <aefind/function/needs.h>
#include <aefind/tree/bitwise.h>
#include <aefind/tree/diadic.h>
#include <aefind/tree/list.h>
#include <aefind/tree/monadic.h>


tree_bitwise_and::~tree_bitwise_and()
{
}


tree_bitwise_and::tree_bitwise_and(const tree::pointer &a1,
        const tree::pointer &a2) :
    tree_diadic(a1, a2)
{
}


tree::pointer
tree_bitwise_and::create(const tree::pointer &a1, const tree::pointer &a2)
{
    return pointer(new tree_bitwise_and(a1, a2));
}


tree::pointer
tree_bitwise_and::create_l(const tree_list &args)
{
    function_needs_two("&", args);
    return create(args[0], args[1]);
}


rpt_value::pointer
tree_bitwise_and::evaluate(string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st) const
{
    rpt_value::pointer v1 =
        get_left()->evaluate(path_unres, path, path_res, st);
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
		i18n("integer value required for bit and (was given $name)")
	    )
        );
	return rpt_value_error::create(s);
    }

    rpt_value::pointer v2 =
        get_right()->evaluate(path_unres, path, path_res, st);
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
		i18n("integer value required for bit and (was given $name)")
	    )
        );
	return rpt_value_error::create(s);
    }

    return rpt_value_integer::create(v1ip->query() & v2ip->query());
}


tree::pointer
tree_bitwise_and::optimize()
    const
{
    tree::pointer tp = create(get_left()->optimize(), get_right()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_bitwise_and::name()
    const
{
    return "&";
}


tree_bitwise_xor::~tree_bitwise_xor()
{
}


tree_bitwise_xor::tree_bitwise_xor(const tree::pointer &a1,
        const tree::pointer &a2) :
    tree_diadic(a1, a2)
{
}


tree::pointer
tree_bitwise_xor::create(const tree::pointer &a1, const tree::pointer &a2)
{
    return pointer(new tree_bitwise_xor(a1, a2));
}


tree::pointer
tree_bitwise_xor::create_l(const tree_list &args)
{
    function_needs_two("^", args);
    return create(args[0], args[1]);
}


rpt_value::pointer
tree_bitwise_xor::evaluate(string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st) const
{
    rpt_value::pointer v1 =
        get_left()->evaluate(path_unres, path, path_res, st);
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
	return rpt_value_error::create(s);
    }

    rpt_value::pointer v2 =
        get_right()->evaluate(path_unres, path, path_res, st);
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
	return rpt_value_error::create(s);
    }

    return rpt_value_integer::create(v1ip->query() ^ v2ip->query());
}


tree::pointer
tree_bitwise_xor::optimize()
    const
{
    tree::pointer tp = create(get_left()->optimize(), get_right()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_bitwise_xor::name()
    const
{
    return "^";
}


tree_bitwise_or::~tree_bitwise_or()
{
}


tree_bitwise_or::tree_bitwise_or(const tree::pointer &a1,
        const tree::pointer &a2) :
    tree_diadic(a1, a2)
{
}


tree::pointer
tree_bitwise_or::create(const tree::pointer &a1, const tree::pointer &a2)
{
    return pointer(new tree_bitwise_or(a1, a2));
}


tree::pointer
tree_bitwise_or::create_l(const tree_list &args)
{
    function_needs_two("|", args);
    return create(args[0], args[1]);
}


rpt_value::pointer
tree_bitwise_or::evaluate(string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st) const
{
    rpt_value::pointer v1 =
        get_left()->evaluate(path_unres, path, path_res, st);
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
	return rpt_value_error::create(s);
    }

    rpt_value::pointer v2 =
        get_right()->evaluate(path_unres, path, path_res, st);
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
	return rpt_value_error::create(s);
    }

    return rpt_value_integer::create(v1ip->query() | v2ip->query());
}


tree::pointer
tree_bitwise_or::optimize()
    const
{
    tree::pointer tp = create(get_left()->optimize(), get_right()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_bitwise_or::name()
    const
{
    return "|";
}


tree_bitwise_not::~tree_bitwise_not()
{
}


tree_bitwise_not::tree_bitwise_not(const tree::pointer &a_arg) :
    tree_monadic(a_arg)
{
}


tree::pointer
tree_bitwise_not::create(const tree::pointer &a_arg)
{
    return pointer(new tree_bitwise_not(a_arg));
}


tree::pointer
tree_bitwise_not::create_l(const tree_list &args)
{
    function_needs_one("~", args);
    return create(args[0]);
}


rpt_value::pointer
tree_bitwise_not::evaluate(string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st) const
{
    //
    // evaluate the argument
    //
    trace(("not::evaluate()\n"));
    rpt_value::pointer v1 = get_arg()->evaluate(path_unres, path, path_res, st);
    if (v1->is_an_error())
	return v1;

    //
    // coerce the argument to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    rpt_value::pointer v2 = rpt_value::integerize(v1);

    //
    // the type of the result depends on
    // the types of the argument
    //
    rpt_value_integer *v2ip = dynamic_cast<rpt_value_integer *>(v2.get());
    if (!v2ip)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Name", v1->name());
	nstring s(sc.subst_intl(i18n("illegal bit not ($name)")));
	return rpt_value_error::create(s);
    }

    return rpt_value_integer::create(~v2ip->query());
}


tree::pointer
tree_bitwise_not::optimize()
    const
{
    tree::pointer tp = create(get_arg()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_bitwise_not::name()
    const
{
    return "~";
}


tree_shift_left::~tree_shift_left()
{
}


tree_shift_left::tree_shift_left(const tree::pointer &a1,
        const tree::pointer &a2) :
    tree_diadic(a1, a2)
{
}


tree::pointer
tree_shift_left::create(const tree::pointer &a1, const tree::pointer &a2)
{
    return pointer(new tree_shift_left(a1, a2));
}


tree::pointer
tree_shift_left::create_l(const tree_list &args)
{
    function_needs_two("<<", args);
    return create(args[0], args[1]);
}


rpt_value::pointer
tree_shift_left::evaluate(string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st) const
{
    rpt_value::pointer v1 =
        get_left()->evaluate(path_unres, path, path_res, st);
    if (v1->is_an_error())
	return v1;
    rpt_value::pointer v1i = rpt_value::integerize(v1);
    rpt_value_integer *v1ip = dynamic_cast<rpt_value_integer *>(v1i.get());

    rpt_value::pointer v2 =
        get_right()->evaluate(path_unres, path, path_res, st);
    if (v2->is_an_error())
	return v2;
    rpt_value::pointer v2i = rpt_value::integerize(v2);
    rpt_value_integer *v2ip = dynamic_cast<rpt_value_integer *>(v2i.get());

    if (!v1ip || !v2ip)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Name1", v1->name());
	sc.var_set_charstar("Name2", v2->name());
	nstring s(sc.subst_intl(i18n("illegal shift ($name1 << $name2)")));
	return rpt_value_error::create(s);
    }

    long v1n = v1ip->query();
    long v2n = v2ip->query();
    return rpt_value_integer::create(v1n << v2n);
}


tree::pointer
tree_shift_left::optimize()
    const
{
    tree::pointer tp = create(get_left()->optimize(), get_right()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_shift_left::name()
    const
{
    return "<<";
}


tree_shift_right::~tree_shift_right()
{
}


tree_shift_right::tree_shift_right(const tree::pointer &a1,
        const tree::pointer &a2) :
    tree_diadic(a1, a2)
{
}


tree::pointer
tree_shift_right::create(const tree::pointer &a1, const tree::pointer &a2)
{
    return pointer(new tree_shift_right(a1, a2));
}


tree::pointer
tree_shift_right::create_l(const tree_list &args)
{
    function_needs_two(">>", args);
    return create(args[0], args[1]);
}


rpt_value::pointer
tree_shift_right::evaluate(string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st) const
{
    rpt_value::pointer v1 =
        get_left()->evaluate(path_unres, path, path_res, st);
    if (v1->is_an_error())
	return v1;
    rpt_value::pointer v1i = rpt_value::integerize(v1);
    rpt_value_integer *v1ip = dynamic_cast<rpt_value_integer *>(v1i.get());

    rpt_value::pointer v2 =
        get_right()->evaluate(path_unres, path, path_res, st);
    if (v2->is_an_error())
	return v2;
    rpt_value::pointer v2i = rpt_value::integerize(v2);
    rpt_value_integer *v2ip = dynamic_cast<rpt_value_integer *>(v2i.get());

    if (!v1ip || !v2ip)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Name1", v1->name());
	sc.var_set_charstar("Name2", v2->name());
	nstring s(sc.subst_intl(i18n("illegal shift ($name1 >> $name2)")));
	return rpt_value_error::create(s);
    }

    long v1n = v1ip->query();
    long v2n = v2ip->query();
    return rpt_value_integer::create(v1n >> v2n);
}


tree::pointer
tree_shift_right::optimize()
    const
{
    tree::pointer tp = create(get_left()->optimize(), get_right()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_shift_right::name()
    const
{
    return ">>";
}
