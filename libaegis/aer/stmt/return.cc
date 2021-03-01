//
//      aegis - project change supervisor
//      Copyright (C) 1996, 2003-2008 Peter Miller
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
#include <libaegis/aer/stmt/return.h>
#include <libaegis/aer/value/void.h>
#include <common/trace.h>


rpt_stmt_return::~rpt_stmt_return()
{
}


rpt_stmt_return::rpt_stmt_return(const rpt_expr::pointer &a_ep) :
    ep(a_ep)
{
}


rpt_stmt::pointer
rpt_stmt_return::create(const rpt_expr::pointer &a_ep)
{
    return pointer(new rpt_stmt_return(a_ep));
}


rpt_stmt::pointer
rpt_stmt_return::create()
{
    return pointer(new rpt_stmt_return(rpt_expr::pointer()));
}


void
rpt_stmt_return::run(rpt_stmt_result_ty *rp)
    const
{
    trace(("return::run()\n{\n"));
    rp->status = rpt_stmt_status_return;
    if (ep)
        rp->thrown = ep->evaluate(false, true);
    else
        rp->thrown = rpt_value_void::create();
    trace(("}\n"));
}
