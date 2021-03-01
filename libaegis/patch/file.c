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
 * MANIFEST: functions to manipulate files
 */

#include <mem.h>
#include <patch/file.h>


patch_ty *
patch_new()
{
	patch_ty	*this;

	this = mem_alloc(sizeof(patch_ty));
	this->action = -1;
	this->usage = -1;
	string_list_constructor(&this->name);
	patch_hunk_list_constructor(&this->actions);
	return this;
}


void
patch_delete(this)
	patch_ty	*this;
{
	string_list_destructor(&this->name);
	patch_hunk_list_destructor(&this->actions);
}


void
patch_append(this, php)
	patch_ty	*this;
	patch_hunk_ty	*php;
{
	patch_hunk_list_append(&this->actions, php);
}
