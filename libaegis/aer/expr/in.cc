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
#include <libaegis/aer/expr/constant.h>
#include <libaegis/aer/expr/in.h>
#include <libaegis/aer/expr/rel.h>
#include <libaegis/aer/lex.h>
#include <libaegis/aer/value/boolean.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/list.h>
#include <libaegis/sub.h>


rpt_expr_in::~rpt_expr_in()
{
}


rpt_expr_in::rpt_expr_in(const rpt_expr::pointer &a, const rpt_expr::pointer &b)
{
    append(a);
    append(b);
}


rpt_expr::pointer
rpt_expr_in::create(const rpt_expr::pointer &a, const rpt_expr::pointer &b)
{
    return pointer(new rpt_expr_in(a, b));
}


rpt_value::pointer
rpt_expr_in::evaluate()
    const
{
    rpt_value::pointer lhs = nth_child(0)->evaluate(true, true);
    if (lhs->is_an_error())
	return lhs;

    rpt_value::pointer rhs = nth_child(1)->evaluate(true, true);
    if (rhs->is_an_error())
    {
	return rhs;
    }
    rpt_value_list *rvlp = dynamic_cast<rpt_value_list *>(rhs.get());
    if (!rvlp)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Name", rhs->name());
	nstring s(sc.subst_intl(i18n("list value required (was given $name)")));
        return rpt_value_error::create(nth_child(1)->get_pos(), s);
    }

    rpt_expr::pointer e1 = rpt_expr_constant::create(lhs);
    assert(!e1->get_pos());
    e1->pos_from(nth_child(0));
    size_t n = rvlp->size();
    for (size_t j = 0; j < n; ++j)
    {
	rpt_value::pointer vp = rvlp->nth(j);
        rpt_expr::pointer e2 = rpt_expr_constant::create(vp);
	assert(!e2->get_pos());
	e2->pos_from(nth_child(1));
        rpt_expr::pointer e3 = rpt_expr_eq::create(e1, e2);
        assert(e3->get_pos());
        rpt_value::pointer result = e3->evaluate(true, true);
	if (result->is_an_error())
            return result;

        rpt_value_boolean *rvbp =
            dynamic_cast<rpt_value_boolean *>(result.get());
	if (rvbp && rvbp->query())
            return result;
    }

    return rpt_value_boolean::create(false);
}
