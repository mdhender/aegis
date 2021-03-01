//
//      aegis - project change supervisor
//      Copyright (C) 1994, 1996, 2003-2008 Peter Miller
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
#include <libaegis/aer/stmt/expr.h>
#include <libaegis/aer/value.h>


rpt_stmt_expr::~rpt_stmt_expr()
{
}


rpt_stmt_expr::rpt_stmt_expr(const rpt_expr::pointer &a_ep) :
    ep(a_ep)
{
}


rpt_stmt::pointer
rpt_stmt_expr::create(const rpt_expr::pointer &a_ep)
{
    return pointer(new rpt_stmt_expr(a_ep));
}


void
rpt_stmt_expr::run(rpt_stmt_result_ty *rp)
    const
{
    trace(("stmt_expr::run()\n{\n"));
    rpt_value::pointer vp = ep->evaluate(true, true);
    if (vp->is_an_error())
    {
        rp->status = rpt_stmt_status_error;
        rp->thrown = vp;
    }
    else
    {
        rp->status = rpt_stmt_status_normal;
        rp->thrown.reset();
    }
    trace(("}\n"));
}
