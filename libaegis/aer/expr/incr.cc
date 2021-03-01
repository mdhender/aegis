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
#include <libaegis/aer/expr/incr.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/real.h>
#include <libaegis/aer/value/ref.h>
#include <libaegis/sub.h>


rpt_expr_inc_pre::~rpt_expr_inc_pre()
{
}


rpt_expr_inc_pre::rpt_expr_inc_pre(const rpt_expr::pointer &arg)
{
    append(arg);

    if (!arg->lvalue())
	arg->parse_error(i18n("modifiable lvalue required for increment"));
}


rpt_expr::pointer
rpt_expr_inc_pre::create(const rpt_expr::pointer &arg)
{
    return pointer(new rpt_expr_inc_pre(arg));
}


rpt_value::pointer
rpt_expr_inc_pre::evaluate()
    const
{
    assert(get_nchildren() == 1);
    rpt_value::pointer vp = nth_child(0)->evaluate(true, false);
    if (vp->is_an_error())
	return vp;
    rpt_value_reference *rvrp = dynamic_cast<rpt_value_reference *>(vp.get());
    if (!rvrp)
    {
        sub_context_ty sc;
	sc.var_set_charstar("Name", vp->name());
	nstring s
        (
	    sc.subst_intl
	    (
                i18n("modifiable lvalue required for increment (was "
                    "given $name)")
	    )
        );
	return rpt_value_error::create(nth_child(0)->get_pos(), s);
    }

    rpt_value::pointer v2 = rvrp->get();
    rpt_value::pointer v2a = rpt_value::arithmetic(v2);

    rpt_value_integer *v2aip = dynamic_cast<rpt_value_integer *>(v2a.get());
    if (v2aip)
    {
        rpt_value::pointer result =
            rpt_value_integer::create(1 + v2aip->query());
        rvrp->set(result);
        return result;
    }

    rpt_value_real *v2arp = dynamic_cast<rpt_value_real *>(v2a.get());
    if (v2arp)
    {
        rpt_value::pointer result = rpt_value_real::create(1 + v2arp->query());
        rvrp->set(result);
        return result;
    }

    sub_context_ty sc;
    sc.var_set_charstar("Name", v2->name());
    nstring s
    (
        sc.subst_intl
        (
            i18n("arithmetic type required for increment (was given $name)")
        )
    );
    return rpt_value_error::create(nth_child(0)->get_pos(), s);
}


rpt_expr_dec_pre::~rpt_expr_dec_pre()
{
}


rpt_expr_dec_pre::rpt_expr_dec_pre(const rpt_expr::pointer &arg)
{
    append(arg);

    if (!arg->lvalue())
	arg->parse_error(i18n("modifiable lvalue required for decrement"));
}


rpt_expr::pointer
rpt_expr_dec_pre::create(const rpt_expr::pointer &arg)
{
    return pointer(new rpt_expr_dec_pre(arg));
}


rpt_value::pointer
rpt_expr_dec_pre::evaluate()
    const
{
    assert(get_nchildren() == 1);
    rpt_value::pointer vp = nth_child(0)->evaluate(true, false);
    if (vp->is_an_error())
	return vp;
    rpt_value_reference *rvrp = dynamic_cast<rpt_value_reference *>(vp.get());
    if (!rvrp)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Name", vp->name());
	nstring s
        (
	    sc.subst_intl
	    (
                i18n("modifiable lvalue required for decrement (was "
                    "given $name)")
	    )
        );
	return rpt_value_error::create(nth_child(0)->get_pos(), s);
    }

    rpt_value::pointer v2 = rvrp->get();
    rpt_value::pointer v2a = rpt_value::arithmetic(v2);

    rpt_value_integer *v2aip = dynamic_cast<rpt_value_integer *>(v2a.get());
    if (v2aip)
    {
	rpt_value::pointer result =
            rpt_value_integer::create(v2aip->query() - 1);
        rvrp->set(result);
        return result;
    }

    rpt_value_real *v2arp = dynamic_cast<rpt_value_real *>(v2a.get());
    if (v2arp)
    {
	rpt_value::pointer result = rpt_value_real::create(v2arp->query() - 1);
        rvrp->set(result);
        return result;
    }

    sub_context_ty sc;
    sc.var_set_charstar("Name", v2a->name());
    nstring s
    (
        sc.subst_intl
        (
            i18n("arithmetic type required for decrement (was given $name)")
        )
    );
    return rpt_value_error::create(nth_child(0)->get_pos(), s);
}


