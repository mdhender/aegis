/*
 *	aegis - project change supervisor
 *	Copyright (C) 1995, 1996, 1998, 1999, 2002, 2003 Peter Miller;
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
 * MANIFEST: functions to implement the builtin wrap function
 */

#include <ac/ctype.h>

#include <aer/expr.h>
#include <aer/func/wrap.h>
#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <aer/value/list.h>
#include <aer/value/string.h>
#include <mem.h>
#include <sub.h>


static int
wrap_verify(rpt_expr_ty *ep)
{
    return (ep->nchild == 2);
}


static string_ty *
trim(string_ty *s)
{
    static size_t   buflen;
    static char	    *buf;
    char	    *bp;
    char	    *sp;

    if (buflen < s->str_length)
    {
	buflen = s->str_length;
	buf = (char *)mem_change_size(buf, buflen);
    }
    bp = buf;
    sp = s->str_text;
    while (*sp && *sp != '\n' && isspace((unsigned char)*sp))
	++sp;
    while (*sp)
    {
	if (!*sp)
	    break;
	if (*sp == '\n')
	{
	    *bp++ = *sp++;
	    while (*sp && *sp != '\n' && isspace((unsigned char)*sp))
	       	++sp;
	}
	else if (isspace((unsigned char)*sp))
	{
	    *bp++ = ' ';
	    do
	       	++sp;
	    while
	       	(*sp && *sp != '\n' && isspace((unsigned char)*sp));
	    if (!*sp || *sp == '\n')
	       	--bp;
	}
	else
	    *bp++ = *sp++;
    }
    return str_n_from_c(buf, bp - buf);
}


static rpt_value_ty *
wrap_run(rpt_expr_ty *ep, size_t argc, rpt_value_ty **argv)
{
    rpt_value_ty    *a1;
    rpt_value_ty    *a2;
    rpt_value_ty    *result;
    rpt_value_ty    *tmp;
    string_ty	    *subject;
    string_ty	    *s;
    long	    width;
    char	    *sp;

    a1 = rpt_value_stringize(argv[0]);
    if (a1->method->type != rpt_value_type_string)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	rpt_value_free(a1);
	sub_var_set_charstar(scp, "Function", "wrap");
	sub_var_set_charstar(scp, "Number", "1");
	sub_var_set_charstar(scp, "Name", argv[0]->method->name);
	s =
	    subst_intl
	    (
	       	scp,
    i18n("$function: argument $number: string value required (was given $name)")
	    );
	sub_context_delete(scp);
	tmp = rpt_value_error(ep->pos, s);
	str_free(s);
	return tmp;
    }
    subject = trim(rpt_value_string_query(a1));
    rpt_value_free(a1);

    a2 = rpt_value_integerize(argv[1]);
    if (a2->method->type != rpt_value_type_integer)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	rpt_value_free(a2);
	sub_var_set_charstar(scp, "Function", "wrap");
	sub_var_set_charstar(scp, "Number", "2");
	sub_var_set_charstar(scp, "Name", argv[1]->method->name);
	s =
	    subst_intl
	    (
	       	scp,
   i18n("$function: argument $number: integer value required (was given $name)")
	    );
	sub_context_delete(scp);
	tmp = rpt_value_error(ep->pos, s);
	str_free(s);
	return tmp;
    }
    width = rpt_value_integer_query(a2);
    rpt_value_free(a2);
    if (width < 1)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Function", "wrap");
	sub_var_set_charstar(scp, "Number", "2");
	sub_var_set_long(scp, "Value", width);
	s =
	    subst_intl
	    (
	       	scp,
		i18n("$function: argument $number: width $value out of range")
	    );
	sub_context_delete(scp);
	tmp = rpt_value_error(ep->pos, s);
	str_free(s);
	return tmp;
    }

    /*
     * the result is a list
     * create an empty one se we can start filling it
     */
    result = rpt_value_list();

    sp = subject->str_text;
    while (*sp)
    {
	char		*end_p;
	string_ty	*os;

	/*
	 * find where the line ends
	 */
	end_p = sp;
	while (end_p - sp < width && *end_p && *end_p != '\n')
	    ++end_p;
	if (*end_p && *end_p != '\n')
	{
	    char	    *w;

	    /*
	     * see if there is a better place to wrap
	     */
	    w = end_p;
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

	/*
	 * append the line to the result
	 */
	os = str_n_from_c(sp, end_p - sp);
	tmp = rpt_value_string(os);
	str_free(os);
	rpt_value_list_append(result, tmp);
	rpt_value_free(tmp);

	/*
	 * skip line terminator and spaces
	 */
	sp = end_p;
	if (*sp == '\n')
	    ++sp;
	while (isspace((unsigned char)*sp) && *sp != '\n')
	    ++sp;
    }

    /*
     * clean up and go home
     */
    str_free(subject);
    return result;
}


