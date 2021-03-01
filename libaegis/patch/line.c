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
 * MANIFEST: functions to manipulate lines
 */

#include <patch/line.h>


void
patch_line_constructor(this, type, value)
	patch_line_ty	*this;
	patch_line_type	type;
	string_ty	*value;
{
	this->type = type;
	this->value = str_copy(value);
}


void
patch_line_destructor(this)
	patch_line_ty	*this;
{
	str_free(this->value);
	this->type = 0;
	this->value = 0;
}
