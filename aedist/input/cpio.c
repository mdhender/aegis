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


static void destructor _((input_ty *));

static void
destructor(fp)
	input_ty	*fp;
{
	input_cpio_ty	*this;

	this = (input_cpio_ty *)fp;
	input_delete(this->deeper);
}


static long iread _((input_ty *, void *, long));

static long
iread(fp, data, len)
	input_ty	*fp;
	void		*data;
	long		len;
{
	assert(0);
	return -1;
}


static int iget _((input_ty *));

static int
iget(fp)
	input_ty	*fp;
{
	assert(0);
	return -1;
}


static long itell _((input_ty *));

static long
itell(fp)
	input_ty	*fp;
{
	assert(0);
	return 0;
}


static const char *iname _((input_ty *));

static const char *
iname(fp)
	input_ty	*fp;
{
	input_cpio_ty	*this;

	this = (input_cpio_ty *)fp;
	return input_name(this->deeper);
}


static long ilength _((input_ty *));

static long
ilength(fp)
	input_ty	*fp;
{
	input_cpio_ty	*this;

	this = (input_cpio_ty *)fp;
	return input_length(this->deeper);
}


static input_vtbl_ty vtbl =
{
	sizeof(input_cpio_ty),
	destructor,
	iread,
	iget,
	itell,
	iname,
	ilength,
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
