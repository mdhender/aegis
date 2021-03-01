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
#include <libaegis/patch/hunk.h>


patch_hunk_ty *
patch_hunk_new(void)
{
	patch_hunk_ty	*this_thing;

	this_thing = (patch_hunk_ty *)mem_alloc(sizeof(patch_hunk_ty));
	this_thing->inserts = 0;
	this_thing->deletes = 0;
	patch_line_list_constructor(&this_thing->before);
	patch_line_list_constructor(&this_thing->after);
	return this_thing;
}


void
patch_hunk_delete(patch_hunk_ty *this_thing)
{
	patch_line_list_destructor(&this_thing->before);
	patch_line_list_destructor(&this_thing->after);
	mem_free(this_thing);
}
