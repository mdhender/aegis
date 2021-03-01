//
//      aegis - project change supervisor
//      Copyright (C) 1994, 2002-2008 Peter Miller.
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
#include <libaegis/aer/expr/constant.h>
#include <libaegis/aer/value/ref.h>


rpt_expr_constant::~rpt_expr_constant()
{
}


rpt_expr_constant::rpt_expr_constant(const rpt_value::pointer &a_value) :
    value(a_value)
{
    trace(("rpt_expr_constant::rpt_expr_constant(this = %08lX, "
        "a_value = %08lX)\n", (long)this, (long)a_value.get()));
}


rpt_expr::pointer
rpt_expr_constant::create(const rpt_value::pointer &a_value)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer(new rpt_expr_constant(a_value));
}


rpt_value::pointer
rpt_expr_constant::evaluate()
    const
{
    trace(("rpt_expr_constant::evaluate(this = %08lX)\n{\n", (long)this));
    trace(("return %08lX;\n", (long)value.get()));
    trace(("}\n"));
    return value;
}


bool
rpt_expr_constant::lvalue()
    const
{
    trace(("rpt_expr_constant::lvalue(this = %08lX)\n", (long)this));
    bool is_ref = !!dynamic_cast<const rpt_value_reference *>(value.get());
    trace(("return %s\n", is_ref ? "true" : "false"));
    return is_ref;
}
