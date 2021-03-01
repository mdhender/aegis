//
//	aegis - project change supervisor
//	Copyright (C) 1991-2006, 2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <libaegis/os.h>


string_ty *
os_below_dir(string_ty *higher, string_ty *lower)
{
    char            *s1;
    char            *s2;

    s1 = higher->str_text;
    s2 = lower->str_text;
    while (*s1 == *s2 && *s1)
	s1++, s2++;
    if (*s1)
	return 0;
    if (!*s2)
	return str_from_c("");
    if (*s2 != '/')
	return 0;
    return str_from_c(s2 + 1);
}


nstring
os_below_dir(const nstring &higher, const nstring &lower)
{
    const char *hi = higher.c_str();
    const char *lo = lower.c_str();
    while (*hi == *lo && *hi)
	hi++, lo++;
    if (*hi)
	return "";
    if (!*lo)
	return ".";
    if (*lo != '/')
	return "";
    return nstring(lo + 1);
}
