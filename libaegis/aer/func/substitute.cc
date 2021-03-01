//
//	aegis - project change supervisor
//	Copyright (C) 1998, 1999, 2001, 2002, 2004-2008 Peter Miller
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
#include <common/regula_expre.h>
#include <common/trace.h>
#include <libaegis/aer/expr.h>
#include <libaegis/aer/func/substitute.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/sub.h>


rpt_func_substitute::~rpt_func_substitute()
{
}


rpt_func_substitute::rpt_func_substitute()
{
}


rpt_func::pointer
rpt_func_substitute::create()
{
    return pointer(new rpt_func_substitute());
}


const char *
rpt_func_substitute::name()
    const
{
    return "subst";
}


bool
rpt_func_substitute::optimizable()
    const
{
    return true;
}


bool
rpt_func_substitute::verify(const rpt_expr::pointer &ep)
    const
{
    return (ep->get_nchildren() == 3 || ep->get_nchildren() == 4);
}


rpt_value::pointer
rpt_func_substitute::run(const rpt_expr::pointer &ep, size_t argc,
    rpt_value::pointer *argv) const
{
    trace(("rpt_func_substitute::run()\n"));
    //
    // Get the match pattern.
    //
    rpt_value::pointer arg = argv[0];
    assert(!arg->is_an_error());
    arg = rpt_value::stringize(arg);
    rpt_value_string *s0p = dynamic_cast<rpt_value_string *>(arg.get());
    if (!s0p)
    {
	sub_context_ty sc(__FILE__, __LINE__);
	sc.var_set_charstar("Function", "subst");
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
    nstring lhs(s0p->query());

    //
    // Get the replacement pattern.
    //
    arg = argv[1];
    assert(!arg->is_an_error());
    arg = rpt_value::stringize(arg);
    rpt_value_string *s1p = dynamic_cast<rpt_value_string *>(arg.get());
    if (!s1p)
    {
	sub_context_ty sc(__FILE__, __LINE__);
	sc.var_set_charstar("Function", "subst");
	sc.var_set_long("Number", 2);
	sc.var_set_charstar("Name", argv[1]->name());
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
    nstring rhs(s1p->query());

    //
    // Get the string to be worked over.
    //
    arg = argv[2];
    assert(!arg->is_an_error());
    arg = rpt_value::stringize(arg);
    rpt_value_string *s2p = dynamic_cast<rpt_value_string *>(arg.get());
    if (!s2p)
    {
	sub_context_ty sc(__FILE__, __LINE__);
	sc.var_set_charstar("Function", "subst");
	sc.var_set_long("Number", 3);
	sc.var_set_charstar("Name", argv[2]->name());
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
    nstring query_input(s2p->query());

    //
    // Get the count of how many times to match.
    //
    long maximum_matches = 0;
    if (argc >= 4)
    {
	arg = argv[3];
	assert(!arg->is_an_error());
	arg = rpt_value::integerize(arg);
        rpt_value_integer *rip = dynamic_cast<rpt_value_integer *>(arg.get());
	if (!rip)
	{
	    sub_context_ty sc(__FILE__, __LINE__);
	    sc.var_set_charstar("Function", "subst");
	    sc.var_set_long("Number", 2);
	    sc.var_set_charstar("Name", argv[3]->name());
	    nstring s
            (
		sc.subst_intl
		(
                    i18n("$function: argument $number: integer value "
                        "required (was given $name)")
		)
            );
	    return rpt_value_error::create(ep->get_pos(), s);
	}
	maximum_matches = rip->query();
    }

    //
    // perform the substitution
    //
    regular_expression re(lhs);
    nstring result;
    if (!re.match_and_substitute(rhs, query_input, maximum_matches, result))
    {
	//
        // Error... probably the LHS pattern was erroneous, but it could
        // have been the RHS pattern.
	//
	// Return an error result.
	//
	sub_context_ty sc(__FILE__, __LINE__);
	sc.var_set_charstar("Function", "subst");
	sc.var_set_long("Number", 1);
	sc.var_set_charstar("MeSsaGe", re.strerror());
	nstring s(sc.subst_intl(i18n("$function: argument $number: $message")));
	return rpt_value_error::create(ep->get_pos(), s);
    }

    //
    // build the result
    //
    trace_nstring(result);
    return rpt_value_string::create(result);
}
