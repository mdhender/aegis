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
 * MANIFEST: functions to manipulate hunk_lists
 */

#include <mem.h>
#include <patch/hunk_list.h>


void
patch_hunk_list_constructor(this)
	patch_hunk_list_ty *this;
{
	this->length = 0;
	this->maximum = 0;
	this->item = 0;
}


void
patch_hunk_list_destructor(this)
	patch_hunk_list_ty *this;
{
	size_t		j;

	for (j = 0; j < this->length; ++j)
		patch_hunk_delete(this->item[j]);
	if (this->item)
		mem_free(this->item);
	this->length = 0;
	this->maximum = 0;
	this->item = 0;
}


void
patch_hunk_list_append(this, php)
	patch_hunk_list_ty *this;
	patch_hunk_ty	*php;
{
	if (this->length >= this->maximum)
	{
		size_t		nbytes;

		this->maximum = this->maximum * 2 + 8;
		nbytes = this->maximum * sizeof(this->item[0]);
		this->item = mem_change_size(this->item, nbytes);
	}
	this->item[this->length++] = php;
}
