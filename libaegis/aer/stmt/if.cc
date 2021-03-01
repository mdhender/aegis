//
//      aegis - project change supervisor
//      Copyright (C) 1994-1996, 1999, 2003-2008 Peter Miller
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

#include <common/error.h>
#include <common/trace.h>
#include <libaegis/aer/expr.h>
#include <libaegis/aer/stmt/if.h>
#include <libaegis/aer/stmt/null.h>
#include <libaegis/aer/value/boolean.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/sub.h>


rpt_stmt_if::~rpt_stmt_if()
{
}


rpt_stmt_if::rpt_stmt_if(const rpt_expr::pointer &a_condition,
        const rpt_stmt::pointer &a_then, const rpt_stmt::pointer &a_else) :
    condition(a_condition)
{
    append(a_then);
    append(a_else);
}


rpt_stmt::pointer
rpt_stmt_if::create(const rpt_expr::pointer &a_condition,
    const rpt_stmt::pointer &a_then, const rpt_stmt::pointer &a_else)
{
    return pointer(new rpt_stmt_if(a_condition, a_then, a_else));
}


rpt_stmt::pointer
rpt_stmt_if::create(const rpt_expr::pointer &a_condition,
    const rpt_stmt::pointer &a_then)
{
    return create(a_condition, a_then, rpt_stmt_null::create());
}


void
rpt_stmt_if::run(rpt_stmt_result_ty *rp)
    const
{
    trace(("stmt_if::run()\n{\n"));
    assert(get_nchildren() == 2);
    rpt_value::pointer vp = condition->evaluate(true, true);
    if (vp->is_an_error())
    {
        rp->status = rpt_stmt_status_error;
        rp->thrown = vp;
        trace(("}\n"));
        return;
    }
    rpt_value::pointer vp2 = rpt_value::booleanize(vp);
    rpt_value_boolean *vp2_bool_p =
        dynamic_cast<rpt_value_boolean *>(vp2.get());
    if (!vp2_bool_p)
    {
        sub_context_ty sc;
        sc.var_set_charstar("Name", vp->name());
        nstring s
        (
            sc.subst_intl
            (
                i18n("boolean value required in if statement (not $name)")
            )
        );
        rp->status = rpt_stmt_status_error;
        rp->thrown = rpt_value_error::create(condition->get_pos(), s);
        trace(("}\n"));
        return;
    }
    int flag = !vp2_bool_p->query();
    trace(("condition was %s\n", (flag ? "false" : "true")));
    nth_child(flag)->run(rp);
    trace(("}\n"));
}
