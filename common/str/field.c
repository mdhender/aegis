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
 * MANIFEST: functions to manipulate fields
 */

#include <ac/string.h>

#include <str.h>


/*
 * NAME
 *	str_field - extract a field from a string
 *
 * SYNOPSIS
 *	string_ty *str_field(string_ty *, char separator, int field_number);
 *
 * DESCRIPTION
 *	The str_field functipon is used to erxtract a field from a string.
 *	Fields of the string are separated by ``separator'' characters.
 *	Fields are numbered from 0.
 *
 * RETURNS
 *	Asking for a field off the end of the string will result in a null
 *	pointer return.  The null string is considered to have one empty field.
 */

string_ty *
str_field(s, sep, fldnum)
	string_ty	*s;
	int		sep;
	int		fldnum;
{
	char		*cp;
	char		*ep;

	cp = s->str_text;
	while (fldnum > 0)
	{
		ep = strchr(cp, sep);
		if (!ep)
			return 0;
		cp = ep + 1;
		--fldnum;
	}
	ep = strchr(cp, sep);
	if (ep)
		return str_n_from_c(cp, ep - cp);
	return str_from_c(cp);
}
