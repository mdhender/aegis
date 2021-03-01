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
// MANIFEST: functions to manipulate hunk_lists
//

#include <common/mem.h>
#include <libaegis/patch/hunk_list.h>


void
patch_hunk_list_constructor(patch_hunk_list_ty *this_thing)
{
	this_thing->length = 0;
	this_thing->maximum = 0;
	this_thing->item = 0;
}


void
patch_hunk_list_destructor(patch_hunk_list_ty *this_thing)
{
	size_t		j;

	for (j = 0; j < this_thing->length; ++j)
		patch_hunk_delete(this_thing->item[j]);
	if (this_thing->item)
		mem_free(this_thing->item);
	this_thing->length = 0;
	this_thing->maximum = 0;
	this_thing->item = 0;
}


void
patch_hunk_list_append(patch_hunk_list_ty *this_thing, patch_hunk_ty *php)
{
	if (this_thing->length >= this_thing->maximum)
	{
		size_t		nbytes;

		this_thing->maximum = this_thing->maximum * 2 + 8;
		nbytes = this_thing->maximum * sizeof(this_thing->item[0]);
		this_thing->item =
                    (patch_hunk_ty **)mem_change_size(this_thing->item, nbytes);
	}
	this_thing->item[this_thing->length++] = php;
}
