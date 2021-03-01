//
//	aegis - project change supervisor
//	Copyright (C) 1998, 1999, 2001, 2002, 2004, 2005 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate quotes
//

#include <ac/ctype.h>
#include <ac/string.h>

#include <aer/expr.h>
#include <aer/func/quote.h>
#include <aer/value/error.h>
#include <aer/value/string.h>
#include <nstring.h>
#include <stracc.h>
#include <sub.h>


static int
quote_url_verify(rpt_expr_ty *ep)
{
    return (ep->nchild == 1);
}


static rpt_value_ty *
quote_url_run(rpt_expr_ty *ep, size_t argc, rpt_value_ty **argv)
{
    rpt_value_ty *a1 = rpt_value_stringize(argv[0]);
    if (a1->method->type != rpt_value_type_string)
    {
	sub_context_ty *scp = sub_context_new();
	rpt_value_free(a1);
	sub_var_set_charstar(scp, "Function", "quote_url");
	sub_var_set_long(scp, "Number", 1);
	sub_var_set_charstar(scp, "Name", argv[0]->method->name);
	string_ty *s =
	    subst_intl
	    (
		scp,
    i18n("$function: argument $number: string value required (was given $name)")
	    );
	sub_context_delete(scp);
	rpt_value_ty *result = rpt_value_error(ep->pos, s);
	str_free(s);
	return result;
    }
    nstring a1s(rpt_value_string_query(a1));
    rpt_value_free(a1);

    nstring rs = a1s.url_quote();
    rpt_value_ty *result = rpt_value_string(rs.get_ref());

    //
    // clean up and go home
    //
    return result;
}


rpt_func_ty rpt_func_quote_url =
{
    "quote_url",
    1, // optimizable
    quote_url_verify,
    quote_url_run
};


static int
unquote_url_verify(rpt_expr_ty *ep)
{
    return (ep->nchild == 1);
}


static rpt_value_ty *
unquote_url_run(rpt_expr_ty *ep, size_t argc, rpt_value_ty **argv)
{
    rpt_value_ty *a1 = rpt_value_stringize(argv[0]);
    if (a1->method->type != rpt_value_type_string)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	rpt_value_free(a1);
	sub_var_set_charstar(scp, "Function", "unquote_url");
	sub_var_set_long(scp, "Number", 1);
	sub_var_set_charstar(scp, "Name", argv[0]->method->name);
	string_ty *s =
	    subst_intl
	    (
		scp,
    i18n("$function: argument $number: string value required (was given $name)")
	    );
	sub_context_delete(scp);
	rpt_value_ty *result = rpt_value_error(ep->pos, s);
	str_free(s);
	return result;
    }
    nstring a1s(rpt_value_string_query(a1));
    rpt_value_free(a1);

    nstring rs = a1s.url_unquote();
    rpt_value_ty *result = rpt_value_string(rs.get_ref());

    //
    // clean up and go home
    //
    return result;
}


rpt_func_ty rpt_func_unquote_url =
{
    "unquote_url",
    1, // optimizable
    unquote_url_verify,
    unquote_url_run
};


static int
quote_html_verify(rpt_expr_ty *ep)
{
    return (ep->nchild == 1);
}


static rpt_value_ty *
quote_html_run(rpt_expr_ty *ep, size_t argc, rpt_value_ty **argv)
{
    rpt_value_ty *a1 = rpt_value_stringize(argv[0]);
    if (a1->method->type != rpt_value_type_string)
    {
	sub_context_ty *scp = sub_context_new();
	rpt_value_free(a1);
	sub_var_set_charstar(scp, "Function", "quote_html");
	sub_var_set_long(scp, "Number", 1);
	sub_var_set_charstar(scp, "Name", argv[0]->method->name);
	string_ty *s =
	    subst_intl
	    (
		scp,
    i18n("$function: argument $number: string value required (was given $name)")
	    );
	sub_context_delete(scp);
	rpt_value_ty *result = rpt_value_error(ep->pos, s);
	str_free(s);
	return result;
    }
    nstring a1s(rpt_value_string_query(a1));
    rpt_value_free(a1);

    nstring rs = a1s.html_quote();
    rpt_value_ty *result = rpt_value_string(rs.get_ref());

    //
    // clean up and go home
    //
    return result;
}


rpt_func_ty rpt_func_quote_html =
{
    "quote_html",
    1, // optimizable
    quote_html_verify,
    quote_html_run
};


static int
quote_tcl_verify(rpt_expr_ty *ep)
{
    return (ep->nchild == 1);
}


static rpt_value_ty *
quote_tcl_run(rpt_expr_ty *ep, size_t argc, rpt_value_ty **argv)
{
    rpt_value_ty *a1 = rpt_value_stringize(argv[0]);
    if (a1->method->type != rpt_value_type_string)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	rpt_value_free(a1);
	sub_var_set_charstar(scp, "Function", "quote_tcl");
	sub_var_set_long(scp, "Number", 1);
	sub_var_set_charstar(scp, "Name", argv[0]->method->name);
	string_ty *s =
	    subst_intl
	    (
		scp,
    i18n("$function: argument $number: string value required (was given $name)")
	    );
	sub_context_delete(scp);
	rpt_value_ty *result = rpt_value_error(ep->pos, s);
	str_free(s);
	return result;
    }
    string_ty *s = str_copy(rpt_value_string_query(a1));
    rpt_value_free(a1);

    stracc_t sa;
    const char *sp = s->str_text;
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
    str_free(s);
    s = sa.mkstr();
    rpt_value_ty *result = rpt_value_string(s);
    str_free(s);

    //
    // clean up and go home
    //
    return result;
}


rpt_func_ty rpt_func_quote_tcl =
{
    "quote_tcl",
    1, // optimizable
    quote_tcl_verify,
    quote_tcl_run
};
