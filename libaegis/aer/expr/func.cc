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

#include <common/mem.h>
#include <libaegis/aer/expr/func.h>
#include <libaegis/aer/func.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/func.h>
#include <libaegis/sub.h>


rpt_expr_func::~rpt_expr_func()
{
}


rpt_expr_func::rpt_expr_func(const rpt_expr::pointer &e1,
    const rpt_expr::pointer &e2)
{
    append(e1);
    append(e2);
}


rpt_expr::pointer
rpt_expr_func::create(const rpt_expr::pointer &e1, const rpt_expr::pointer &e2)
{
    return pointer(new rpt_expr_func(e1, e2));
}


rpt_value::pointer
rpt_expr_func::evaluate()
    const
{
    //
    // get the function pointer from the first argument
    //
    assert(get_nchildren() == 2);
    rpt_value::pointer vp = nth_child(0)->evaluate(true, true);
    if (vp->is_an_error())
        return vp;
    rpt_value_func *rvfp = dynamic_cast<rpt_value_func *>(vp.get());
    if (!rvfp)
    {
        sub_context_ty sc;
        sc.var_set_charstar("Name", vp->name());
        nstring s(sc.subst_intl(i18n("invalid function name ($name)")));
        return rpt_value_error::create(nth_child(0)->get_pos(), s);
    }
    rpt_func::pointer fp = rvfp->query();

    //
    // get the argument list from the second argument
    //
    rpt_expr::pointer ep = nth_child(1);
    if (!fp->verify(ep))
    {
        sub_context_ty sc;
        nstring s(sc.subst_intl(i18n("invalid function arguments")));
        return rpt_value_error::create(get_pos(), s);
    }
    size_t argc = ep->get_nchildren();
    rpt_value::pointer *argv = new rpt_value::pointer [argc];
    for (size_t j = 0; j < argc; ++j)
    {
        vp = ep->nth_child(j)->evaluate(true, true);
        argv[j] = vp;
        if (vp->is_an_error())
        {
            delete [] argv;
            return vp;
        }
    }

    //
    // evaluate the function
    //
    vp = fp->run(ep, argc, argv);

    //
    // free the argument list
    //
    delete [] argv;

    //
    // return the function return value
    //
    return vp;
}


// vim: set ts=8 sw=4 et :
