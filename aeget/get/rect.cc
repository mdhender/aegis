//
//	aegis - project change supervisor
//	Copyright (C) 2003-2006, 2008 Peter Miller
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

#include <common/error.h> // for assert
#include <common/str_list.h>
#include <aeget/get/rect.h>
#include <aeget/rect.h>


static long
string_to_integer(const char *s, long dflt)
{
    char *end = 0;
    long result = strtol(s, &end, 0);
    assert(end);
    if (end == s || *end)
	return dflt;
    return result;
}


static bool
string_to_integer3(const char *s, long *result)
{
    char *end = 0;
    long n = strtol(s, &end, 0);
    assert(end);
    if (end == s || *end != ',')
	return false;
    *result++ = n;
    s = end + 1;

    end = 0;
    n = strtol(s, &end, 0);
    assert(end);
    if (end == s || *end != ',')
	return false;
    *result++ = n;
    s = end + 1;

    end = 0;
    n = strtol(s, &end, 0);
    assert(end);
    if (end == s || *end)
	return false;
    *result++ = n;
    return true;
}


void
get_rect(string_list_ty *modifier_p)
{
    string_list_ty &modifier = *modifier_p;
    long width = 10;
    long height = 10;
    const char *label = 0;
    for (size_t j = 0; j < modifier.size(); ++j)
    {
	string_ty *s = modifier[j];
	if (0 == strncasecmp(s->str_text, "width=", 6))
	{
	    width = string_to_integer(s->str_text + 6, 10);
	}
	if (0 == strncasecmp(s->str_text, "height=", 7))
	{
	    height = string_to_integer(s->str_text + 7, 10);
	}
	if (0 == strncasecmp(s->str_text, "label=", 6))
	{
	    label = s->str_text + 6;
	}
	if (0 == strncasecmp(s->str_text, "color=", 6))
	{
	    long clr[3];
	    if (string_to_integer3(s->str_text + 6, clr))
		rect_color(clr[0], clr[1], clr[2]);
	}
    }
    rect_mime(1);
    rect(0, width, height, label);
}
