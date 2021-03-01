//
//	aegis - project change supervisor
//	Copyright (C) 1995, 1996, 1999, 2003-2008 Peter Miller
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

#include <common/ac/string.h>

#include <common/error.h>
#include <libaegis/aer/expr.h>
#include <libaegis/aer/func/split.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/list.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/sub.h>


rpt_func_split::~rpt_func_split()
{
}


rpt_func_split::rpt_func_split()
{
}


rpt_func::pointer
rpt_func_split::create()
{
    return pointer(new rpt_func_split());
}


const char *
rpt_func_split::name()
    const
{
    return "split";
}


bool
rpt_func_split::optimizable()
    const
{
    return true;
}


bool
rpt_func_split::verify(const rpt_expr::pointer &ep)
    const
{
    return (ep->get_nchildren() == 2);
}


rpt_value::pointer
rpt_func_split::run(const rpt_expr::pointer &ep, size_t,
    rpt_value::pointer *argv) const
{
    rpt_value::pointer a1 = argv[0];
    assert(!a1->is_an_error());
    a1 = rpt_value::stringize(a1);
    rpt_value_string *a1sp = dynamic_cast<rpt_value_string *>(a1.get());
    if (!a1sp)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Function", "split");
	sc.var_set_long("Number", 1);
	sc.var_set_charstar("Name", a1->name());
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

    rpt_value::pointer a2 = argv[1];
    assert(!a2->is_an_error());
    a2 = rpt_value::stringize(a2);
    rpt_value_string *a2sp = dynamic_cast<rpt_value_string *>(a2.get());
    if (!a2sp)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Function", "split");
	sc.var_set_long("Number", 2);
	sc.var_set_charstar("Name", a2->name());
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

    const char *sp = a1sp->query().c_str();
    const char *sep = a2sp->query().c_str();
    if (!*sep)
	sep = " \n\r\t\f\b";

    //
    // the result is a list
    // create an empty one so we can start filling it
    //
    rpt_value_list *rlp = new rpt_value_list();
    rpt_value::pointer result(rlp);
    while (*sp)
    {
	//
	// find where the line ends
	//
	const char *end_p = sp;
	while (*end_p && !strchr(sep, *end_p))
	    ++end_p;

	//
	// append the line to the result
	//
	nstring os(sp, end_p - sp);
	rpt_value::pointer tmp = rpt_value_string::create(os);
	rlp->append(tmp);

	//
	// skip the separator
	//
	sp = end_p;
	if (*sp)
	    ++sp;
    }

    //
    // clean up and go home
    //
    return result;
}
