//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate updateprefixs
//

#include <common/ac/string.h>

#include <aecvsserver/module.h>


bool
is_update_prefix(string_ty *above, string_ty *below, bool deep)
{
    if (str_equal(below, above))
	return true;
    if
    (
	below->str_length < above->str_length
    ||
	below->str_text[above->str_length] != '/'
    ||
	0 != memcmp(below->str_text, above->str_text, above->str_length)
    )
	return false;
    if (deep)
	return true;
    const char *cp = below->str_text + above->str_length + 1;
    while (*cp == '/')
	++cp;
    if (!*cp)
	return true;
    return (0 == strchr(cp, '/'));
}
