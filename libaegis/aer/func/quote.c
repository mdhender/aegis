/*
 *	aegis - project change supervisor
 *	Copyright (C) 1998, 1999, 2001, 2002 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate quotes
 */

#include <ac/ctype.h>
#include <ac/string.h>

#include <aer/expr.h>
#include <aer/func/quote.h>
#include <aer/value/error.h>
#include <aer/value/string.h>
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
    rpt_value_ty    *a1;
    rpt_value_ty    *result;
    string_ty	    *s;
    char	    *sp;
    stracc_t	    sa;

    stracc_constructor(&sa);
    a1 = rpt_value_stringize(argv[0]);
    if (a1->method->type != rpt_value_type_string)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	rpt_value_free(a1);
	sub_var_set_charstar(scp, "Function", "quote_url");
	sub_var_set_charstar(scp, "Number", "1");
	sub_var_set_charstar(scp, "Name", argv[0]->method->name);
	s =
	    subst_intl
	    (
		scp,
    i18n("$function: argument $number: string value required (was given $name)")
	    );
	sub_context_delete(scp);
	result = rpt_value_error(ep->pos, s);
	str_free(s);
	return result;
    }
    s = str_copy(rpt_value_string_query(a1));
    rpt_value_free(a1);

    stracc_open(&sa);
    sp = s->str_text;
    for (;;)
    {
	unsigned char	c;

	c = *sp++;
	if (!c)
	    break;
	/* C locale */
	if (!isprint(c) || strchr(" \"#%&'+:=?~", c))
	{
	    /*
	     *	    #:?	    URL special characters
	     *	    %	    escape the escape character
	     *	    ~	    illegal in portable character sets
	     *	    &=	    confuse forms-based browsers
	     *	    "'	    I've included some shell meta
	     *		    characters here, too, because
	     *		    these are usually passed through
	     *		    shell scripts.
	     */
	    static char	    hex[] =	    "0123456789ABCDEF";

	    stracc_char(&sa, '%');
	    stracc_char(&sa, hex[(c >> 4) & 15]);
	    stracc_char(&sa, hex[c & 15]);
	}
	else
	    stracc_char(&sa, c);
    }
    str_free(s);
    s = stracc_close(&sa);
    result = rpt_value_string(s);
    str_free(s);

    /*
     * clean up and go home
     */
    stracc_destructor(&sa);
    return result;
}


rpt_func_ty rpt_func_quote_url =
{
    "quote_url",
    1, /* optimizable */
    quote_url_verify,
    quote_url_run
};


static int
unquote_url_verify(rpt_expr_ty *ep)
{
    return (ep->nchild == 1);
}


static int
unhex(int c)
{
    switch (c)
    {
    default:
	return -1;

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
	return (c - '0');

    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
	return (c - 'a' + 10);

    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
	return (c - 'A' + 10);
    }
}


static rpt_value_ty *
unquote_url_run(rpt_expr_ty *ep, size_t argc, rpt_value_ty **argv)
{
    rpt_value_ty    *a1;
    rpt_value_ty    *result;
    string_ty	    *s;
    char	    *sp;
    int		    c;
    int		    n1;
    int		    n2;
    stracc_t	    sa;

    stracc_constructor(&sa);
    a1 = rpt_value_stringize(argv[0]);
    if (a1->method->type != rpt_value_type_string)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	rpt_value_free(a1);
	sub_var_set_charstar(scp, "Function", "unquote_url");
	sub_var_set_charstar(scp, "Number", "1");
	sub_var_set_charstar(scp, "Name", argv[0]->method->name);
	s =
	    subst_intl
	    (
		scp,
    i18n("$function: argument $number: string value required (was given $name)")
	    );
	sub_context_delete(scp);
	result = rpt_value_error(ep->pos, s);
	str_free(s);
	return result;
    }
    s = str_copy(rpt_value_string_query(a1));
    rpt_value_free(a1);

    stracc_open(&sa);
    sp = s->str_text;
    for (;;)
    {
	c = (unsigned char)*sp++;
	if (!c)
	    break;
	if (c == '%' && (n1 = unhex(sp[0])) >= 0 && (n2 = unhex(sp[1])) >= 0)
	{
	    c = (n1 << 4) + n2;
	    sp += 2;
	}
	stracc_char(&sa, c);
    }
    str_free(s);
    s = stracc_close(&sa);
    result = rpt_value_string(s);
    str_free(s);

    /*
     * clean up and go home
     */
    stracc_destructor(&sa);
    return result;
}


