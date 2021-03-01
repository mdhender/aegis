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
// MANIFEST: functions to manipulate rects
//

#include <ac/stdlib.h>

#include <get/rect.h>
#include <rect.h>
#include <str_list.h>


static long
string_to_integer(string_ty *s, long dflt)
{
    long            result;
    char            *end;

    result = strtol(s->str_text, &end, 0);
    if (end == s->str_text || *end)
	return dflt;
    return result;
}


void
get_rect(string_list_ty *modifier)
{
    long            width = 0;
    long            height = 0;
    const char      *label = 0;

    if (modifier->nstrings >= 2)
	width = string_to_integer(modifier->string[1], 10);
    else
	width = 10;

    if (modifier->nstrings >= 3)
	height = string_to_integer(modifier->string[2], 10);
    else
	height = 10;

    if (modifier->nstrings >= 4)
	label = modifier->string[3]->str_text;

    rect_mime(1);
    rect(0, width, height, label);
}
