//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2003-2006, 2008 Peter Miller
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
