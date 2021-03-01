/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1996, 1997, 1998, 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate lists of strings
 */

#include <ac/ctype.h>
#include <ac/stddef.h>
#include <ac/string.h>
#include <ac/stdlib.h>
#include <ac/time.h>

#include <str_list.h>
#include <error.h>
#include <mem.h>


/*
 * NAME
 *	string_list_append - append to a word list
 *
 * SYNOPSIS
 *	void string_list_append(string_list_ty *wlp, string_ty *wp);
 *
 * DESCRIPTION
 *	Wl_append is used to append to a word list.
 *
 * CAVEAT
 *	The word being appended IS copied.
 */

void
string_list_append(wlp, w)
	string_list_ty		*wlp;
	string_ty	*w;
{
	size_t		nbytes;

	if (wlp->nstrings >= wlp->nstrings_max)
	{
		/*
		 * always 8 less than a power of 2, which is
		 * most efficient for many memory allocators
		 */
		wlp->nstrings_max = wlp->nstrings_max * 2 + 8;
		nbytes = wlp->nstrings_max * sizeof(string_ty *);
		wlp->string = mem_change_size(wlp->string, nbytes);
	}
	wlp->string[wlp->nstrings++] = str_copy(w);
}


void
string_list_append_list(wlp, arg)
	string_list_ty		*wlp;
	string_list_ty		*arg;
{
	size_t		j;

	for (j = 0; j < arg->nstrings; ++j)
		string_list_append(wlp, arg->string[j]);
}


void
string_list_prepend(wlp, w)
	string_list_ty		*wlp;
	string_ty	*w;
{
	size_t		nbytes;
	size_t		j;

	if (wlp->nstrings >= wlp->nstrings_max)
	{
		/*
		 * always 8 less than a power of 2, which is
		 * most efficient for many memory allocators
		 */
		wlp->nstrings_max = wlp->nstrings_max * 2 + 8;
		nbytes = wlp->nstrings_max * sizeof(string_ty *);
		wlp->string = mem_change_size(wlp->string, nbytes);
	}
	for (j = wlp->nstrings; j > 0; --j)
		wlp->string[j] = wlp->string[j - 1];
	wlp->nstrings++;
	wlp->string[0] = str_copy(w);
}


/*
 * NAME
 *	string_list_destructor - free a word list
 *
 * SYNOPSIS
 *	void string_list_destructor(string_list_ty *wlp);
 *
 * DESCRIPTION
 *	Wl_free is used to free the contents of a word list
 *	when it is finished with.
 *
 * CAVEAT
 *	It is assumed that the contents of the word list were all
 *	created using strdup() or similar, and grown using string_list_append().
 */

void
string_list_destructor(wlp)
	string_list_ty		*wlp;
{
	size_t		j;

	for (j = 0; j < wlp->nstrings; j++)
		str_free(wlp->string[j]);
	if (wlp->string)
		mem_free(wlp->string);
	wlp->nstrings = 0;
	wlp->nstrings_max = 0;
	wlp->string = 0;
}


/*
 * NAME
 *	string_list_member - word list membership
 *
 * SYNOPSIS
 *	int string_list_member(string_list_ty *wlp, string_ty *wp);
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
string_list_member(wlp, w)
	string_list_ty		*wlp;
	string_ty	*w;
{
	size_t		j;

	for (j = 0; j < wlp->nstrings; j++)
		if (str_equal(wlp->string[j], w))
			return 1;
	return 0;
}


/*
 * NAME
 *	string_list_copy - copy a word list
 *
 * SYNOPSIS
 *	void string_list_copy(string_list_ty *to, string_list_ty *from);
 *
 * DESCRIPTION
 *	Wl_copy is used to copy word lists.
 *
 * RETURNS
 *	A copy of the 'to' word list is placed in 'from'.
 *
 * CAVEAT
 *	It is the responsibility of the caller to ensure that the
 *	new word list is freed when finished with, by a call to string_list_destructor().
 */

void
string_list_copy(to, from)
	string_list_ty		*to;
	string_list_ty		*from;
{
	size_t		j;

	string_list_constructor(to);
	for (j = 0; j < from->nstrings; j++)
		string_list_append(to, str_copy(from->string[j]));
}


string_list_ty *
string_list_new()
{
	string_list_ty	*slp;

	slp = mem_alloc(sizeof(string_list_ty));
	string_list_constructor(slp);
	return slp;
}


void
string_list_delete(slp)
	string_list_ty	*slp;
{
	string_list_destructor(slp);
	mem_free(slp);
}


/*
 * NAME
 *	wl2str - form a string from a word list
 *
 * SYNOPSIS
 *	string_ty *wl2str(string_list_ty *wlp, int start, int stop, char *sep);
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
	string_list_ty		*wl;
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
	for (j = start; j <= stop && j < wl->nstrings; j++)
	{
		s = wl->string[j];
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
	for (j = start; j <= stop && j < wl->nstrings; j++)
	{
		s = wl->string[j];
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
 *	void str2wl(string_list_ty *wlp, string_ty *s, char *sep, int ewhite);
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
 *	using strndup() and string_list_append().
 *
 * CAVEAT
 *	Quoting is not understood.
 */

