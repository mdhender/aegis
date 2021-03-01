/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001 Peter Miller;
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
 * MANIFEST: functions to manipulate downcases
 */

#include <ac/ctype.h>

#include <mem.h>
#include <str.h>


/*
 * NAME
 *	str_downcase - lowercase string
 *
 * SYNOPSIS
 *	string_ty *str_downcase(string_ty *);
 *
 * DESCRIPTION
 *	The str_downcase function is used to form a string which is a lowercase
 *	form of the supplied string.
 *
 * RETURNS
 *	string_ty* - a pointer to a string in dynamic memory.  Use str_free when
 *	finished with.
 *
 * CAVEAT
 *	The contents of the structure pointed to MUST NOT be altered.
 */

string_ty *
str_downcase(s)
	string_ty	*s;
{
	static char	*tmp;
	static size_t	tmplen;
	string_ty	*retval;
	char		*cp1;
	char		*cp2;

	if (!tmp)
	{
		tmplen = s->str_length;
		if (tmplen < 16)
			tmplen = 16;
		tmp = mem_alloc(tmplen);
	}
	else
	{
		if (tmplen < s->str_length)
		{
			tmplen = s->str_length;
			tmp = mem_change_size(tmp, tmplen);
		}
	}
	for (cp1 = s->str_text, cp2 = tmp; *cp1; ++cp1, ++cp2)
	{
		unsigned char c = *cp1;
		if (isupper(c))
			c = tolower(c);
		*cp2 = c;
	}
	retval = str_n_from_c(tmp, s->str_length);
	return retval;
}
