/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate capitalizes
 */

#include <ac/ctype.h>

#include <language.h>
#include <str.h>
#include <stracc.h>


string_ty *
str_capitalize(string_ty *s)
{
    static stracc_t sa;
    size_t          j;
    int             prev_was_alpha;

    language_human();
    prev_was_alpha = 0;
    stracc_open(&sa);
    for (j = 0; j < s->str_length; ++j)
    {
	char            c;

	c = s->str_text[j];
	if (islower(c))
	{
	    if (!prev_was_alpha)
		c = toupper(c);
	    prev_was_alpha = 1;
	}
	else if (isupper(c))
	{
	    if (prev_was_alpha)
		c = tolower(c);
	    prev_was_alpha = 1;
	}
	else
	    prev_was_alpha = 0;
	stracc_char(&sa, c);
    }
    language_C();
    return stracc_close(&sa);
}
