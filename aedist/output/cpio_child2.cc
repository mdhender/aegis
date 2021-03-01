//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003, 2004 Peter Miller;
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
// MANIFEST: functions to manipulate cpio_child2s
//

#include <output/cpio_child.h>
#include <output/cpio_child2.h>
#include <output/memory.h>
#include <output/private.h>
#include <str.h>


typedef struct output_cpio_child2_ty output_cpio_child2_ty;
struct output_cpio_child2_ty
{
	output_ty	inheroted;
	output_ty	*deeper;
	string_ty	*name;
	output_ty	*buffer;
};


static void
output_cpio_child2_destructor(output_ty *fp)
{
	output_cpio_child2_ty *this_thing;
	output_ty	*tmp;

	this_thing = (output_cpio_child2_ty *)fp;
	tmp =
		output_cpio_child_open
		(
			this_thing->deeper,
			this_thing->name,
			output_ftell(this_thing->buffer)
		);
	output_memory_forward(this_thing->buffer, tmp);
	output_delete(tmp);
	output_delete(this_thing->buffer);
	str_free(this_thing->name);
	//
	// DO NOT output_delete(this_thing->deeper);
	// this is output_cpio::destructor's job.
	//
}


static string_ty *
output_cpio_child2_filename(output_ty *fp)
{
	output_cpio_child2_ty *this_thing;

	this_thing = (output_cpio_child2_ty *)fp;
	return output_filename(this_thing->deeper);
}


static long
output_cpio_child2_ftell(output_ty *fp)
{
	output_cpio_child2_ty *this_thing;

	this_thing = (output_cpio_child2_ty *)fp;
	return output_ftell(this_thing->buffer);
}


static void
output_cpio_child2_write(output_ty *fp, const void *data, size_t len)
{
	output_cpio_child2_ty *this_thing;

	this_thing = (output_cpio_child2_ty *)fp;
	output_write(this_thing->buffer, data, len);
}


static void
output_cpio_child2_flush(output_ty *fp)
{
	output_cpio_child2_ty *this_thing;

	this_thing = (output_cpio_child2_ty *)fp;
	output_flush(this_thing->buffer);
}


static void
output_cpio_child2_eoln(output_ty *fp)
{
	output_cpio_child2_ty *this_thing;

	this_thing = (output_cpio_child2_ty *)fp;
	output_end_of_line(this_thing->buffer);
}



static output_vtbl_ty vtbl =
{
	sizeof(output_cpio_child2_ty),
	output_cpio_child2_destructor,
	output_cpio_child2_filename,
	output_cpio_child2_ftell,
	output_cpio_child2_write,
	output_cpio_child2_flush,
	output_generic_page_width,
	output_generic_page_length,
	output_cpio_child2_eoln,
	"cpio buffered child",
};


output_ty *
output_cpio_child2_open(output_ty *deeper, string_ty *name)
{
	output_ty	*result;
	output_cpio_child2_ty *this_thing;

	result = output_new(&vtbl);
	this_thing = (output_cpio_child2_ty *)result;
	this_thing->deeper = deeper;
	this_thing->name = str_copy(name);
	this_thing->buffer = output_memory_open();
	return result;
}
