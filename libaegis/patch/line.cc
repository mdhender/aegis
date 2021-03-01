//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2003-2005 Peter Miller;
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
// MANIFEST: functions to manipulate lines
//

#include <libaegis/patch/line.h>


void
patch_line_constructor(patch_line_ty *this_thing, patch_line_type type,
    string_ty *value)
{
    this_thing->type = type;
    this_thing->value = str_copy(value);
}


void
patch_line_destructor(patch_line_ty *this_thing)
{
    str_free(this_thing->value);
    this_thing->type = (patch_line_type)0;
    this_thing->value = 0;
}
