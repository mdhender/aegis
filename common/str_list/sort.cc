//
//	aegis - project change supervisor
//	Copyright (C) 2003-2006 Peter Miller
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
// MANIFEST: functions to manipulate sorts
//

#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/str_list.h>


static int
wl_sort_cmp(const void *va, const void *vb)
{
    string_ty *a = *(string_ty **)va;
    string_ty *b = *(string_ty **)vb;
    // C locale
    return strcmp(a->str_text, b->str_text);
}


void
string_list_ty::sort()
{
    qsort(string, nstrings, sizeof(string[0]), wl_sort_cmp);
}