rpt_func_ty rpt_func_unquote_url =
{
    "unquote_url",
    1, /* optimizable */
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
    rpt_value_ty    *a1;
    rpt_value_ty    *result;
    string_ty	    *s;
    char	    *sp;
    stracc_t	    sa;

    stracc_constructor(&sa);
    a1 = rpt_value_stringize(argv[0]);
    if (a1->method->type != rpt_value_type_string)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	rpt_value_free(a1);
	sub_var_set_charstar(scp, "Function", "quote_html");
	sub_var_set_charstar(scp, "Number", "1");
	sub_var_set_charstar(scp, "Name", argv[0]->method->name);
	s =
	    subst_intl
	    (
		scp,
    i18n("$function: argument $number: string value required (was given $name)")
	    );
	sub_context_delete(scp);
	result = rpt_value_error(ep->pos, s);
	str_free(s);
	return result;
    }
    s = str_copy(rpt_value_string_query(a1));
    rpt_value_free(a1);

    stracc_open(&sa);
    sp = s->str_text;
    for (;;)
    {
	unsigned char	c;

	c = *sp++;
	if (!c)
	    break;
	if (c == '<')
	    stracc_chars(&sa, "&lt;", 4);
	else if (c == '&')
	    stracc_chars(&sa, "&amp;", 5);
	else if (isspace(c) || isprint(c))
	    stracc_char(&sa, c);
	else
	{
	    /*
	     * The above isprint() is in the C locale.
	     * This is probably not correct.
	     *
	     * To get it exactly right, we need to know
	     * the User's locale, and map the characters
	     * according to the character set used by the
	     * current locale.	This information isn't
	     * readily available.
	     *
	     * Decimal!	 Why didn't these guys just use the
	     * one escape mechanism twice.  Sheesh.
	     */
	    stracc_char(&sa, '&');
	    stracc_char(&sa, '#');
	    if (c >= 100)
		stracc_char(&sa, '0' + (c / 100) % 10);
	    if (c >= 10)
		stracc_char(&sa, '0' + (c / 10) % 10);
	    stracc_char(&sa, '0' + c % 10);
	    stracc_char(&sa, ';');
	}
    }
    str_free(s);
    s = stracc_close(&sa);
    result = rpt_value_string(s);
    str_free(s);

    /*
     * clean up and go home
     */
    stracc_destructor(&sa);
    return result;
}


rpt_func_ty rpt_func_quote_html =
{
    "quote_html",
    1, /* optimizable */
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
    rpt_value_ty    *a1;
    rpt_value_ty    *result;
    string_ty	    *s;
    char	    *sp;
    stracc_t	    sa;

    stracc_constructor(&sa);
    a1 = rpt_value_stringize(argv[0]);
    if (a1->method->type != rpt_value_type_string)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	rpt_value_free(a1);
	sub_var_set_charstar(scp, "Function", "quote_tcl");
	sub_var_set_charstar(scp, "Number", "1");
	sub_var_set_charstar(scp, "Name", argv[0]->method->name);
	s =
	    subst_intl
	    (
		scp,
    i18n("$function: argument $number: string value required (was given $name)")
	    );
	sub_context_delete(scp);
	result = rpt_value_error(ep->pos, s);
	str_free(s);
	return result;
    }
    s = str_copy(rpt_value_string_query(a1));
    rpt_value_free(a1);

    stracc_open(&sa);
    sp = s->str_text;
    for (;;)
    {
	unsigned char	c;

	c = *sp++;
	if (!c)
	    break;
	/* C locale */
	if (!isspace(c) && !isprint(c))
	{
	    stracc_char(&sa, '\\');
	    stracc_char(&sa, '0' + ((c >> 6) & 3));
	    stracc_char(&sa, '0' + ((c >> 3) & 7));
	    stracc_char(&sa, '0' + (c & 7));
	}
	else if (strchr("$\\\"[]{}", c))
	{
	    stracc_char(&sa, '\\');
	    stracc_char(&sa, c);
	}
	else
	    stracc_char(&sa, c);
    }
    str_free(s);
    s = stracc_close(&sa);
    result = rpt_value_string(s);
    str_free(s);

    /*
     * clean up and go home
     */
    stracc_destructor(&sa);
    return result;
}


rpt_func_ty rpt_func_quote_tcl =
{
    "quote_tcl",
    1, /* optimizable */
    quote_tcl_verify,
    quote_tcl_run
};
