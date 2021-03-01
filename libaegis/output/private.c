/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2001, 2003 Peter Miller;
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
#include <output/private.h>
#include <page.h>


output_ty *
output_new(output_vtbl_ty *vptr)
{
	output_ty	*this_thing;

	assert(vptr);
	assert(vptr->size > sizeof(output_ty));
	this_thing = (output_ty *)mem_alloc(vptr->size);
	this_thing->vptr = vptr;
	this_thing->del_cb = 0;
	this_thing->del_cb_arg = 0;
	this_thing->buffer_size = (size_t)1 << 13;
	this_thing->buffer =
            (unsigned char *)mem_alloc(this_thing->buffer_size);
	this_thing->buffer_position = this_thing->buffer;
	this_thing->buffer_end = this_thing->buffer + this_thing->buffer_size;
	return this_thing;
}


void
output_generic_flush(output_ty *fp)
{
}


int
output_generic_page_width(output_ty *fp)
{
	return page_width_get(-1) - 1;
}


int
output_generic_page_length(output_ty *fp)
{
	return page_length_get(-1);
}
