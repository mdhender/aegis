//
//	aegis - project change supervisor
//	Copyright (C) 2007 Peter Miller
//
//      This program is free software; you can redistribute it and/or
//      modify it under the terms of the GNU General Public License as
//      published by the Free Software Foundation; either version 2 of
//      the License, or (at your option) any later version.
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

#include <libaegis/aer/expr/comma.h>
#include <libaegis/aer/value.h>


rpt_expr_comma::~rpt_expr_comma()
{
}


rpt_expr_comma::rpt_expr_comma(const pointer &lhs, const pointer &rhs)
{
    append(lhs);
    append(rhs);
}


rpt_expr::pointer
rpt_expr_comma::create(const pointer &lhs, const pointer &rhs)
{
    return pointer(new rpt_expr_comma(lhs, rhs));
}


rpt_value::pointer
rpt_expr_comma::evaluate()
    const
{
    rpt_value::pointer result = nth_child(0)->evaluate(true, true);
    if (result->is_an_error())
        return result;

    return nth_child(1)->evaluate(true, true);
}
