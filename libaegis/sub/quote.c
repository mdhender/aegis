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
 * MANIFEST: functions to implement the quote substitution
 */

#include <ac/wchar.h>
#include <ac/wctype.h>

#include <mem.h>
#include <sub.h>
#include <sub/quote.h>
#include <trace.h>
#include <wstr_list.h>

static wchar_t	*buffer;
static size_t	buf_max;
static size_t	buf_pos;


static void stash _((wint_t));

static void
stash(c)
	wint_t	c;
{
	if (buf_pos >= buf_max)
	{
		size_t	nbytes;

		buf_max = buf_max * 2 + 8;
		nbytes = buf_max * sizeof(wchar_t);
		buffer = mem_change_size(buffer, nbytes);
	}
	buffer[buf_pos++] = c;
}


wstring_ty *
sub_quote(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*s;
	wchar_t		*wcp;
	int		needs_quoting;

	/*
	 * Convert the work list to a single string.
	 */
	trace(("sub_quote()\n{\n"/*}*/));
	s = wstring_list_to_wstring(arg, (size_t)1, arg->nitems, (char *)0);

	/*
	 * Work out if the string needs quoting.
	 *
	 * The empty string is not quoted, even though it could be argued
	 * that it needs to be.  It has proven more useful in the present
	 * form, because it allows empty filename lists to pass through
	 * and remain empty.
	 */
	needs_quoting = 0;
	for (wcp = s->wstr_text; *wcp; ++wcp)
	{
		if (iswspace(*wcp))
		{
			needs_quoting = 1;
			break;
		}
		switch (*wcp)
		{
		default:
			continue;

		case (wchar_t)'!': case (wchar_t)'"': case (wchar_t)'#':
		case (wchar_t)'$': case (wchar_t)'&': case (wchar_t)'\'':
		case (wchar_t)'(': case (wchar_t)')': case (wchar_t)'*':
		case (wchar_t)':': case (wchar_t)';': case (wchar_t)'<':
		case (wchar_t)'=': case (wchar_t)'>': case (wchar_t)'?':
		case (wchar_t)'[': case (wchar_t)'\\': case (wchar_t)']':
		case (wchar_t)'^': case (wchar_t)'`': case (wchar_t)'{':
		case (wchar_t)'|': case (wchar_t)'}': case (wchar_t)'~':
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
	buf_pos = 0;
	stash((wchar_t)'"');
	for (wcp = s->wstr_text; *wcp; ++wcp)
	{
		switch (*wcp)
		{
		case (wchar_t)'\n':
		case (wchar_t)'!':
		case (wchar_t)'"':
		case (wchar_t)'\\':
		case (wchar_t)'`':
			stash((wchar_t)'\\');
			/* fall through... */

		default:
			stash(*wcp);
			break;
		}
	}
	stash((wchar_t)'"');
	wstr_free(s);

	/*
	 * Turn the buffer back into a wide string, and return it.
	 */
	s = wstr_n_from_wc(buffer, buf_pos);
	trace(("return %8.8lX;\n", (long)s));
	trace((/*{*/"}\n"));
	return s;
}
