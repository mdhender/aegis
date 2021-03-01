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

#include <common/mem.h>
#include <libaegis/patch/line_list.h>



void
patch_line_list_constructor(patch_line_list_ty *this_thing)
{
    this_thing->start_line_number = -1;
    this_thing->length = 0;
    this_thing->maximum = 0;
    this_thing->item = 0;
}


void
patch_line_list_destructor(patch_line_list_ty *this_thing)
{
    size_t          j;

    for (j = 0; j < this_thing->length; ++j)
	patch_line_destructor(this_thing->item + j);
    delete [] this_thing->item;
    this_thing->start_line_number = -1;
    this_thing->length = 0;
    this_thing->maximum = 0;
    this_thing->item = 0;
}


void
patch_line_list_append(patch_line_list_ty *this_thing, patch_line_type type,
    string_ty *value)
{
    if (this_thing->length >= this_thing->maximum)
    {
	size_t new_maximum = this_thing->maximum * 2 + 8;
	patch_line_ty *new_item = new patch_line_ty [new_maximum];
	for (size_t j = 0; j < this_thing->length; ++j)
	    new_item[j] = this_thing->item[j];
	delete [] this_thing->item;
	this_thing->item = new_item;
	this_thing->maximum = new_maximum;
    }
    patch_line_constructor(
        this_thing->item + this_thing->length++, type, value);
}
