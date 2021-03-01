/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2001, 2002 Peter Miller;
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

#include <str.h>
#include <stracc.h>
#include <trace.h>


string_ty *
str_quote_shell(string_ty *s)
{
    const char	    *cp;
    int		    needs_quoting;
    static stracc_t buffer;
    int		    mode;

    /*
     * Convert the work list to a single string.
     */
    trace(("str_quote()\n{\n"));

    /*
     * Work out if the string needs quoting.
     *
     * The empty string is not quoted, even though it could be argued
     * that it needs to be.  It has proven more useful in the present
     * form, because it allows empty filename lists to pass through
     * and remain empty.
     */
    needs_quoting = 0;
    mode = 0;
    for (cp = s->str_text; *cp; ++cp)
    {
	if (isspace((unsigned char)*cp))
	{
	    needs_quoting = 1;
	    break;
	}
	switch (*cp)
	{
	default:
	    continue;

	case '!':
	    /* special for bash and csh */
	    if (!mode)
	       	mode = '\'';
	    needs_quoting = 1;
	    break;

	case '\'':
	    if (!mode)
	       	mode = '"';
	    needs_quoting = 1;
	    break;

	case '"': case '#': case '$': case '&':
	case '(': case ')': case '*': case ':': case ';': case '<':
	case '=': case '>': case '?': case '[': case '\\': case ']':
	case '^': case '`': case '{': case '|': case '}': case '~':
	    needs_quoting = 1;
	    break;
	}
	break;
    }

    /*
     * If it doesn't need quoting, return immediately.
     */
    if (!needs_quoting)
    {
	s = str_copy(s);
	trace(("return %8.8lX;\n", (long)s));
	trace(("}\n"));
	return s;
    }

    /*
     * If we have a choice, use single quote mode,
     * it's shorter and easier to read.
     */
    if (!mode)
	mode ='\'';

    /*
     * Form the quoted string, using the minimum number of escapes.
     *
     * The gotcha here is the backquote: the `blah` substitution is
     * still active within double quotes.  And so are a few others.
     *
     * Also, there are some difficulties: the single quote can't be
     * quoted within single quotes, and the exclamation mark can't
     * be quoted by anything *except* single quotes.  Sheesh.
     *
     * Also, the rules change depending on which style of quoting
     * is in force at the time.
     */
    stracc_open(&buffer);
    stracc_char(&buffer, mode);
    for (cp = s->str_text; *cp; ++cp)
    {
	if (mode == '\'')
	{
    	    /* within single quotes */
    	    if (*cp == '\'')
    	    {
		/*
		 * You can't quote a single quote within
		 * single quotes.  Need to change to
		 * double quote mode.
		 */
		stracc_chars(&buffer, "'\"'", 3);
		mode = '"';
	    }
	    else
		stracc_char(&buffer, *cp);
	}
	else
	{
	    /* within double quotes */
	    switch (*cp)
	    {
	    case '!':
		/*
		 * You can't quote an exclamation mark
		 * within double quotes.  Need to change
		 * to single quote mode.
		 */
		stracc_chars(&buffer, "\"'!", 3);
		mode = '\'';
		break;

	    case '\n':
	    case '"':
	    case '\\':
	    case '`': /* stop command substitutions */
	    case '$': /* stop variable substitutions */
		    stracc_char(&buffer, '\\');
		    /* fall through... */

	    default:
		stracc_char(&buffer, *cp);
		break;
	    }
	}
    }
    stracc_char(&buffer, mode);
    s = stracc_close(&buffer);

    /*
     * all done
     */
    trace(("return %8.8lX;\n", (long)s));
    trace(("}\n"));
    return s;
}
