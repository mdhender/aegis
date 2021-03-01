//
//	aegis - project change supervisor
//	Copyright (C) 1994, 1996, 2002-2008 Peter Miller
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
#include <libaegis/aer/expr/list.h>
#include <libaegis/aer/value/list.h>


rpt_expr_list::~rpt_expr_list()
{
}


rpt_expr_list::rpt_expr_list()
{
}


rpt_expr::pointer
rpt_expr_list::create()
{
    return pointer(new rpt_expr_list());
}


rpt_value::pointer
rpt_expr_list::evaluate()
    const
{
    rpt_value_list *p = new rpt_value_list();
    rpt_value::pointer result((rpt_value *)p);
    for (size_t j = 0; ; ++j)
    {
        rpt_expr::pointer ep = nth_child(j);
        if (!ep)
            break;
        rpt_value::pointer vp = ep->evaluate(false, true);
	if (vp->is_an_error())
	{
	    return vp;
	}
	p->append(vp);
    }
    return result;
}
