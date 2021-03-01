/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994 Peter Miller.
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
#include <ac/stddef.h>
#include <ac/string.h>
#include <ac/stdlib.h>
#include <ac/time.h>

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
	size_t		nbytes;

	if (wlp->wl_nwords >= wlp->wl_nwords_max)
	{
		/*
		 * always 8 less than a power of 2, which is
		 * most efficient for many memory allocators
		 */
		wlp->wl_nwords_max = wlp->wl_nwords_max * 2 + 8;
		nbytes = wlp->wl_nwords_max * sizeof(string_ty *);
		wlp->wl_word = mem_change_size(wlp->wl_word, nbytes);
	}
	wlp->wl_word[wlp->wl_nwords++] = str_copy(w);
}


void
wl_prepend(wlp, w)
	wlist		*wlp;
	string_ty	*w;
{
	size_t		nbytes;
	size_t		j;

	if (wlp->wl_nwords >= wlp->wl_nwords_max)
	{
		/*
		 * always 8 less than a power of 2, which is
		 * most efficient for many memory allocators
		 */
		wlp->wl_nwords_max = wlp->wl_nwords_max * 2 + 8;
		nbytes = wlp->wl_nwords_max * sizeof(string_ty *);
		wlp->wl_word = mem_change_size(wlp->wl_word, nbytes);
	}
	for (j = wlp->wl_nwords; j > 0; --j)
		wlp->wl_word[j] = wlp->wl_word[j - 1];
	wlp->wl_nwords++;
	wlp->wl_word[0] = str_copy(w);
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
	size_t		j;

	for (j = 0; j < wlp->wl_nwords; j++)
		str_free(wlp->wl_word[j]);
	if (wlp->wl_word)
		mem_free(wlp->wl_word);
	wlp->wl_nwords = 0;
	wlp->wl_nwords_max = 0;
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
	size_t		j;

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
	size_t		j;

	wl_zero(to);
	for (j = 0; j < from->wl_nwords; j++)
		wl_append(to, str_copy(from->wl_word[j]));
}


/*
 * NAME
 *	wl2str - form a string from a word list
 *
 * SYNOPSIS
 *	string_ty *wl2str(wlist *wlp, int start, int stop, char *sep);
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
wl2str(wl, start, stop, sep)
	wlist		*wl;
	int		start;
	int		stop;
	char		*sep;
{
	int		j;
	static char	*tmp;
	static size_t	tmplen;
	size_t		length;
	size_t		seplen;
	char		*pos;
	string_ty	*s;

	if (!sep)
		sep = " ";
	seplen = strlen(sep);
	length = 0;
	for (j = start; j <= stop && j < wl->wl_nwords; j++)
	{
		s = wl->wl_word[j];
		if (s->str_length)
		{
			if (length)
				length += seplen;
			length += s->str_length;
		}
	}

	if (tmplen < length)
	{
		tmplen = length;
		tmp = mem_change_size(tmp, tmplen);
	}

	pos = tmp;
	for (j = start; j <= stop && j < wl->wl_nwords; j++)
	{
		s = wl->wl_word[j];
		if (s->str_length)
		{
			if (pos != tmp)
			{
				memcpy(pos, sep, seplen);
				pos += seplen;
			}
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
 *	void str2wl(wlist *wlp, string_ty *s, char *sep, int ewhite);
 *
 * DESCRIPTION
 *	Str2wl is used to form a word list from a string.
 *	wlp	- where to put the word list
 *	s	- string to break
 *	sep	- separators, default to " " if 0 given
 *	ewhite	- supress extra white space around separators
 *
 * RETURNS
 *	The string is broken on spaces into words,
 *	using strndup() and wl_append().
 *
 * CAVEAT
 *	Quoting is not understood.
 */

void
str2wl(slp, s, sep, ewhite)
	wlist		*slp;
	string_ty	*s;
	char		*sep;
	int		ewhite;
{
	char		*cp;
	int		more;

	if (!sep)
	{
		sep = " \t\n\f\r";
		ewhite = 1;
	}
	wl_zero(slp);
	cp = s->str_text;
	more = 0;
	while (*cp || more)
	{
		string_ty	*w;
		char		*cp1;
		char		*cp2;

		if (ewhite)
			while (isspace(*cp))
				cp++;
		if (!*cp && !more)
			break;
		more = 0;
		cp1 = cp;
		while (*cp && !strchr(sep, *cp))
			cp++;
		if (*cp)
		{
			cp2 = cp + 1;
			more = 1;
		}
		else
			cp2 = cp;
		if (ewhite)
			while (cp > cp1 && isspace(cp[-1]))
				cp--;
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
	size_t		j;

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
	size_t		j;
	size_t		k;

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
	wlp->wl_nwords_max = 0;
	wlp->wl_word = 0;
}


int
wl_equal(a, b)
	wlist		*a;
	wlist		*b;
{
	size_t		j, k;

	for (j = 0; j < a->wl_nwords; ++j)
	{
		for (k = 0; k < b->wl_nwords; ++k)
			if (str_equal(a->wl_word[j], b->wl_word[k]))
				break;
		if (k >= b->wl_nwords)
			return 0;
	}
	for (j = 0; j < b->wl_nwords; ++j)
	{
		for (k = 0; k < a->wl_nwords; ++k)
			if (str_equal(b->wl_word[j], a->wl_word[k]))
				break;
		if (k >= a->wl_nwords)
			return 0;
	}
	return 1;
}


int
wl_subset(a,b)
	wlist		*a;
	wlist		*b;
{
	size_t		j, k;

	/*
	 * test if "a is a subset of b"
	 */
	if (a->wl_nwords > b->wl_nwords)
		return 0;
	for (j = 0; j < a->wl_nwords; ++j)
	{
		for (k = 0; k < b->wl_nwords; ++k)
			if (str_equal(a->wl_word[j], b->wl_word[k]))
				break;
		if (k >= b->wl_nwords)
			return 0;
	}
	return 1;
}
