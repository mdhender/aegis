/*
 *	aegis - project change supervisor
 *	Copyright (C) 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate str2wls
 */

#include <ac/ctype.h>
#include <ac/string.h>

#include <str_list.h>


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
str2wl(string_list_ty *slp, string_ty *s, const char *sep, int ewhite)
{
    char            *cp;
    int             more;

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
	string_ty       *w;
	char            *cp1;
	char            *cp2;

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
