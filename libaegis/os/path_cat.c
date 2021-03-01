/*
 *	aegis - project change supervisor
 *	Copyright (C) 1998, 1999 Peter Miller;
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
 * MANIFEST: functions to join path elements
 */

#include <os.h>


string_ty *
os_path_cat(s1, s2)
	string_ty	*s1;
	string_ty	*s2;
{
	static string_ty *dot;
	size_t		s1len;
	char		*cp2;
	size_t		cp2len;

	if (!dot)
		dot = str_from_c(".");
	if (!s1->str_length)
		s1 = dot;
	if (!s2->str_length)
		s2 = dot;
	if (str_equal(s1, dot))
		return str_copy(s2);
	if (str_equal(s2, dot))
		return str_copy(s1);
	for
	(
		s1len = s1->str_length;
		s1len > 0 && s1->str_text[s1len - 1] == '/';
		--s1len
	)
		;
	cp2 = s2->str_text;
	cp2len = s2->str_length;
	while (cp2len > 0 && *cp2 == '/')
		cp2++, cp2len--;
	return str_format("%.*S/%.*s", (int)s1len, s1, (int)cp2len, cp2);
}


string_ty *
os_path_rel2abs(root, path)
	string_ty	*root;
	string_ty	*path;
{
	if (path->str_text[0] == '/')
		return str_copy(path);
	return os_path_cat(root, path);
}
