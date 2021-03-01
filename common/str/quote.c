/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999 Peter Miller;
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
str_quote_shell(s)
	string_ty	*s;
{
	const char	*cp;
	int		needs_quoting;
	static stracc_t	buffer;

	/*
	 * Convert the work list to a single string.
	 */
	trace(("str_quote()\n{\n"/*}*/));

	/*
	 * Work out if the string needs quoting.
	 *
	 * The empty string is not quoted, even though it could be argued
	 * that it needs to be.  It has proven more useful in the present
	 * form, because it allows empty filename lists to pass through
	 * and remain empty.
	 */
	needs_quoting = 0;
	for (cp = s->str_text; *cp; ++cp)
	{
		if (isspace(*cp))
		{
			needs_quoting = 1;
			break;
		}
		switch (*cp)
		{
		default:
			continue;

		case '!': case '"': case '#': case '$': case '&': case '\'':
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
		trace((/*{*/"}\n"));
		return s;
	}

	/*
	 * Form the quoted string, using the minimum number of escapes.
	 *
	 * The gotcha here is the backquote: the `blah` substitution is
	 * still active within double quotes.
	 */
	stracc_open(&buffer);
	stracc_char(&buffer, '"');
	for (cp = s->str_text; *cp; ++cp)
	{
		switch (*cp)
		{
		case '\n':
		case '!':
		case '"':
		case '\\':
		case '`':
			stracc_char(&buffer, '\\');
			/* fall through... */

		default:
			stracc_char(&buffer, *cp);
			break;
		}
	}
	stracc_char(&buffer, '"');
	s = stracc_close(&buffer);

	/*
	 * all done
	 */
	trace(("return %8.8lX;\n", (long)s));
	trace((/*{*/"}\n"));
	return s;
}
