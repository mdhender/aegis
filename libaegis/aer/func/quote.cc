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

#include <common/ac/ctype.h>
#include <common/ac/string.h>

#include <common/nstring.h>
#include <common/nstring/accumulator.h>
#include <libaegis/aer/expr.h>
#include <libaegis/aer/func/quote.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/sub.h>


rpt_func_quote_url::~rpt_func_quote_url()
{
}


rpt_func_quote_url::rpt_func_quote_url()
{
}


rpt_func::pointer
rpt_func_quote_url::create()
{
    return pointer(new rpt_func_quote_url());
}


const char *
rpt_func_quote_url::name()
    const
{
    return "quote_url";
}


bool
rpt_func_quote_url::optimizable()
    const
{
    return true;
}


bool
rpt_func_quote_url::verify(const rpt_expr::pointer &ep)
    const
{
    return (ep->get_nchildren() == 1);
}


rpt_value::pointer
rpt_func_quote_url::run(const rpt_expr::pointer &ep, size_t,
    rpt_value::pointer *argv) const
{
    rpt_value::pointer a1 = rpt_value::stringize(argv[0]);
    rpt_value_string *rvsp = dynamic_cast<rpt_value_string *>(a1.get());
    if (!rvsp)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Function", "quote_url");
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

    return rpt_value_string::create(rvsp->query().url_quote());
}


rpt_func_unquote_url::~rpt_func_unquote_url()
{
}


rpt_func_unquote_url::rpt_func_unquote_url()
{
}


rpt_func::pointer
rpt_func_unquote_url::create()
{
    return pointer(new rpt_func_unquote_url());
}


const char *
rpt_func_unquote_url::name()
    const
{
    return "unquote_url";
}


bool
rpt_func_unquote_url::optimizable()
    const
{
    return true;
}


bool
rpt_func_unquote_url::verify(const rpt_expr::pointer &ep)
    const
{
    return (ep->get_nchildren() == 1);
}


rpt_value::pointer
rpt_func_unquote_url::run(const rpt_expr::pointer &ep, size_t,
    rpt_value::pointer *argv) const
{
    rpt_value::pointer a1 = rpt_value::stringize(argv[0]);
    rpt_value_string *rvsp = dynamic_cast<rpt_value_string *>(a1.get());
    if (!rvsp)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Function", "unquote_url");
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
    nstring a1s(rvsp->query());

    nstring rs = a1s.url_unquote();
    rpt_value::pointer result = rpt_value_string::create(rs);

    //
    // clean up and go home
    //
    return result;
}


rpt_func_quote_html::~rpt_func_quote_html()
{
}


rpt_func_quote_html::rpt_func_quote_html()
{
}


rpt_func::pointer
rpt_func_quote_html::create()
{
    return pointer(new rpt_func_quote_html());
}


const char *
rpt_func_quote_html::name()
    const
{
    return "quote_html";
}


bool
rpt_func_quote_html::optimizable()
    const
{
    return true;
}


bool
rpt_func_quote_html::verify(const rpt_expr::pointer &ep)
    const
{
    return (ep->get_nchildren() == 1);
}


rpt_value::pointer
rpt_func_quote_html::run(const rpt_expr::pointer &ep, size_t,
    rpt_value::pointer *argv) const
{
    rpt_value::pointer a1 = rpt_value::stringize(argv[0]);
    rpt_value_string *rvsp = dynamic_cast<rpt_value_string *>(a1.get());
    if (!rvsp)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Function", "quote_html");
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
    nstring a1s(rvsp->query());

    nstring rs = a1s.html_quote();
    rpt_value::pointer result = rpt_value_string::create(rs);

    //
    // clean up and go home
    //
    return result;
}


rpt_func_quote_tcl::~rpt_func_quote_tcl()
{
}


rpt_func_quote_tcl::rpt_func_quote_tcl()
{
}


rpt_func::pointer
rpt_func_quote_tcl::create()
{
    return pointer(new rpt_func_quote_tcl());
}


const char *
rpt_func_quote_tcl::name()
    const
{
    return "quote_tcl";
}


bool
rpt_func_quote_tcl::optimizable()
    const
{
    return true;
}


bool
rpt_func_quote_tcl::verify(const rpt_expr::pointer &ep)
    const
{
    return (ep->get_nchildren() == 1);
}


rpt_value::pointer
rpt_func_quote_tcl::run(const rpt_expr::pointer &ep, size_t,
    rpt_value::pointer *argv) const
{
    rpt_value::pointer a1 = rpt_value::stringize(argv[0]);
    rpt_value_string *rvsp = dynamic_cast<rpt_value_string *>(a1.get());
    if (!rvsp)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Function", "quote_tcl");
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
    nstring s(rvsp->query());

    nstring_accumulator sa;
    const char *sp = s.c_str();
    for (;;)
    {
	unsigned char c = *sp++;
	if (!c)
	    break;
	// C locale
	if (!isspace(c) && !isprint(c))
	{
	    sa.push_back('\\');
	    sa.push_back('0' + ((c >> 6) & 3));
	    sa.push_back('0' + ((c >> 3) & 7));
	    sa.push_back('0' + (c & 7));
	}
	else if (strchr("$\\\"[]{}", c))
	{
	    sa.push_back('\\');
	    sa.push_back(c);
	}
	else
	    sa.push_back(c);
    }
    return rpt_value_string::create(sa.mkstr());
}
