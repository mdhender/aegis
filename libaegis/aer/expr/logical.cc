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

#include <common/trace.h>
#include <libaegis/aer/expr/logical.h>
#include <libaegis/aer/value/boolean.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/sub.h>


rpt_expr_and_logical::~rpt_expr_and_logical()
{
}


rpt_expr_and_logical::rpt_expr_and_logical(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    append(lhs);
    append(rhs);
}


rpt_expr::pointer
rpt_expr_and_logical::create(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    return pointer(new rpt_expr_and_logical(lhs, rhs));
}


rpt_value::pointer
rpt_expr_and_logical::evaluate()
    const
{
    //
    // eveluate the left hand side
    //
    assert(get_nchildren() == 2);
    rpt_value::pointer v1 = nth_child(0)->evaluate(true, true);
    if (v1->is_an_error())
        return v1;
    rpt_value::pointer v1b = rpt_value::booleanize(v1);
    rpt_value_boolean *v1p = dynamic_cast<rpt_value_boolean *>(v1b.get());
    if (!v1p)
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
        return rpt_value_error::create(nth_child(0)->get_pos(), s);
    }

    //
    // short circuit the evaluation if the LHS is false
    //
    if (!v1p->query())
        return v1b;

    //
    // evaluate the right hand side
    //
    rpt_value::pointer v2 = nth_child(1)->evaluate(true, true);
    if (v2->is_an_error())
        return v2;
    rpt_value::pointer v2b = rpt_value::booleanize(v2);
    rpt_value_boolean *v2p = dynamic_cast<rpt_value_boolean *>(v2b.get());
    if (!v2p)
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
        return rpt_value_error::create(nth_child(1)->get_pos(), s);
    }
    return v2b;
}


rpt_expr_or_logical::~rpt_expr_or_logical()
{
}


rpt_expr_or_logical::rpt_expr_or_logical(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    append(lhs);
    append(rhs);
}


rpt_expr::pointer
rpt_expr_or_logical::create(const rpt_expr::pointer &lhs,
    const rpt_expr::pointer &rhs)
{
    return pointer(new rpt_expr_or_logical(lhs, rhs));
}


rpt_value::pointer
rpt_expr_or_logical::evaluate()
    const
{
    //
    // evaluate the left hand side
    //
    assert(get_nchildren() == 2);
    rpt_value::pointer v1 = nth_child(0)->evaluate(true, true);
    if (v1->is_an_error())
        return v1;
    rpt_value::pointer v1b = rpt_value::booleanize(v1);
    rpt_value_boolean *v1p = dynamic_cast<rpt_value_boolean *>(v1b.get());
    if (!v1p)
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
        return rpt_value_error::create(nth_child(0)->get_pos(), s);
    }

    //
    // short circuit the evaluation if LHS is true
    //
    if (v1p->query())
        return v1b;

    //
    // evaluate the right hand side
    //
    rpt_value::pointer v2 = nth_child(1)->evaluate(true, true);
    if (v2->is_an_error())
        return v2;
    rpt_value::pointer v2b = rpt_value::booleanize(v2);
    rpt_value_boolean *v2p = dynamic_cast<rpt_value_boolean *>(v2b.get());
    if (!v2p)
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
        return rpt_value_error::create(nth_child(1)->get_pos(), s);
    }
    return v2b;
}


rpt_expr_not_logical::~rpt_expr_not_logical()
{
}


rpt_expr_not_logical::rpt_expr_not_logical(const rpt_expr::pointer &arg)
{
    append(arg);
}


rpt_expr::pointer
rpt_expr_not_logical::create(const rpt_expr::pointer &arg)
{
    return pointer(new rpt_expr_not_logical(arg));
}


rpt_value::pointer
rpt_expr_not_logical::evaluate()
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
    // coerce the argument to boolean type
    //  (will not give error if can't, will copy instead)
    //
    rpt_value::pointer v2 = rpt_value::booleanize(v1);

    //
    // the type of the result depends on
    // the types of the argument
    //
    rpt_value_boolean *v2p = dynamic_cast<rpt_value_boolean *>(v2.get());
    if (!v2p)
    {
        sub_context_ty sc;
        sc.var_set_charstar("Name", v2->name());
        nstring s(sc.subst_intl(i18n("illegal logical not ($name)")));
        rpt_value::pointer result =
            rpt_value_error::create(nth_child(0)->get_pos(), s);
        trace(("}\n"));
        return result;
    }

    rpt_value::pointer result = rpt_value_boolean::create(!v2p->query());
    trace(("return %p;\n", result.get()));
    trace(("}\n"));
    return result;
}


rpt_expr_if::~rpt_expr_if()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


rpt_expr_if::rpt_expr_if(const rpt_expr::pointer &e1,
    const rpt_expr::pointer &e2, const rpt_expr::pointer &e3)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    append(e1);
    append(e2);
    append(e3);
}


rpt_expr::pointer
rpt_expr_if::create(const rpt_expr::pointer &e1, const rpt_expr::pointer &e2,
    const rpt_expr::pointer &e3)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer(new rpt_expr_if(e1, e2, e3));
}


rpt_value::pointer
rpt_expr_if::evaluate()
    const
{
    //
    // evaluate the argument
    //
    trace(("rpt_expr_if::evaluate()\n{\n"));
    assert(get_nchildren() == 3);
    rpt_value::pointer v1 = nth_child(0)->evaluate(true, true);
    if (v1->is_an_error())
    {
        trace(("}\n"));
        return v1;
    }

    //
    // coerce the argument to boolean type
    // (will not give error if can't, will copy instead)
    //
    rpt_value::pointer v1b = rpt_value::booleanize(v1);
    trace(("v1b->name() = \"%s\"\n", v1b->name()));
    rpt_value_boolean *v1p = dynamic_cast<rpt_value_boolean *>(v1b.get());
    if (!v1p)
    {
        trace(("mark\n"));
        sub_context_ty sc;
        sc.var_set_charstar("Name", v1b->name());
        nstring s
        (
            sc.subst_intl
            (
                i18n("boolean value required for arithmetic if (was "
                    "given $name)")
            )
        );
        rpt_value::pointer result =
            rpt_value_error::create(nth_child(0)->get_pos(), s);
        trace(("}\n"));
        return result;
    }

    trace(("mark\n"));
    bool cond = v1p->query();
    rpt_value::pointer result = nth_child(cond ? 1 : 2)->evaluate(true, true);

    trace(("return %p;\n", result.get()));
    trace(("}\n"));
    return result;
}


// vim: set ts=8 sw=4 et :
