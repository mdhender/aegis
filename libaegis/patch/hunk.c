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
 * MANIFEST: functions to manipulate hunks
 */

#include <mem.h>
#include <patch/hunk.h>


patch_hunk_ty *
patch_hunk_new()
{
	patch_hunk_ty	*this;

	this = mem_alloc(sizeof(patch_hunk_ty));
	this->inserts = 0;
	this->deletes = 0;
	patch_line_list_constructor(&this->before);
	patch_line_list_constructor(&this->after);
	return this;
}


void
patch_hunk_delete(this)
	patch_hunk_ty	*this;
{
	patch_line_list_destructor(&this->before);
	patch_line_list_destructor(&this->after);
	mem_free(this);
}
