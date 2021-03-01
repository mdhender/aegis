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

#include <libaegis/aer/expr.h>
#include <libaegis/aer/lex.h>
#include <libaegis/aer/stmt/while.h>
#include <libaegis/aer/value/boolean.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/sub.h>


rpt_stmt_while::~rpt_stmt_while()
{
}


rpt_stmt_while::rpt_stmt_while(const rpt_expr::pointer &a_condition,
        const rpt_stmt::pointer &sub) :
    condition(a_condition)
{
    append(sub);
}


rpt_stmt::pointer
rpt_stmt_while::create(const rpt_expr::pointer &e, const rpt_stmt::pointer &sub)
{
    return pointer(new rpt_stmt_while(e, sub));
}


void
rpt_stmt_while::run(rpt_stmt_result_ty *rp)
    const
{
    //
    // loop until you drop
    //
    for (;;)
    {
        //
        // evaluate the looping condition
        //
        rpt_value::pointer vp = condition->evaluate(true, true);
        if (vp->is_an_error())
        {
            rp->status = rpt_stmt_status_error;
            rp->thrown = vp;
            return;
        }

        //
        // coerce the looping condition to boolean
        //      it is an error if we can't
        //
        rpt_value::pointer vp2 = rpt_value::booleanize(vp);
        rpt_value_boolean *vp2bp = dynamic_cast<rpt_value_boolean *>(vp2.get());
        if (!vp2bp)
        {
            sub_context_ty sc;
            sc.var_set_charstar("Name", vp2->name());
            nstring s
            (
                sc.subst_intl
                (
                    i18n("loop condition must be boolean (not $name)")
                )
            );
            rp->status = rpt_stmt_status_error;
            rp->thrown = rpt_value_error::create(condition->get_pos(), s);
            return;
        }

        //
        // if looping condition is false,
        //      stop looping
        //
        if (!vp2bp->query())
        {
            break;
        }

        //
        // run the inner statement
        //
        nth_child(0)->run(rp);
        switch (rp->status)
        {
        case rpt_stmt_status_normal:
        case rpt_stmt_status_continue:
            continue;

        case rpt_stmt_status_error:
        case rpt_stmt_status_return:
            return;

        case rpt_stmt_status_break:
            break;
        }
        break;
    }
    rp->status = rpt_stmt_status_normal;
    rp->thrown.reset();
}


rpt_stmt_do::~rpt_stmt_do()
{
}


rpt_stmt_do::rpt_stmt_do(const rpt_stmt::pointer &sub,
        const rpt_expr::pointer &a_condition) :
    condition(a_condition)
{
    append(sub);
}


rpt_stmt::pointer
rpt_stmt_do::create(const rpt_stmt::pointer &sub, const rpt_expr::pointer &e)
{
    return pointer(new rpt_stmt_do(sub, e));
}


void
rpt_stmt_do::run(rpt_stmt_result_ty *rp)
    const
{
    //
    // loop until you drop
    //
    for (;;)
    {
        //
        // run the inner statement
        //
        nth_child(0)->run(rp);
        if (rp->status == rpt_stmt_status_break)
            break;
        if
        (
            rp->status != rpt_stmt_status_normal
        &&
            rp->status != rpt_stmt_status_continue
        )
            return;

        //
        // evaluate the looping condition
        //
        rpt_value::pointer vp = condition->evaluate(true, true);
        if (vp->is_an_error())
        {
            rp->status = rpt_stmt_status_error;
            rp->thrown = vp;
            return;
        }

        //
        // coerce the looping condition to boolean
        //      it is an error if we can't
        //
        rpt_value::pointer vp2 = rpt_value::booleanize(vp);
        rpt_value_boolean *vp2bp = dynamic_cast<rpt_value_boolean *>(vp2.get());
        if (!vp2bp)
        {
            sub_context_ty sc;
            sc.var_set_charstar("Name", vp2->name());
            nstring s
            (
                sc.subst_intl
                (
                    i18n("loop condition must be boolean (not $name)")
                )
            );
            rp->status = rpt_stmt_status_error;
            rp->thrown = rpt_value_error::create(condition->get_pos(), s);
            return;
        }

        //
        // if looping condition is false,
        //      stop looping
        //
        if (!vp2bp->query())
        {
            break;
        }
    }
    rp->status = rpt_stmt_status_normal;
    rp->thrown.reset();
}
