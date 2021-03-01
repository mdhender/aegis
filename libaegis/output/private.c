/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2001 Peter Miller;
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
 * MANIFEST: functions to perform common output functions
 */

#include <ac/string.h>

#include <error.h> /* for assert */
#include <mem.h>
#include <option.h>
#include <output/private.h>


output_ty *
output_new(vptr)
	output_vtbl_ty	*vptr;
{
	output_ty	*this;

	assert(vptr);
	assert(vptr->size > sizeof(output_ty));
	this = mem_alloc(vptr->size);
	this->vptr = vptr;
	this->del_cb = 0;
	this->del_cb_arg = 0;
	this->buffer_size = (size_t)1 << 13;
	this->buffer = mem_alloc(this->buffer_size);
	this->buffer_position = this->buffer;
	this->buffer_end = this->buffer + this->buffer_size;
	return this;
}


void
output_generic_flush(fp)
	output_ty	*fp;
{
}


int
output_generic_page_width(fp)
	output_ty	*fp;
{
	return option_page_width_get(-1) - 1;
}


int
output_generic_page_length(fp)
	output_ty	*fp;
{
	return option_page_length_get(-1);
}
