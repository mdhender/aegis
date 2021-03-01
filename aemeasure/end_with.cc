//
//	aegis - project change supervisor
//	Copyright (C) 2003, 2004 Peter Miller;
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
// MANIFEST: functions to manipulate end_withs
//

#include <ac/string.h>

#include <end_with.h>


static int
ends_with(const char *haystack, const char *needle)
{
    size_t          haystack_len;
    size_t          needle_len;

    haystack_len = strlen(haystack);
    needle_len = strlen(needle);
    return
    (
	haystack_len >= needle_len
    &&
	0 == memcmp(haystack + haystack_len - needle_len, needle, needle_len)
    );
}


int
ends_with_one_of(const char *haystack, const char **needles)
{
    while (*needles)
    {
	if (ends_with(haystack, *needles))
	    return 1;
	++needles;
    }
    return 0;
}
