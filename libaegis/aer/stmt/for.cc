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

#include <common/trace.h>
#include <libaegis/aer/expr.h>
#include <libaegis/aer/lex.h>
#include <libaegis/aer/stmt/for.h>
#include <libaegis/aer/value/boolean.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/list.h>
#include <libaegis/aer/value/null.h>
#include <libaegis/aer/value/ref.h>
#include <libaegis/sub.h>


rpt_stmt_for::~rpt_stmt_for()
{
}


rpt_stmt_for::rpt_stmt_for(const rpt_expr::pointer &e0,
    const rpt_expr::pointer &e1, const rpt_expr::pointer &e2,
    const rpt_stmt::pointer &body)
{
    e[0] = e0;
    e[1] = e1;
    e[2] = e2;
    append(body);
}


rpt_stmt::pointer
rpt_stmt_for::create(const rpt_expr::pointer &e0, const rpt_expr::pointer &e1,
    const rpt_expr::pointer &e2, const rpt_stmt::pointer &body)
{
    return pointer(new rpt_stmt_for(e0, e1, e2, body));
}


void
rpt_stmt_for::run(rpt_stmt_result_ty *rp)
    const
{
    //
    // evaluate the initializing expression
    //
    rpt_value::pointer vp = e[0]->evaluate(true, true);
    if (vp->is_an_error())
    {
       	rp->status = rpt_stmt_status_error;
       	rp->thrown = vp;
       	return;
    }

    //
    // loop until need to stop
    //
    for (;;)
    {
	//
	// evaluate the looping condition
	//
       	vp = e[1]->evaluate(true, true);
       	if (vp->is_an_error())
       	{
	    rp->status = rpt_stmt_status_error;
	    rp->thrown = vp;
	    return;
       	}

	//
	// coerce the looping condition to boolean
	//    it is an error if we can't
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
	    rp->thrown = rpt_value_error::create(e[1]->get_pos(), s);
	    return;
       	}

	//
	// if looping condition is false,
	//	stop looping
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
       	case rpt_stmt_status_error:
       	case rpt_stmt_status_return:
	    //
	    // immediate termination
	    //
	    trace(("loop exit\n"));
	    return;

       	case rpt_stmt_status_normal:
       	case rpt_stmt_status_continue:
	    //
	    // evaluate the increment expression
	    //
	    trace(("loop normal\n"));
	    vp = e[2]->evaluate(true, true);
	    if (vp->is_an_error())
	    {
	     	rp->status = rpt_stmt_status_error;
	     	rp->thrown = vp;
	     	return;
	    }

	    //
	    // do another iteration
	    //
	    continue;

       	case rpt_stmt_status_break:
	    //
	    // drop out of the loop
	    //
	    trace(("loop break\n"));
	    break;
       	}
       	break;
    }
    rp->status = rpt_stmt_status_normal;
    rp->thrown.reset();
}


rpt_stmt_foreach::~rpt_stmt_foreach()
{
}


rpt_stmt_foreach::rpt_stmt_foreach(const rpt_expr::pointer &e0,
    const rpt_expr::pointer &e1, const rpt_stmt::pointer &body)
{
    e[0] = e0;
    e[1] = e1;
    append(body);
    if (!e0->lvalue())
       	e0->parse_error(i18n("the loop variable must be modifiable"));
}


rpt_stmt::pointer
rpt_stmt_foreach::create(const rpt_expr::pointer &e0,
    const rpt_expr::pointer &e1, const rpt_stmt::pointer &body)
{
    return pointer(new rpt_stmt_foreach(e0, e1, body));
}


void
rpt_stmt_foreach::run(rpt_stmt_result_ty *rp)
    const
{
    //
    // evaluate the initializing expression
    //
    trace(("foreach::run()\n{\n"));
    rpt_value::pointer lhs = e[0]->evaluate(true, false);
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
	       	i18n("the loop variable must be modifiable (not $name)")
	    )
        );
       	rp->status = rpt_stmt_status_error;
       	rp->thrown = rpt_value_error::create(e[0]->get_pos(), s);
       	trace(("}\n"));
       	return;
    }

    rpt_value::pointer rhs = e[1]->evaluate(true, true);
    if (rhs->is_an_error())
    {
       	rp->status = rpt_stmt_status_error;
       	rp->thrown = rhs;
       	trace(("}\n"));
       	return;
    }
    rpt_value_null *rhs_null_p = dynamic_cast<rpt_value_null *>(rhs.get());
    if (rhs_null_p)
    {
       	//
	// pretend nul is the empty list, and do nothing
	//
       	trace(("loop list nul\n"));
       	rp->status = rpt_stmt_status_normal;
       	rp->thrown.reset();
       	trace(("}\n"));
       	return;
    }
    rpt_value_list *rhs_list_p = dynamic_cast<rpt_value_list *>(rhs.get());
    if (!rhs_list_p)
    {
       	sub_context_ty sc;
       	sc.var_set_charstar("Name", rhs->name());
       	nstring s
        (
	    sc.subst_intl
	    (
	       	i18n("the loop iteration value must be a list (not $name)")
	    )
        );
       	rp->status = rpt_stmt_status_error;
       	rp->thrown = rpt_value_error::create(e[1]->get_pos(), s);
       	trace(("}\n"));
       	return;
    }

    //
    // loop across all values
    //
    size_t rhs_length = rhs_list_p->size();
    for (size_t j = 0; j < rhs_length; ++j)
    {
       	//
	// set the loop variable
	//
       	trace(("loop index %ld;\n", (long)j));
        rpt_value::pointer vp = rhs_list_p->nth(j);
       	lhs_ref_p->set(vp);

       	//
	// run the inner statement
	//
       	nth_child(0)->run(rp);
       	switch (rp->status)
       	{
       	case rpt_stmt_status_error:
       	case rpt_stmt_status_return:
	    //
	    // immediate termination
	    //
	    trace(("loop exit\n"));
	    trace(("}\n"));
	    return;

       	case rpt_stmt_status_normal:
       	case rpt_stmt_status_continue:
	    //
	    // do another iteration
	    //
	    trace(("loop normal\n"));
	    continue;

       	case rpt_stmt_status_break:
	    //
	    // drop out of the loop
	    //
	    trace(("loop break\n"));
	    break;
       	}
       	break;
    }
    trace(("loop complete\n"));
    rp->status = rpt_stmt_status_normal;
    rp->thrown.reset();
    trace(("}\n"));
}


rpt_stmt_break::~rpt_stmt_break()
{
}


rpt_stmt_break::rpt_stmt_break()
{
}


rpt_stmt::pointer
rpt_stmt_break::create()
{
    return pointer(new rpt_stmt_break());
}


void
rpt_stmt_break::run(rpt_stmt_result_ty *rp)
    const
{
    trace(("rpt_stmt_break::run();\n"));
    rp->status = rpt_stmt_status_break;
    rp->thrown.reset();
}


rpt_stmt_continue::~rpt_stmt_continue()
{
}


rpt_stmt_continue::rpt_stmt_continue()
{
}


rpt_stmt::pointer
rpt_stmt_continue::create()
{
    return pointer(new rpt_stmt_continue());
}


void
rpt_stmt_continue::run(rpt_stmt_result_ty *rp)
    const
{
    trace(("rpt_stmt_continue::run();\n"));
    rp->status = rpt_stmt_status_continue;
    rp->thrown.reset();
}
