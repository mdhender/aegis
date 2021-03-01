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

#include <common/ac/ctype.h>

#include <common/abbreviate.h>
#include <libaegis/ael/build_header.h>
#include <libaegis/ael/column_width.h>


string_ty *
ael_build_header(string_ty *s)
{
    size_t          max;
    int             max2;
    string_ty       *s2;
    string_ty       *s3;
    static const char minus[] = "---------------------------------------------";

    for (max = 0; max < ATTR_WIDTH && max < s->str_length; ++max)
    {
	if (!isprint((unsigned char)s->str_text[max]))
	    break;
    }
    s2 = abbreviate_dirname(s, max);
    max2 = s2->str_length + 2;
    if (max2 > ATTR_WIDTH)
	max2 = ATTR_WIDTH;
    s3 = str_format("%s\n%.*s", s2->str_text, max2, minus);
    str_free(s2);
    return s3;
}
