/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001, 2003 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate lists
 */

#include <mem.h>
#include <patch/list.h>
#include <patch/file.h>


patch_list_ty *
patch_list_new(void)
{
	patch_list_ty	*this_thing;

	this_thing = (patch_list_ty *)mem_alloc(sizeof(patch_list_ty));
	this_thing->project_name = 0;
	this_thing->change_number = 0;
	this_thing->brief_description = 0;
	this_thing->description = 0;

	this_thing->length = 0;
	this_thing->maximum = 0;
	this_thing->item = 0;
	return this_thing;
}


void
patch_list_delete(patch_list_ty *this_thing)
{
	size_t		j;

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

	for (j = 0; j < this_thing->length; ++j)
		patch_delete(this_thing->item[j]);
	if (this_thing->item)
		mem_free(this_thing->item);
	this_thing->length = 0;
	this_thing->maximum = 0;
	this_thing->item = 0;
}


void
patch_list_append(patch_list_ty *this_thing, patch_ty *pp)
{
	if (this_thing->length >= this_thing->maximum)
	{
		size_t		nbytes;

		this_thing->maximum = this_thing->maximum * 2 + 4;
		nbytes = this_thing->maximum * sizeof(this_thing->item[0]);
		this_thing->item =
                    (patch_ty **)mem_change_size(this_thing->item, nbytes);
	}
	this_thing->item[this_thing->length++] = pp;
}
