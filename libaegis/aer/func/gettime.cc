//
//	aegis - project change supervisor
//	Copyright (C) 1997, 1999, 2004-2008 Peter Miller
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
#include <common/gettime.h>
#include <libaegis/aer/expr.h>
#include <libaegis/aer/func/gettime.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/aer/value/time.h>
#include <libaegis/sub.h>


rpt_func_gettime::~rpt_func_gettime()
{
}


rpt_func_gettime::rpt_func_gettime()
{
}


rpt_func::pointer
rpt_func_gettime::create()
{
    return pointer(new rpt_func_gettime());
}


const char *
rpt_func_gettime::name()
    const
{
    return "gettime";
}


bool
rpt_func_gettime::optimizable()
    const
{
    return true;
}


bool
rpt_func_gettime::verify(const rpt_expr::pointer &ep)
    const
{
    return (ep->get_nchildren() == 1);
}


rpt_value::pointer
rpt_func_gettime::run(const rpt_expr::pointer &ep, size_t,
    rpt_value::pointer *argv) const
{
    //
    // See if it looks like a number.
    // Use that if so.
    //
    rpt_value::pointer tmp = rpt_value::integerize(argv[0]);
    rpt_value_integer *rvip = dynamic_cast<rpt_value_integer *>(tmp.get());
    if (rvip)
    {
	time_t t = rvip->query();
	return rpt_value_time::create(t);
    }

    //
    // Coerce the argument to a string.
    // It is an error if it can't be.
    //
    tmp = rpt_value::stringize(argv[0]);
    rpt_value_string *rvsp = dynamic_cast<rpt_value_string *>(tmp.get());
    if (!rvsp)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Function", "gettime");
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
    // Scan the string and try to make a time out of it.
    // It is an error if this can't be done.
    //
    nstring s(rvsp->query());
    time_t t = date_scan(s.c_str());
    if (t == (time_t)-1)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Function", "gettime");
	sc.var_set_long("Number", 1);
	nstring es
        (
	    sc.subst_intl
	    (
                i18n("$function: argument $number: cannot convert string "
                    "into a time")
	    )
        );
	return rpt_value_error::create(ep->get_pos(), es);
    }

    //
    // build the return value
    //
    return rpt_value_time::create(t);
}


rpt_func_mktime::~rpt_func_mktime()
{
}


rpt_func_mktime::rpt_func_mktime()
{
}


rpt_func::pointer
rpt_func_mktime::create()
{
    return pointer(new rpt_func_mktime());
}


const char *
rpt_func_mktime::name()
    const
{
    return "mktime";
}
