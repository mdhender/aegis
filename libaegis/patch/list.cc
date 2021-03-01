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

#include <libaegis/patch/list.h>


patch_list_ty *
patch_list_new(void)
{
    patch_list_ty *this_thing = new patch_list_ty;
    this_thing->project_name = 0;
    this_thing->change_number = 0;
    this_thing->brief_description = 0;
    this_thing->description = 0;
    this_thing->comment = 0;

    this_thing->length = 0;
    this_thing->maximum = 0;
    this_thing->item = 0;
    return this_thing;
}


void
patch_list_delete(patch_list_ty *this_thing)
{
    if (this_thing->project_name)
    {
	str_free(this_thing->project_name);
	this_thing->project_name = 0;
    }
    this_thing->change_number = 0;
    if (this_thing->brief_description)
    {
	str_free(this_thing->brief_description);
	this_thing->brief_description = 0;
    }
    if (this_thing->description)
    {
	str_free(this_thing->description);
	this_thing->description = 0;
    }
    if (this_thing->comment)
    {
	str_free(this_thing->comment);
	this_thing->comment = 0;
    }

    for (size_t j = 0; j < this_thing->length; ++j)
	patch_delete(this_thing->item[j]);
    delete [] this_thing->item;
    this_thing->length = 0;
    this_thing->maximum = 0;
    this_thing->item = 0;
    delete this_thing;
}


void
patch_list_append(patch_list_ty *this_thing, patch_ty *pp)
{
    if (this_thing->length >= this_thing->maximum)
    {
	this_thing->maximum = this_thing->maximum * 2 + 4;
	patch_ty **new_item = new patch_ty * [this_thing->maximum];
	for (size_t j = 0; j < this_thing->length; ++j)
	    new_item[j] = this_thing->item[j];
	delete [] this_thing->item;
	this_thing->item = new_item;
    }
    this_thing->item[this_thing->length++] = pp;
}
