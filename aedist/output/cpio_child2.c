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
 * MANIFEST: functions to manipulate cpio_child2s
 */

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


static void destructor _((output_ty *));

static void
destructor(fp)
	output_ty	*fp;
{
	output_cpio_child2_ty *this;
	output_ty	*tmp;

	this = (output_cpio_child2_ty *)fp;
	tmp =
		output_cpio_child_open
		(
			this->deeper,
			this->name,
			output_ftell(this->buffer)
		);
	output_memory_forward(this->buffer, tmp);
	output_delete(tmp);
	output_delete(this->buffer);
	str_free(this->name);
	/*
	 * DO NOT output_delete(this->deeper);
	 * this is output_cpio::destructor's job.
	 */
}


static const char *filename _((output_ty *));

static const char *
filename(fp)
	output_ty	*fp;
{
	output_cpio_child2_ty *this;

	this = (output_cpio_child2_ty *)fp;
	return output_filename(this->deeper);
}


static long otell _((output_ty *));

static long
otell(fp)
	output_ty	*fp;
{
	output_cpio_child2_ty *this;

	this = (output_cpio_child2_ty *)fp;
	return output_ftell(this->buffer);
}


static void oputc _((output_ty *, int));

static void
oputc(fp, c)
	output_ty	*fp;
	int		c;
{
	output_cpio_child2_ty *this;

	this = (output_cpio_child2_ty *)fp;
	output_fputc(this->buffer, c);
}


static void owrite _((output_ty *, const void *, size_t));

static void
owrite(fp, data, len)
	output_ty	*fp;
	const void	*data;
	size_t		len;
{
	output_cpio_child2_ty *this;

	this = (output_cpio_child2_ty *)fp;
	output_write(this->buffer, data, len);
}



static output_vtbl_ty vtbl =
{
	sizeof(output_cpio_child2_ty),
	"cpio buffered child",
	destructor,
	filename,
	otell,
	oputc,
	output_generic_fputs,
	owrite,
};


output_ty *
output_cpio_child2_open(deeper, name)
	output_ty	*deeper;
	string_ty	*name;
{
	output_ty	*result;
	output_cpio_child2_ty *this;

	result = output_new(&vtbl);
	this = (output_cpio_child2_ty *)result;
	this->deeper = deeper;
	this->name = str_copy(name);
	this->buffer = output_memory_open();
	return result;
}
