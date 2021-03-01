//
//	aegis - project change supervisor
//	Copyright (C) 1997, 1999, 2003-2008 Peter Miller
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
#include <libaegis/aer/expr.h>
#include <libaegis/aer/func/round.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/real.h>
#include <libaegis/sub.h>


rpt_func_round::~rpt_func_round()
{
}


rpt_func_round::rpt_func_round()
{
}


rpt_func::pointer
rpt_func_round::create()
{
    return pointer(new rpt_func_round());
}


const char *
rpt_func_round::name()
    const
{
    return "round";
}


bool
rpt_func_round::optimizable()
    const
{
    return true;
}


bool
rpt_func_round::verify(const rpt_expr::pointer &ep)
    const
{
    return (ep->get_nchildren() == 1);
}


rpt_value::pointer
rpt_func_round::run(const rpt_expr::pointer &ep, size_t,
    rpt_value::pointer *argv) const
{
    //
    // See if it looks like a number.
    //
    rpt_value::pointer tmp = rpt_value::realize(argv[0]);
    rpt_value_real *rvrp = dynamic_cast<rpt_value_real *>(tmp.get());
    if (!rvrp)
    {
        sub_context_ty sc;
        sc.var_set_charstar("Function", "round");
        sc.var_set_charstar("Name", argv[0]->name());
        sc.var_set_long("Number", 1);
        nstring s
        (
            sc.subst_intl
            (
                i18n("$function: argument $number: real value required "
                    "(was given $name)")
            )
        );
        return rpt_value_error::create(ep->get_pos(), s);
    }

    double n = rvrp->query();
    n = floor(n + 0.5);
    return rpt_value_real::create(n);
}


rpt_func_floor::~rpt_func_floor()
{
}


rpt_func_floor::rpt_func_floor()
{
}


rpt_func::pointer
rpt_func_floor::create()
{
    return pointer(new rpt_func_floor());
}


const char *
rpt_func_floor::name()
    const
{
    return "floor";
}


bool
rpt_func_floor::optimizable()
    const
{
    return true;
}


bool
rpt_func_floor::verify(const rpt_expr::pointer &ep)
    const
{
    return (ep->get_nchildren() == 1);
}


rpt_value::pointer
rpt_func_floor::run(const rpt_expr::pointer &ep, size_t,
    rpt_value::pointer *argv) const
{
    //
    // See if it looks like a number.
    //
    rpt_value::pointer tmp = rpt_value::realize(argv[0]);
    rpt_value_real *rvrp = dynamic_cast<rpt_value_real *>(tmp.get());
    if (!rvrp)
    {
        sub_context_ty sc;
        sc.var_set_charstar("Function", "floor");
        sc.var_set_charstar("Name", argv[0]->name());
        sc.var_set_long("Number", 1);
        nstring s
        (
            sc.subst_intl
            (
                i18n("$function: argument $number: real value required "
                    "(was given $name)")
            )
        );
        return rpt_value_error::create(ep->get_pos(), s);
    }

    double n = rvrp->query();
    n = floor(n);
    return rpt_value_real::create(n);
}


rpt_func_ceil::~rpt_func_ceil()
{
}


rpt_func_ceil::rpt_func_ceil()
{
}


rpt_func::pointer
rpt_func_ceil::create()
{
    return pointer(new rpt_func_ceil());
}


const char *
rpt_func_ceil::name()
    const
{
    return "ceil";
}


bool
rpt_func_ceil::optimizable()
    const
{
    return true;
}


bool
rpt_func_ceil::verify(const rpt_expr::pointer &ep)
    const
{
    return (ep->get_nchildren() == 1);
}


rpt_value::pointer
rpt_func_ceil::run(const rpt_expr::pointer &ep, size_t,
    rpt_value::pointer *argv) const
{
    //
    // See if it looks like a number.
    //
    rpt_value::pointer tmp = rpt_value::realize(argv[0]);
    rpt_value_real *rvrp = dynamic_cast<rpt_value_real *>(tmp.get());
    if (!rvrp)
    {
        sub_context_ty sc;
        sc.var_set_charstar("Function", "ceil");
        sc.var_set_charstar("Name", argv[0]->name());
        sc.var_set_long("Number", 1);
        nstring s
        (
            sc.subst_intl
            (
                i18n("$function: argument $number: real value required "
                    "(was given $name)")
            )
        );
        return rpt_value_error::create(ep->get_pos(), s);
    }

    double n = rvrp->query();
    n = ceil(n);
    return rpt_value_real::create(n);
}


rpt_func_trunc::~rpt_func_trunc()
{
}


rpt_func_trunc::rpt_func_trunc()
{
}


rpt_func::pointer
rpt_func_trunc::create()
{
    return pointer(new rpt_func_trunc());
}


const char *
rpt_func_trunc::name()
    const
{
    return "trunc";
}


bool
rpt_func_trunc::optimizable()
    const
{
    return true;
}


bool
rpt_func_trunc::verify(const rpt_expr::pointer &ep)
    const
{
    return (ep->get_nchildren() == 1);
}


rpt_value::pointer
rpt_func_trunc::run(const rpt_expr::pointer &ep, size_t,
    rpt_value::pointer *argv) const
{
    //
    // See if it looks like a number.
    //
    rpt_value::pointer tmp = rpt_value::realize(argv[0]);
    rpt_value_real *rvrp = dynamic_cast<rpt_value_real *>(tmp.get());
    if (!rvrp)
    {
        sub_context_ty sc;
        sc.var_set_charstar("Function", "trunc");
        sc.var_set_charstar("Name", argv[0]->name());
        sc.var_set_long("Number", 1);
        nstring s
        (
            sc.subst_intl
            (
                i18n("$function: argument $number: real value required "
                    "(was given $name)")
            )
        );
        return rpt_value_error::create(ep->get_pos(), s);
    }

    double n = rvrp->query();
    if (n >= 0)
        n = floor(n);
    else
        n = ceil(n);
    return rpt_value_real::create(n);
}
