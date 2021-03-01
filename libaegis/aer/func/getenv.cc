//
//	aegis - project change supervisor
//	Copyright (C) 1997, 1999, 2003-2008 Peter Miller
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

#include <common/ac/stdlib.h>

#include <common/error.h>
#include <libaegis/aer/expr.h>
#include <libaegis/aer/func/getenv.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/aer/value/time.h>
#include <libaegis/sub.h>


rpt_func_getenv::~rpt_func_getenv()
{
}


rpt_func_getenv::rpt_func_getenv()
{
}


rpt_func::pointer
rpt_func_getenv::create()
{
    return pointer(new rpt_func_getenv());
}


const char *
rpt_func_getenv::name()
    const
{
    return "getenv";
}


bool
rpt_func_getenv::optimizable()
    const
{
    return true;
}


bool
rpt_func_getenv::verify(const rpt_expr::pointer &ep)
    const
{
    return (ep->get_nchildren() == 1);
}


rpt_value::pointer
rpt_func_getenv::run(const rpt_expr::pointer &ep, size_t,
    rpt_value::pointer *argv) const
{
    //
    // Coerce the argument to a string.
    // It is an error if it can't be.
    //
    rpt_value::pointer tmp = rpt_value::stringize(argv[0]);
    rpt_value_string *rvsp = dynamic_cast<rpt_value_string *>(tmp.get());
    if (!rvsp)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Function", "getenv");
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

    //
    // Scan the string and try to retrieve its content as an envar.
    // It is not an error if this can't be done.
    //
    nstring s(rvsp->query());
    const char *cp = getenv(s.c_str());
    if (!cp)
	cp = "";

    //
    // build the return value
    //
    return rpt_value_string::create(cp);
}
