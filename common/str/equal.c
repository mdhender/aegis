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
 * MANIFEST: functions to manipulate equals
 */

#include <str.h>


/*
 * NAME
 *	str_equal - test equality of strings
 *
 * SYNOPSIS
 *	int str_equal(string_ty *, string_ty *);
 *
 * DESCRIPTION
 *	The str_equal function is used to test if two strings are equal.
 *
 * RETURNS
 *	int; zero if the strings are not equal, nonzero if the strings are
 *	equal.
 *
 * CAVEAT
 *	This function is implemented as a macro in strings.h
 */

#undef str_equal

int
str_equal(s1, s2)
	string_ty	*s1;
	string_ty	*s2;
{
	return (s1 == s2);
}