void
str2wl(slp, s, sep, ewhite)
	string_list_ty		*slp;
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
	string_list_constructor(slp);
	cp = s->str_text;
	more = 0;
	while (*cp || more)
	{
		string_ty	*w;
		char		*cp1;
		char		*cp2;

		if (ewhite)
		{
			while (isspace((unsigned char)*cp))
				cp++;
		}
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
			while (cp > cp1 && isspace((unsigned char)cp[-1]))
				cp--;
		w = str_n_from_c(cp1, cp - cp1);
		string_list_append(slp, w);
		str_free(w);
		cp = cp2;
	}
}


/*
 * NAME
 *	wl_insert - a insert a word into a list
 *
 * SYNOPSIS
 *	void wl_insert(string_list_ty *wlp, string_ty *wp);
 *
 * DESCRIPTION
 *	Wl_insert is similar to string_list_append, however it does not
 *	append the word unless it is not already in the list.
 *
 * CAVEAT
 *	If the word is inserted it is copied.
 */

void
string_list_append_unique(wlp, wp)
	string_list_ty		*wlp;
	string_ty	*wp;
{
	size_t		j;

	for (j = 0; j < wlp->nstrings; j++)
		if (str_equal(wlp->string[j], wp))
			return;
	string_list_append(wlp, wp);
}


void
string_list_append_list_unique(wlp, arg)
	string_list_ty		*wlp;
	string_list_ty		*arg;
{
	size_t		j;

	for (j = 0; j < arg->nstrings; ++j)
		string_list_append_unique(wlp, arg->string[j]);
}


/*
 * NAME
 *	string_list_remove - remove list member
 *
 * SYNOPSIS
 *	void string_list_remove(string_list_ty *wlp, string_ty *wp);
 *
 * DESCRIPTION
 *	The string_list_remove function is used to delete a member of a word list.
 *
 * RETURNS
 *	void
 */

void
string_list_remove(wlp, wp)
	string_list_ty		*wlp;
	string_ty	*wp;
{
	size_t		j;
	size_t		k;

	for (j = 0; j < wlp->nstrings; ++j)
	{
		if (str_equal(wlp->string[j], wp))
		{
			wlp->nstrings--;
			for (k = j; k < wlp->nstrings; ++k)
				wlp->string[k] = wlp->string[k + 1];
			str_free(wp);
			break;
		}
	}
}


void
string_list_remove_list(wlp, arg)
	string_list_ty		*wlp;
	string_list_ty		*arg;
{
	size_t		j;

	for (j = 0; j < arg->nstrings; ++j)
		string_list_remove(wlp, arg->string[j]);
}


void
string_list_constructor(wlp)
	string_list_ty		*wlp;
{
	wlp->nstrings = 0;
	wlp->nstrings_max = 0;
	wlp->string = 0;
}


int
string_list_equal(a, b)
	string_list_ty		*a;
	string_list_ty		*b;
{
	size_t		j, k;

	for (j = 0; j < a->nstrings; ++j)
	{
		for (k = 0; k < b->nstrings; ++k)
			if (str_equal(a->string[j], b->string[k]))
				break;
		if (k >= b->nstrings)
			return 0;
	}
	for (j = 0; j < b->nstrings; ++j)
	{
		for (k = 0; k < a->nstrings; ++k)
			if (str_equal(b->string[j], a->string[k]))
				break;
		if (k >= a->nstrings)
			return 0;
	}
	return 1;
}


int
string_list_subset(a,b)
	string_list_ty		*a;
	string_list_ty		*b;
{
	size_t		j, k;

	/*
	 * test if "a is a subset of b"
	 */
	if (a->nstrings > b->nstrings)
		return 0;
	for (j = 0; j < a->nstrings; ++j)
	{
		for (k = 0; k < b->nstrings; ++k)
			if (str_equal(a->string[j], b->string[k]))
				break;
		if (k >= b->nstrings)
			return 0;
	}
	return 1;
}


static int wl_sort_cmp _((const void *, const void *));

static int
wl_sort_cmp(va, vb)
	const void	*va;
	const void	*vb;
{
	string_ty	*a;
	string_ty	*b;

	a = *(string_ty **)va;
	b = *(string_ty **)vb;
	/* C locale */
	return strcmp(a->str_text, b->str_text);
}


void
string_list_sort(wlp)
	string_list_ty		*wlp;
{
	qsort
	(
		wlp->string,
		wlp->nstrings,
		sizeof(wlp->string[0]),
		wl_sort_cmp
	);
}


#ifdef DEBUG

int
string_list_validate(slp)
	string_list_ty	*slp;
{
	size_t		j;

	for (j = 0; j < slp->nstrings; ++j)
		if (!str_validate(slp->string[j]))
			return 0;
	return 1;
}

#endif
