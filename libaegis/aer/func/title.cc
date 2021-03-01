//
//	aegis - project change supervisor
//	Copyright (C) 1994-1996, 1999, 2002, 2004-2008 Peter Miller
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
#include <libaegis/aer/func/print.h>
#include <libaegis/aer/func/title.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/aer/value/void.h>
#include <libaegis/col.h>
#include <libaegis/sub.h>


rpt_func_title::~rpt_func_title()
{
}


rpt_func_title::rpt_func_title()
{
}


rpt_func::pointer
rpt_func_title::create()
{
    return pointer(new rpt_func_title());
}


const char *
rpt_func_title::name()
    const
{
    return "title";
}


bool
rpt_func_title::optimizable()
    const
{
    return false;
}


bool
rpt_func_title::verify(const rpt_expr::pointer &ep)
    const
{
    return (ep->get_nchildren() <= 2);
}


rpt_value::pointer
rpt_func_title::run(const rpt_expr::pointer &ep, size_t argc,
    rpt_value::pointer *argv) const
{
    trace(("title::run()\n"));
    nstring t1;
    if (argc >= 1)
    {
        trace(("argv[0] is a %s\n", argv[0]->name()));
	rpt_value::pointer vp = rpt_value::stringize(argv[0]);
        trace(("vp is a %s\n", vp->name()));
        rpt_value_string *a1sp = dynamic_cast<rpt_value_string *>(vp.get());
	if (!a1sp)
	{
	    sub_context_ty sc;
	    sc.var_set_charstar("Function", "title");
	    sc.var_set_long("Number", 1);
	    sc.var_set_charstar("Name", argv[0]->name());
	    nstring s
            (
		sc.subst_intl
		(
                    i18n("$function: argument $number: unable to print "
                        "$name value")
		)
            );
	    return rpt_value_error::create(ep->get_pos(), s);
	}
	t1 = nstring(a1sp->query());
    }

    nstring t2;
    if (argc >= 2)
    {
        trace(("argv[1] is a %s\n", argv[1]->name()));
	rpt_value::pointer vp = rpt_value::stringize(argv[1]);
        trace(("vp is a %s\n", vp->name()));
        rpt_value_string *a2sp = dynamic_cast<rpt_value_string *>(vp.get());
        if (!a2sp)
	{
	    sub_context_ty sc;
	    sc.var_set_charstar("Function", "title");
	    sc.var_set_long("Number", 2);
	    sc.var_set_charstar("Name", argv[1]->name());
	    nstring s
            (
		sc.subst_intl
		(
                    i18n("$function: argument $number: unable to print "
                        "$name value")
		)
            );
	    return rpt_value_error::create(ep->get_pos(), s);
	}
	t2 = nstring(a2sp->query());
    }

    rpt_func_print__colp->title(t1.c_str(), t2.c_str());
    return rpt_value_void::create();
}
