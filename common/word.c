/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993 Peter Miller.
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
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: functions to manipulate lists of strings
 */

#include <ctype.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <main.h>
#include <string.h>
#include <word.h>
#include <error.h>
#include <mem.h>


/*
 * NAME
 *	wl_append - append to a word list
 *
 * SYNOPSIS
 *	void wl_append(wlist *wlp, string_ty *wp);
 *
 * DESCRIPTION
 *	Wl_append is used to append to a word list.
 *
 * CAVEAT
 *	The word being appended IS copied.
 */

void
wl_append(wlp, w)
	wlist		*wlp;
	string_ty	*w;
{
	*(string_ty **)
	enlarge
	(
		&wlp->wl_nwords,
		(char **)&wlp->wl_word, sizeof(string_ty *)
	) =
		str_copy(w);
}


/*
 * NAME
 *	wl_free - free a word list
 *
 * SYNOPSIS
 *	void wl_free(wlist *wlp);
 *
 * DESCRIPTION
 *	Wl_free is used to free the contents of a word list
 *	when it is finished with.
 *
 * CAVEAT
 *	It is assumed that the contents of the word list were all
 *	created using strdup() or similar, and grown using wl_append().
 */

void
wl_free(wlp)
	wlist		*wlp;
{
	int		j;

	for (j = 0; j < wlp->wl_nwords; j++)
		str_free(wlp->wl_word[j]);
	if (wlp->wl_nwords)
		free(wlp->wl_word);
	wlp->wl_nwords = 0;
	wlp->wl_word = 0;
}


/*
 * NAME
 *	wl_member - word list membership
 *
 * SYNOPSIS
 *	int wl_member(wlist *wlp, string_ty *wp);
 *
 * DESCRIPTION
 *	Wl_member is used to determine if the given word is
 *	contained in the given word list.
 *
 * RETURNS
 *	A zero if the word is not in the list,
 *	and a non-zero if it is.
 */

int
wl_member(wlp, w)
	wlist		*wlp;
	string_ty	*w;
{
	int		j;

	for (j = 0; j < wlp->wl_nwords; j++)
		if (str_equal(wlp->wl_word[j], w))
			return 1;
	return 0;
}


/*
 * NAME
 *	wl_copy - copy a word list
 *
 * SYNOPSIS
 *	void wl_copy(wlist *to, wlist *from);
 *
 * DESCRIPTION
 *	Wl_copy is used to copy word lists.
 *
 * RETURNS
 *	A copy of the 'to' word list is placed in 'from'.
 *
 * CAVEAT
 *	It is the responsibility of the caller to ensure that the
 *	new word list is freed when finished with, by a call to wl_free().
 */

void
wl_copy(to, from)
	wlist		*to;
	wlist		*from;
{
	int		j;

	wl_zero(to);
	for (j = 0; j < from->wl_nwords; j++)
		wl_append(to, str_copy(from->wl_word[j]));
}


/*
 * NAME
 *	wl2str - form a string from a word list
 *
 * SYNOPSIS
 *	string_ty *wl2str(wlist *wlp, int start, int stop);
 *
 * DESCRIPTION
 *	Wl2str is used to form a string from a word list.
 *
 * RETURNS
 *	A pointer to the newly formed string in dynamic memory.
 *
 * CAVEAT
 *	It is the responsibility of the caller to ensure that the
 *	new string is freed when finished with, by a call to free().
 */

string_ty *
wl2str(wl, start, stop)
	wlist		*wl;
	int		start;
	int		stop;
{
	int		j;
	static char	*tmp;
	static size_t	tmplen;
	size_t		length;
	char		*pos;
	string_ty	*s;

	length = 0;
	for (j = start; j <= stop && j < wl->wl_nwords; j++)
	{
		s = wl->wl_word[j];
		if (s->str_length)
		{
			if (length)
				++length;
			length += s->str_length;
		}
	}

	if (!tmp)
	{
		tmplen = length;
		if (tmplen < 16)
			tmplen = 16;
		tmp = mem_alloc(tmplen);
	}
	else
	{
		if (tmplen < length)
		{
			tmplen = length;
			mem_change_size(&tmp, tmplen);
		}
	}

	pos = tmp;
	for (j = start; j <= stop && j < wl->wl_nwords; j++)
	{
		s = wl->wl_word[j];
		if (s->str_length)
		{
			if (pos != tmp)
				*pos++ = ' ';
			memcpy(pos, s->str_text, s->str_length);
			pos += s->str_length;
		}
	}

	s = str_n_from_c(tmp, length);
	return s;
}


/*
 * NAME
 *	str2wl - string to word list
 *
 * SYNOPSIS
 *	void str2wl(wlist *wlp, string_ty *s);
 *
 * DESCRIPTION
 *	Str2wl is used to form a word list from a string.
 *
 * RETURNS
 *	The string is broken on spaces into words,
 *	using strndup() and wl_append().
 *
 * CAVEAT
 *	Quoting is not understood.
 */

void
str2wl(slp, s, sep)
	wlist		*slp;
	string_ty	*s;
	char		*sep;
{
	char		*cp;
	int		more;

	wl_zero(slp);
	cp = s->str_text;
	more = 0;
	while (*cp || more)
	{
		string_ty	*w;
		char		*cp1;
		char		*cp2;

		while (isspace(*cp))
			cp++;
		if (!*cp && !more)
			break;
		more = 0;
		cp1 = cp;
		if (sep)
		{
			while (*cp && !strchr(sep, *cp))
				cp++;
			if (*cp)
			{
				cp2 = cp + 1;
				more = 1;
			}
			else
				cp2 = cp;
			while (cp > cp1 && isspace(cp[-1]))
				cp--;
		}
		else
		{
			while (*cp && !isspace(*cp))
				cp++;
			if (*cp)
				cp2 = cp + 1;
			else
				cp2 = cp;
		}
		w = str_n_from_c(cp1, cp - cp1);
		wl_append(slp, w);
		str_free(w);
		cp = cp2;
	}
}


/*
 * NAME
 *	wl_insert - a insert a word into a list
 *
 * SYNOPSIS
 *	void wl_insert(wlist *wlp, string_ty *wp);
 *
 * DESCRIPTION
 *	Wl_insert is similar to wl_append, however it does not
 *	append the word unless it is not already in the list.
 *
 * CAVEAT
 *	If the word is inserted it is copied.
 */

void
wl_append_unique(wlp, wp)
	wlist		*wlp;
	string_ty	*wp;
{
	int		j;

	for (j = 0; j < wlp->wl_nwords; j++)
		if (str_equal(wlp->wl_word[j], wp))
			return;
	wl_append(wlp, wp);
}


/*
 * NAME
 *	wl_delete - remove list member
 *
 * SYNOPSIS
 *	void wl_delete(wlist *wlp, string_ty *wp);
 *
 * DESCRIPTION
 *	The wl_delete function is used to delete a member of a word list.
 *
 * RETURNS
 *	void
 */

void
wl_delete(wlp, wp)
	wlist		*wlp;
	string_ty	*wp;
{
	int		j;
	int		k;

	for (j = 0; j < wlp->wl_nwords; ++j)
	{
		if (str_equal(wlp->wl_word[j], wp))
		{
			wlp->wl_nwords--;
			for (k = j; k < wlp->wl_nwords; ++k)
				wlp->wl_word[k] = wlp->wl_word[k + 1];
			str_free(wp);
			break;
		}
	}
}

void
wl_zero(wlp)
	wlist		*wlp;
{
	wlp->wl_nwords = 0;
	wlp->wl_word = 0;
}
