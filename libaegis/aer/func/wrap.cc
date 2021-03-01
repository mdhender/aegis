//
//	aegis - project change supervisor
//	Copyright (C) 1995, 1996, 1998, 1999, 2002-2008 Peter Miller
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

#include <common/ac/ctype.h>

#include <common/stracc.h>
#include <common/trace.h>
#include <libaegis/aer/expr.h>
#include <libaegis/aer/func/wrap.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/list.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/sub.h>


rpt_func_wrap::~rpt_func_wrap()
{
}


rpt_func_wrap::rpt_func_wrap()
{
}


rpt_func::pointer
rpt_func_wrap::create()
{
    return pointer(new rpt_func_wrap());
}


const char *
rpt_func_wrap::name()
    const
{
    return "wrap";
}


bool
rpt_func_wrap::optimizable()
    const
{
    return true;
}


bool
rpt_func_wrap::verify(const rpt_expr::pointer &ep)
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return (ep->get_nchildren() == 2);
}


rpt_value::pointer
rpt_func_wrap::run(const rpt_expr::pointer &ep, size_t,
    rpt_value::pointer *argv) const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    rpt_value::pointer a1 = rpt_value::stringize(argv[0]);
    rpt_value_string *a1sp = dynamic_cast<rpt_value_string *>(a1.get());
    if (!a1sp)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Function", "wrap");
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
    subject = subject.trim_lines();

    rpt_value::pointer a2 = rpt_value::integerize(argv[1]);
    rpt_value_integer *a2ip = dynamic_cast<rpt_value_integer *>(a2.get());
    if (!a2ip)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Function", "wrap");
	sc.var_set_long("Number", 2);
	sc.var_set_charstar("Name", argv[1]->name());
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
    long width = a2ip->query();
    if (width < 1)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Function", "wrap");
	sc.var_set_long("Number", 2);
	sc.var_set_long("Value", width);
	nstring s
        (
	    sc.subst_intl
	    (
		i18n("$function: argument $number: width $value out of range")
	    )
        );
	return rpt_value_error::create(ep->get_pos(), s);
    }

    //
    // the result is a list
    // create an empty one se we can start filling it
    //
    rpt_value_list *p = new rpt_value_list();
    rpt_value::pointer result(p);

    const char *sp = subject.c_str();
    while (*sp)
    {
	//
	// find where the line ends
	//
	const char *end_p = sp;
	while (end_p - sp < width && *end_p && *end_p != '\n')
	    ++end_p;
	if (*end_p && *end_p != '\n')
	{
	    //
	    // see if there is a better place to wrap
	    //
	    const char *w = end_p;
	    while (w > sp && !isspace((unsigned char)(w[-1])))
		--w;
	    if (w > sp + 1)
		end_p = w - 1;
	    else
	    {
		w = end_p;
		while (w > sp && !ispunct((unsigned char)(w[-1])))
	    	    --w;
		if (w > sp + 1)
	    	    end_p = w - 1;
	    }
	}

	//
	// append the line to the result
	//
	nstring os(sp, end_p - sp);
	rpt_value::pointer tmp = rpt_value_string::create(os);
	p->append(tmp);

	//
	// skip line terminator and spaces
	//
	sp = end_p;
	if (*sp == '\n')
	    ++sp;
	while (isspace((unsigned char)*sp) && *sp != '\n')
	    ++sp;
    }

    return result;
}


rpt_func_wrap_html::~rpt_func_wrap_html()
{
}


rpt_func_wrap_html::rpt_func_wrap_html()
{
}


rpt_func::pointer
rpt_func_wrap_html::create()
{
    return pointer(new rpt_func_wrap_html());
}


const char *
rpt_func_wrap_html::name()
    const
{
    return "wrap_html";
}


bool
rpt_func_wrap_html::optimizable()
    const
{
    return true;
}


bool
rpt_func_wrap_html::verify(const rpt_expr::pointer &ep)
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return (ep->get_nchildren() == 2);
}


rpt_value::pointer
rpt_func_wrap_html::run(const rpt_expr::pointer &ep, size_t,
    rpt_value::pointer *argv) const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    rpt_value::pointer a1 = rpt_value::stringize(argv[0]);
    rpt_value_string *a1sp = dynamic_cast<rpt_value_string *>(a1.get());
    if (!a1sp)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Function", "wrap");
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
    subject = subject.trim_lines();

    rpt_value::pointer a2 = rpt_value::integerize(argv[1]);
    rpt_value_integer *a2ip = dynamic_cast<rpt_value_integer *>(a2.get());
    if (!a2ip)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Function", "wrap");
	sc.var_set_long("Number", 2);
	sc.var_set_charstar("Name", argv[1]->name());
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
    long width = a2ip->query();

    if (width < 1)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Function", "wrap");
	sc.var_set_long("Number", 2);
	sc.var_set_long("Value", width);
	nstring s
        (
	    sc.subst_intl
	    (
	        i18n("$function: argument $number: width $value out of range")
	    )
        );
	return rpt_value_error::create(ep->get_pos(), s);
    }

    //
    // the result is a list
    // create an empty one se we can start filling it
    //
    rpt_value_list *p = new rpt_value_list();
    rpt_value::pointer result(p);

    const char *sp = subject.c_str();
    while (*sp)
    {
	//
	// find where the line ends
	//
	const char *end_p = sp;
	while (end_p - sp < width && *end_p && *end_p != '\n')
	    ++end_p;
	if (*end_p && *end_p != '\n')
	{
	    //
	    // see if there is a better place to wrap
	    //
	    const char *w = end_p;
	    while (w > sp && !isspace((unsigned char)(w[-1])))
		--w;
	    if (w > sp + 1)
		end_p = w - 1;
	    else
	    {
		w = end_p;
		while (w > sp && !ispunct((unsigned char)(w[-1])))
	    	    --w;
		if (w > sp + 1)
	    	    end_p = w - 1;
	    }
	}

	//
	// append the line to the result
	//
	nstring os(sp, end_p - sp);
	rpt_value::pointer tmp = rpt_value_string::create(os);
	p->append(tmp);

	//
	// skip line terminator and spaces
	//
	// Insert HTML paragraph breaks or line breaks
	// if appropriate.
	//
	sp = end_p;
	while (*sp && *sp != '\n' && isspace((unsigned char)*sp))
	    ++sp;
	if (*sp == '\n')
	{
	    ++sp;
	    if (*sp && isspace((unsigned char)*sp))
	    {
		os = nstring("<p>");
		tmp = rpt_value_string::create(os);
		p->append(tmp);
	    }
	    else
	    {
		os = nstring("<br>");
		tmp = rpt_value_string::create(os);
		p->append(tmp);
	    }
	}
	while (*sp && isspace((unsigned char)*sp))
	    ++sp;
    }

    return result;
}
