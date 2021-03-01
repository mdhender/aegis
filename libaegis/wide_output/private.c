/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999-2001 Peter Miller;
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
 * MANIFEST: functions to manipulate privates
 */

#include <error.h> /* for assert */
#include <mem.h>
#include <option.h>
#include <trace.h>
#include <wide_output/private.h>


wide_output_ty *
wide_output_new(vptr)
	wide_output_vtbl_ty	*vptr;
{
	wide_output_ty	*this;

	trace(("wide_output_new(vptr = %08lX)\n{\n", (long)vptr));
	trace(("type is \"%s\"\n", vptr->typename));
	assert(vptr);
	assert(vptr->size > sizeof(wide_output_ty));
	this = mem_alloc(vptr->size);
	this->vptr = vptr;

	this->buffer_size = (size_t)1 << 11;
	this->buffer = mem_alloc(this->buffer_size * sizeof(wchar_t));
	this->buffer_position = this->buffer;
	this->buffer_end = this->buffer + this->buffer_size;

	this->ncallbacks = 0;
	this->ncallbacks_max = 0;
	this->callback = 0;
	trace(("return %08lX;", (long)this));
	trace(("}\n"));
	return this;
}


int
wide_output_generic_page_width(fp)
	wide_output_ty	*fp;
{
	return option_page_width_get(-1) - 1;
}


int
wide_output_generic_page_length(fp)
	wide_output_ty	*fp;
{
	return option_page_length_get(-1);
}
