//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
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

#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/str_list.h>


static int
cmp(const void *va, const void *vb)
{
    string_ty *a = *(string_ty **)va;
    string_ty *b = *(string_ty **)vb;
    // C locale
    return strcasecmp(a->str_text, b->str_text);
}


void
string_list_ty::sort_nocase()
{
    qsort(string, nstrings, sizeof(string[0]), cmp);
}