rpt_func_ty rpt_func_wrap =
{
    "wrap",
    1, /* optimizable */
    wrap_verify,
    wrap_run
};


static int
wrap_html_verify(rpt_expr_ty *ep)
{
    return (ep->nchild == 2);
}


static rpt_value_ty *
wrap_html_run(rpt_expr_ty *ep, size_t argc, rpt_value_ty **argv)
{
    rpt_value_ty    *a1;
    rpt_value_ty    *a2;
    rpt_value_ty    *result;
    rpt_value_ty    *tmp;
    string_ty	    *subject;
    string_ty	    *s;
    long	    width;
    char	    *sp;

    a1 = rpt_value_stringize(argv[0]);
    if (a1->method->type != rpt_value_type_string)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	rpt_value_free(a1);
	sub_var_set_charstar(scp, "Function", "wrap");
	sub_var_set_charstar(scp, "Number", "1");
	sub_var_set_charstar(scp, "Name", argv[0]->method->name);
	s =
	    subst_intl
	    (
	       	scp,
    i18n("$function: argument $number: string value required (was given $name)")
	    );
	sub_context_delete(scp);
	tmp = rpt_value_error(ep->pos, s);
	str_free(s);
	return tmp;
    }
    subject = trim(rpt_value_string_query(a1));
    rpt_value_free(a1);

    a2 = rpt_value_integerize(argv[1]);
    if (a2->method->type != rpt_value_type_integer)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	rpt_value_free(a2);
	sub_var_set_charstar(scp, "Function", "wrap");
	sub_var_set_charstar(scp, "Number", "2");
	sub_var_set_charstar(scp, "Name", argv[1]->method->name);
	s =
	    subst_intl
	    (
	       	scp,
   i18n("$function: argument $number: integer value required (was given $name)")
	    );
	sub_context_delete(scp);
	tmp = rpt_value_error(ep->pos, s);
	str_free(s);
	return tmp;
    }
    width = rpt_value_integer_query(a2);
    rpt_value_free(a2);
    if (width < 1)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Function", "wrap");
	sub_var_set_charstar(scp, "Number", "2");
	sub_var_set_long(scp, "Value", width);
	s =
	    subst_intl
	    (
	       	scp,
	        i18n("$function: argument $number: width $value out of range")
	    );
	sub_context_delete(scp);
	tmp = rpt_value_error(ep->pos, s);
	str_free(s);
	return tmp;
    }

    /*
     * the result is a list
     * create an empty one se we can start filling it
     */
    result = rpt_value_list();

    sp = subject->str_text;
    while (*sp)
    {
	char		*end_p;
	string_ty	*os;

	/*
	 * find where the line ends
	 */
	end_p = sp;
	while (end_p - sp < width && *end_p && *end_p != '\n')
	    ++end_p;
	if (*end_p && *end_p != '\n')
	{
	    char	    *w;

	    /*
	     * see if there is a better place to wrap
	     */
	    w = end_p;
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

	/*
	 * append the line to the result
	 */
	os = str_n_from_c(sp, end_p - sp);
	tmp = rpt_value_string(os);
	str_free(os);
	rpt_value_list_append(result, tmp);
	rpt_value_free(tmp);

	/*
	 * skip line terminator and spaces
	 *
	 * Insert HTML paragraph breaks or line breaks
	 * if appropriate.
	 */
	sp = end_p;
	while (*sp && *sp != '\n' && isspace((unsigned char)*sp))
	    ++sp;
	if (*sp == '\n')
	{
	    ++sp;
	    if (*sp && isspace((unsigned char)*sp))
	    {
		os = str_from_c("<p>");
		tmp = rpt_value_string(os);
		str_free(os);
		rpt_value_list_append(result, tmp);
		rpt_value_free(tmp);
	    }
	    else
	    {
		os = str_from_c("<br>");
		tmp = rpt_value_string(os);
		str_free(os);
		rpt_value_list_append(result, tmp);
		rpt_value_free(tmp);
	    }
	}
	while (*sp && isspace((unsigned char)*sp))
	    ++sp;
    }

    /*
     * clean up and go home
     */
    str_free(subject);
    return result;
}


rpt_func_ty rpt_func_wrap_html =
{
    "wrap_html",
    1, /* optimizable */
    wrap_html_verify,
    wrap_html_run
};
