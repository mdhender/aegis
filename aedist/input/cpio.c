/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate cpios
 */

#include <error.h> /* for assert */
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


static void input_cpio_destructor _((input_ty *));

static void
input_cpio_destructor(fp)
	input_ty	*fp;
{
	input_cpio_ty	*this;

	this = (input_cpio_ty *)fp;
	input_delete(this->deeper);
}


static long input_cpio_read _((input_ty *, void *, size_t));

static long
input_cpio_read(fp, data, len)
	input_ty	*fp;
	void		*data;
	size_t		len;
{
	assert(0);
	return -1;
}


static long input_cpio_ftell _((input_ty *));

static long
input_cpio_ftell(fp)
	input_ty	*fp;
{
	assert(0);
	return 0;
}


static string_ty *input_cpio_name _((input_ty *));

static string_ty *
input_cpio_name(fp)
	input_ty	*fp;
{
	input_cpio_ty	*this;

	this = (input_cpio_ty *)fp;
	return input_name(this->deeper);
}


static long input_cpio_length _((input_ty *));

static long
input_cpio_length(fp)
	input_ty	*fp;
{
	input_cpio_ty	*this;

	this = (input_cpio_ty *)fp;
	return input_length(this->deeper);
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
input_cpio(deeper)
	input_ty	*deeper;
{
	input_ty	*result;
	input_cpio_ty	*this;

	result = input_new(&vtbl);
	this = (input_cpio_ty *)result;
	this->deeper = deeper;
	return result;
}


input_ty *
input_cpio_child(fp, archive_name_p)
	input_ty	*fp;
	string_ty	**archive_name_p;
{
	input_cpio_ty	*this;

	assert(archive_name_p);
	if (fp->vptr != &vtbl)
		return 0;
	this = (input_cpio_ty *)fp;
	return input_cpio_child_open(this->deeper, archive_name_p);
}
