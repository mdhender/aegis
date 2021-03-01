/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001 Peter Miller;
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
patch_list_new()
{
	patch_list_ty	*this;

	this = mem_alloc(sizeof(patch_list_ty));
	this->project_name = 0;
	this->change_number = 0;
	this->brief_description = 0;
	this->description = 0;

	this->length = 0;
	this->maximum = 0;
	this->item = 0;
	return this;
}


void
patch_list_delete(this)
	patch_list_ty	*this;
{
	size_t		j;

	if (this->project_name)
	{
		str_free(this->project_name);
		this->project_name = 0;
	}
	this->change_number = 0;
	if (this->brief_description)
	{
		str_free(this->brief_description);
		this->brief_description = 0;
	}
	if (this->description)
	{
		str_free(this->description);
		this->description = 0;
	}

	for (j = 0; j < this->length; ++j)
		patch_delete(this->item[j]);
	if (this->item)
		mem_free(this->item);
	this->length = 0;
	this->maximum = 0;
	this->item = 0;
}


void
patch_list_append(this, pp)
	patch_list_ty	*this;
	patch_ty	*pp;
{
	if (this->length >= this->maximum)
	{
		size_t		nbytes;

		this->maximum = this->maximum * 2 + 4;
		nbytes = this->maximum * sizeof(this->item[0]);
		this->item = mem_change_size(this->item, nbytes);
	}
	this->item[this->length++] = pp;
}