rpt_expr_inc_post::~rpt_expr_inc_post()
{
}


rpt_expr_inc_post::rpt_expr_inc_post(const rpt_expr::pointer &arg)
{
    append(arg);

    if (!arg->lvalue())
	arg->parse_error(i18n("modifiable lvalue required for increment"));
}


rpt_expr::pointer
rpt_expr_inc_post::create(const rpt_expr::pointer &arg)
{
    return pointer(new rpt_expr_inc_post(arg));
}


rpt_value::pointer
rpt_expr_inc_post::evaluate()
    const
{
    assert(get_nchildren() == 1);
    rpt_value::pointer vp = nth_child(0)->evaluate(true, false);
    if (vp->is_an_error())
	return vp;
    rpt_value_reference *rvrp = dynamic_cast<rpt_value_reference *>(vp.get());
    if (!rvrp)
    {
        sub_context_ty sc;
	sc.var_set_charstar("Name", vp->name());
	nstring s
        (
	    sc.subst_intl
	    (
                i18n("modifiable lvalue required for increment (was "
                    "given $name)")
	    )
        );
	return rpt_value_error::create(nth_child(0)->get_pos(), s);
    }

    rpt_value::pointer v2 = rvrp->get();
    rpt_value::pointer v2a = rpt_value::arithmetic(v2);

    rpt_value_integer *v2aip = dynamic_cast<rpt_value_integer *>(v2a.get());
    if (v2aip)
    {
        rpt_value::pointer result =
            rpt_value_integer::create(1 + v2aip->query());
        rvrp->set(result);
        return v2a;
    }

    rpt_value_real *v2arp = dynamic_cast<rpt_value_real *>(v2a.get());
    if (v2arp)
    {
        rpt_value::pointer result = rpt_value_real::create(1 + v2arp->query());
        rvrp->set(result);
        return v2a;
    }

    sub_context_ty sc;
    sc.var_set_charstar("Name", v2->name());
    nstring s
    (
        sc.subst_intl
        (
            i18n("arithmetic type required for increment (was given $name)")
        )
    );
    return rpt_value_error::create(nth_child(0)->get_pos(), s);
}


rpt_expr_dec_post::~rpt_expr_dec_post()
{
}


rpt_expr_dec_post::rpt_expr_dec_post(const rpt_expr::pointer &arg)
{
    append(arg);

    if (!arg->lvalue())
	arg->parse_error(i18n("modifiable lvalue required for decrement"));
}


rpt_expr::pointer
rpt_expr_dec_post::create(const rpt_expr::pointer &arg)
{
    return pointer(new rpt_expr_dec_post(arg));
}


rpt_value::pointer
rpt_expr_dec_post::evaluate()
    const
{
    assert(get_nchildren() == 1);
    rpt_value::pointer vp = nth_child(0)->evaluate(true, false);
    if (vp->is_an_error())
	return vp;
    rpt_value_reference *rvrp = dynamic_cast<rpt_value_reference *>(vp.get());
    if (!rvrp)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Name", vp->name());
	nstring s
        (
	    sc.subst_intl
	    (
                i18n("modifiable lvalue required for decrement (was "
                    "given $name)")
	    )
        );
	return rpt_value_error::create(nth_child(0)->get_pos(), s);
    }

    rpt_value::pointer v2 = rvrp->get();
    rpt_value::pointer v2a = rpt_value::arithmetic(v2);

    rpt_value_integer *v2aip = dynamic_cast<rpt_value_integer *>(v2a.get());
    if (v2aip)
    {
        rvrp->set(rpt_value_integer::create(v2aip->query() - 1));
        return v2a;
    }

    rpt_value_real *v2arp = dynamic_cast<rpt_value_real *>(v2a.get());
    if (v2arp)
    {
        rvrp->set(rpt_value_real::create(v2arp->query() - 1));
        return v2a;
    }

    sub_context_ty sc;
    sc.var_set_charstar("Name", v2a->name());
    nstring s
    (
        sc.subst_intl
        (
            i18n("arithmetic type required for decrement (was given $name)")
        )
    );
    return rpt_value_error::create(nth_child(0)->get_pos(), s);
}
