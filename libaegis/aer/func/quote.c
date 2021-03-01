/*
 *	aegis - project change supervisor
 *	Copyright (C) 1998 Peter Miller;
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
#include <mem.h>
#include <sub.h>


static int quote_url_verify _((rpt_expr_ty *));

static int
quote_url_verify(ep)
	rpt_expr_ty	*ep;
{
	return (ep->nchild == 1);
}


static rpt_value_ty *quote_url_run _((rpt_expr_ty *, size_t, rpt_value_ty **));

static rpt_value_ty *
quote_url_run(ep, argc, argv)
	rpt_expr_ty	*ep;
	size_t		argc;
	rpt_value_ty	**argv;
{
	rpt_value_ty	*a1;
	rpt_value_ty	*result;
	string_ty	*s;
	char		*sp;
	size_t		pos;
	static size_t	max;
	static char	*buffer;

	a1 = rpt_value_stringize(argv[0]);
	if (a1->method->type != rpt_value_type_string)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		rpt_value_free(a1);
		sub_var_set(scp, "Function", "quote_url");
		sub_var_set(scp, "Number", "1");
		sub_var_set(scp, "Name", "%s", argv[0]->method->name);
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

	pos = 0;
	sp = s->str_text;
	for (;;)
	{
		int c = (unsigned char)*sp++;
		if (!c)
			break;
		if (pos + 3 > max)
		{
			max = max * 2 + 16;
			buffer = mem_change_size(buffer, max);
		}
		/* C locale */
		if (!isprint(c) || strchr(" \"#%&'+:=?~", c))
		{
			/*
			 *	#:?	URL special characters
			 *	%	escape the escape character
			 *	~	illegal in portable character sets
			 *	&=	confuse forms-based browsers
			 *	"'	I've included some shell meta
			 *		characters here, too, because
			 *		these are usually passed through
			 *		shell scripts.
			 */
			static char hex[] = "0123456789ABCDEF";
			buffer[pos++] = '%';
			buffer[pos++] = hex[(c >> 4) & 15];
			buffer[pos++] = hex[c & 15];
		}
		else
			buffer[pos++] = c;
	}
	str_free(s);
	s = str_n_from_c(buffer, pos);
	result = rpt_value_string(s);
	str_free(s);

	/*
	 * clean up and go home
	 */
	return result;
}


rpt_func_ty rpt_func_quote_url =
{
	"quote_url",
	1, /* optimizable */
	quote_url_verify,
	quote_url_run
};


static int unquote_url_verify _((rpt_expr_ty *));

static int
unquote_url_verify(ep)
	rpt_expr_ty	*ep;
{
	return (ep->nchild == 1);
}


static int unhex _((int));

static int
unhex(c)
	int		c;
{
	switch (c)
	{
	default:
		return -1;
	
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9': 
		return (c - '0');

	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': 
		return (c - 'a' + 10);

	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': 
		return (c - 'A' + 10);
	}
}


static rpt_value_ty *unquote_url_run _((rpt_expr_ty *, size_t, rpt_value_ty **));

static rpt_value_ty *
unquote_url_run(ep, argc, argv)
	rpt_expr_ty	*ep;
	size_t		argc;
	rpt_value_ty	**argv;
{
	rpt_value_ty	*a1;
	rpt_value_ty	*result;
	string_ty	*s;
	char		*sp;
	int		c;
	int		n1, n2;
	size_t		pos;
	static size_t	max;
	static char	*buffer;

	a1 = rpt_value_stringize(argv[0]);
	if (a1->method->type != rpt_value_type_string)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		rpt_value_free(a1);
		sub_var_set(scp, "Function", "unquote_url");
		sub_var_set(scp, "Number", "1");
		sub_var_set(scp, "Name", "%s", argv[0]->method->name);
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

	pos = 0;
	sp = s->str_text;
	for (;;)
	{
		c = (unsigned char)*sp++;
		if (!c)
			break;
		if
		(
			c == '%'
		&&
			(n1 = unhex(sp[0])) >= 0
		&&
			(n2 = unhex(sp[1])) >= 0
		)
		{
			c = (n1 << 4) + n2;
			sp += 2;
		}
		if (pos >= max)
		{
			max = max * 2 + 16;
			buffer = mem_change_size(buffer, max);
		}
		buffer[pos++] = c;
	}
	str_free(s);
	s = str_n_from_c(buffer, pos);
	result = rpt_value_string(s);
	str_free(s);

	/*
	 * clean up and go home
	 */
	return result;
}


rpt_func_ty rpt_func_unquote_url =
{
	"unquote_url",
	1, /* optimizable */
	unquote_url_verify,
	unquote_url_run
};


static int quote_html_verify _((rpt_expr_ty *));

static int
quote_html_verify(ep)
	rpt_expr_ty	*ep;
{
	return (ep->nchild == 1);
}


static rpt_value_ty *quote_html_run _((rpt_expr_ty *, size_t, rpt_value_ty **));

static rpt_value_ty *
quote_html_run(ep, argc, argv)
	rpt_expr_ty	*ep;
	size_t		argc;
	rpt_value_ty	**argv;
{
	rpt_value_ty	*a1;
	rpt_value_ty	*result;
	string_ty	*s;
	char		*sp;
	size_t		pos;
	static size_t	max;
	static char	*buffer;

	a1 = rpt_value_stringize(argv[0]);
	if (a1->method->type != rpt_value_type_string)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		rpt_value_free(a1);
		sub_var_set(scp, "Function", "quote_html");
		sub_var_set(scp, "Number", "1");
		sub_var_set(scp, "Name", "%s", argv[0]->method->name);
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

	pos = 0;
	sp = s->str_text;
	for (;;)
	{
		int c = (unsigned char)*sp++;
		if (!c)
			break;
		if (pos + 5 > max)
		{
			max = max * 2 + 16;
			buffer = mem_change_size(buffer, max);
		}
		if (c == '<')
		{
			buffer[pos++] = '&';
			buffer[pos++] = 'l';
			buffer[pos++] = 't';
			buffer[pos++] = ';';
		}
		else if (c == '&')
		{
			buffer[pos++] = '&';
			buffer[pos++] = 'a';
			buffer[pos++] = 'm';
			buffer[pos++] = 'p';
			buffer[pos++] = ';';
		}
		else if (isspace(c) || isprint(c))
			buffer[pos++] = c;
		else
		{
			/*
			 * The above isprint() is in the C locale.
			 * This is probably not correct.
			 *
			 * To get it exactly right, we need to know
			 * the User's locale, and map the characters
			 * according to the character set used by the
			 * current locale.  This information isn't
			 * readily available.
			 *
			 * Decimal!  Why didn't these guys just use the
			 * one escape mechanism twice.  Sheesh.
			 */
			buffer[pos++] = '&';
			if (c >= 100)
				buffer[pos++] = '0' + (c / 100) % 10;
			if (c >= 10)
				buffer[pos++] = '0' + (c / 10) % 10;
			buffer[pos++] = '0' + c % 10;
			buffer[pos++] = ';';
		}
	}
	str_free(s);
	s = str_n_from_c(buffer, pos);
	result = rpt_value_string(s);
	str_free(s);

	/*
	 * clean up and go home
	 */
	return result;
}


rpt_func_ty rpt_func_quote_html =
{
	"quote_html",
	1, /* optimizable */
	quote_html_verify,
	quote_html_run
};
