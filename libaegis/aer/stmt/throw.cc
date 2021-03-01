//
//      aegis - project change supervisor
//      Copyright (C) 1996, 1999, 2003-2008 Peter Miller
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

#include <libaegis/aer/stmt/throw.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/string.h>
#include <common/str.h>
#include <libaegis/sub.h>
#include <common/trace.h>


rpt_stmt_throw::~rpt_stmt_throw()
{
}


rpt_stmt_throw::rpt_stmt_throw(const rpt_expr::pointer &a_ep) :
    ep(a_ep)
{
}


rpt_stmt::pointer
rpt_stmt_throw::create(const rpt_expr::pointer &a_ep)
{
    return pointer(new rpt_stmt_throw(a_ep));
}


void
rpt_stmt_throw::run(rpt_stmt_result_ty *rp)
    const
{
    trace(("throw::run()\n{\n"));
    rpt_value::pointer vp = ep->evaluate(true, true);
    if (vp->is_an_error())
    {
        rp->status = rpt_stmt_status_error;
        rp->thrown = vp;
        trace(("}\n"));
        return;
    }

    rpt_value::pointer vp2 = rpt_value::stringize(vp);
    rpt_value_string *vp2sp = dynamic_cast<rpt_value_string *>(vp2.get());
    if (!vp2sp)
    {
        sub_context_ty sc;
        sc.var_set_charstar("Name", vp2->name());
        nstring s
        (
            sc.subst_intl
            (
                i18n("throw statement requires string argument (not $name)")
            )
        );
        rp->status = rpt_stmt_status_error;
        rp->thrown = rpt_value_error::create(ep->get_pos(), s);
        trace(("}\n"));
        return;
    }

    rp->status = rpt_stmt_status_error;
    rp->thrown = rpt_value_error::create(ep->get_pos(), vp2sp->query());
    trace(("}\n"));
}
