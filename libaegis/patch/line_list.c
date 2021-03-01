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
 * MANIFEST: functions to manipulate line_lists
 */

#include <mem.h>
#include <patch/line_list.h>



void
patch_line_list_constructor(this)
	patch_line_list_ty *this;
{
	this->start_line_number = -1;
	this->length = 0;
	this->maximum = 0;
	this->item = 0;
}


void
patch_line_list_destructor(this)
	patch_line_list_ty *this;
{
	size_t		j;

	for (j = 0; j < this->length; ++j)
		patch_line_destructor(this->item + j);
	if (this->item)
		mem_free(this->item);
	this->start_line_number = -1;
	this->length = 0;
	this->maximum = 0;
	this->item = 0;
}


void
patch_line_list_append(this, type, value)
	patch_line_list_ty *this;
	patch_line_type	type;
	string_ty	*value;
{
	if (this->length >= this->maximum)
	{
		size_t		nbytes;

		this->maximum = this->maximum * 2 + 8;
		nbytes = this->maximum * sizeof(this->item[0]);
		this->item = mem_change_size(this->item, nbytes);
	}
	patch_line_constructor(this->item + this->length++, type, value);
}
