//
//	aegis - project change supervisor
//	Copyright (C) 2002-2008 Peter Miller
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
#include <common/mem.h>
#include <libaegis/aer/expr.h>
#include <libaegis/aer/func/upcase.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/sub.h>


rpt_func_upcase::~rpt_func_upcase()
{
}


rpt_func_upcase::rpt_func_upcase()
{
}


rpt_func::pointer
rpt_func_upcase::create()
{
    return pointer(new rpt_func_upcase());
}


const char *
rpt_func_upcase::name()
    const
{
    return "upcase";
}


bool
rpt_func_upcase::optimizable()
    const
{
    return true;
}


bool
rpt_func_upcase::verify(const rpt_expr::pointer &ep)
    const
{
    return (ep->get_nchildren() == 1);
}


rpt_value::pointer
rpt_func_upcase::run(const rpt_expr::pointer &ep, size_t,
    rpt_value::pointer *argv) const
{
    rpt_value::pointer arg = argv[0];
    assert(!arg->is_an_error());
    arg = rpt_value::stringize(arg);
    rpt_value_string *a1sp = dynamic_cast<rpt_value_string *>(arg.get());
    if (!a1sp)
    {
        sub_context_ty sc;
        sc.var_set_charstar("Function", "upcase");
        sc.var_set_long("Number", 1);
        sc.var_set_charstar("Name", argv[0]->name());
        nstring s
        (
            sc.subst_intl
            (
                i18n("$function: argument $number: string value required "
                    "(was given $name)")
            )
        );
        return rpt_value_error::create(ep->get_pos(), s);
    }
    nstring subject(a1sp->query());

    //
    // build the result
    //
    nstring s = subject.upcase();
    return rpt_value_string::create(s);
}
