//
// aegis - project change supervisor
// Copyright (C) 1996, 1999, 2002-2008, 2012 Peter Miller
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

#include <common/str.h>
#include <common/trace.h>
#include <libaegis/aer/stmt/try.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/ref.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/sub.h>


rpt_stmt_try::~rpt_stmt_try()
{
}


rpt_stmt_try::rpt_stmt_try(const rpt_stmt::pointer &s1,
        const rpt_expr::pointer &a_variable, const rpt_stmt::pointer &s2) :
    variable(a_variable)
{
    append(s1);
    append(s2);
    if (!variable->lvalue())
        variable->parse_error(i18n("the catch variable must be modifiable"));
}


rpt_stmt::pointer
rpt_stmt_try::create(const rpt_stmt::pointer &s1, const rpt_expr::pointer &e,
    const rpt_stmt::pointer &s2)
{
    return pointer(new rpt_stmt_try(s1, e, s2));
}


void
rpt_stmt_try::run(rpt_stmt_result_ty *rp)
    const
{
    //
    // evaluate the catch variable's address
    //
    trace(("try::run()\n{\n"));
    rpt_value::pointer lhs = variable->evaluate(true, false);
    if (lhs->is_an_error())
    {
        rp->status = rpt_stmt_status_error;
        rp->thrown = lhs;
        trace(("}\n"));
        return;
    }
    rpt_value_reference *lhs_ref_p =
        dynamic_cast<rpt_value_reference *>(lhs.get());
    if (!lhs_ref_p)
    {
        sub_context_ty sc;
        sc.var_set_charstar("Name", lhs->name());
        nstring s
        (
            sc.subst_intl
            (
                i18n("the catch variable must be modifiable (not $name)")
            )
        );
        rp->status = rpt_stmt_status_error;
        rp->thrown = rpt_value_error::create(variable->get_pos(), s);
        trace(("}\n"));
        return;
    }

    //
    // run the try clause
    //
    assert(get_nchildren() == 2);
    nth_child(0)->run(rp);
    if (rp->status == rpt_stmt_status_error)
    {
        //
        // set the catch variable
        // to the <string> of the error
        // without the file position
        //
        rpt_value_error *thrown_err_p =
            dynamic_cast<rpt_value_error *>(rp->thrown.get());
        assert(thrown_err_p);
        if (thrown_err_p)
        {
            rpt_value::pointer vp =
                rpt_value_string::create(thrown_err_p->query());
            lhs_ref_p->set(vp);
        }
        else
        {
            lhs_ref_p->set(rp->thrown);
        }

        //
        // run the catch clause
        //
        nth_child(1)->run(rp);
    }
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
