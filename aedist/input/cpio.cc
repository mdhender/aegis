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
// MANIFEST: functions to manipulate cpios
//

#include <error.h> // for assert
#include <input/cpio.h>
#include <input/cpio_child.h>
#include <input/private.h>
#include <str.h>


typedef struct input_cpio_ty input_cpio_ty;
struct input_cpio_ty
{
	input_ty	inherited;
	input_ty	*deeper;
};


static void
input_cpio_destructor(input_ty *fp)
{
	input_cpio_ty	*this_thing;

	this_thing = (input_cpio_ty *)fp;
	input_delete(this_thing->deeper);
}


static long
input_cpio_read(input_ty *fp, void *data, size_t len)
{
	assert(0);
	return -1;
}


static long
input_cpio_ftell(input_ty *fp)
{
	assert(0);
	return 0;
}


static string_ty *
input_cpio_name(input_ty *fp)
{
	input_cpio_ty	*this_thing;

	this_thing = (input_cpio_ty *)fp;
	return input_name(this_thing->deeper);
}


static long
input_cpio_length(input_ty *fp)
{
	input_cpio_ty	*this_thing;

	this_thing = (input_cpio_ty *)fp;
	return input_length(this_thing->deeper);
}


static input_vtbl_ty vtbl =
{
	sizeof(input_cpio_ty),
	input_cpio_destructor,
	input_cpio_read,
	input_cpio_ftell,
	input_cpio_name,
	input_cpio_length,
};


input_ty *
input_cpio(input_ty *deeper)
{
	input_ty	*result;
	input_cpio_ty	*this_thing;

	result = input_new(&vtbl);
	this_thing = (input_cpio_ty *)result;
	this_thing->deeper = deeper;
	return result;
}


input_ty *
input_cpio_child(input_ty *fp, string_ty **archive_name_p)
{
	input_cpio_ty	*this_thing;

	assert(archive_name_p);
	if (fp->vptr != &vtbl)
		return 0;
	this_thing = (input_cpio_ty *)fp;
	return input_cpio_child_open(this_thing->deeper, archive_name_p);
}
