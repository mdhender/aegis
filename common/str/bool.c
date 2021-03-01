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
 * MANIFEST: functions to manipulate bools
 */

#include <ac/ctype.h>

#include <str.h>


/*
 * NAME
 *	str_bool - get boolean value
 *
 * SYNOPSIS
 *	int str_bool(string_ty *s);
 *
 * DESCRIPTION
 *	The str_bool function is used to determine the boolean value of the
 *	given string.  A "false" result is if the string is empty or
 *	0 or blank, and "true" otherwise.
 *
 * RETURNS
 *	int: zero to indicate a "false" result, nonzero to indicate a "true"
 *	result.
 */

int
str_bool(s)
	string_ty	*s;
{
	char		*cp;

	cp = s->str_text;
	while (*cp)
	{
		if (!isspace((unsigned char)*cp) && *cp != '0')
			return 1;
		++cp;
	}
	return 0;
}
