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
 * MANIFEST: functions to manipulate memorys
 */

#include <mem.h>
#include <output/memory.h>
#include <output/private.h>


typedef struct output_memory_ty output_memory_ty;
struct output_memory_ty
{
	output_ty	inherited;
	unsigned char	*buffer;
	size_t		size;
	size_t		maximum;
};


static void destructor _((output_ty *));

static void
destructor(fp)
	output_ty	*fp;
{
	output_memory_ty *this;

	this = (output_memory_ty *)fp;
	if (this->buffer)
		mem_free(this->buffer);
}


static const char *filename _((output_ty *));

static const char *
filename(fp)
	output_ty	*fp;
{
	return "memory";
}


static long otell _((output_ty *));

static long
otell(fp)
	output_ty	*fp;
{
	output_memory_ty *this;

	this = (output_memory_ty *)fp;
	return this->size;
}


static void oputc _((output_ty *, int));

static void
oputc(fp, c)
	output_ty	*fp;
	int		c;
{
	output_memory_ty *this;

	this = (output_memory_ty *)fp;
	while (this->size >= this->maximum)
	{
		this->maximum = 2 * this->maximum + 32;
		this->buffer = mem_change_size(this->buffer, this->maximum);
	}
	this->buffer[this->size++] = c;
}


static void owrite _((output_ty *, const void *, size_t));

static void
owrite(fp, data, len)
	output_ty	*fp;
	const void	*data;
	size_t		len;
{
	output_memory_ty *this;

	this = (output_memory_ty *)fp;
	while (this->size + len > this->maximum)
	{
		this->maximum = 2 * this->maximum + 32;
		this->buffer = mem_change_size(this->buffer, this->maximum);
	}
	memcpy(this->buffer + this->size, data, len);
	this->size += len;
}


static output_vtbl_ty vtbl =
{
	sizeof(output_memory_ty),
	"memory",
	destructor,
	filename,
	otell,
	oputc,
	output_generic_fputs,
	owrite,
};


output_ty *
output_memory_open()
{
	output_ty	*result;
	output_memory_ty *this;

	result = output_new(&vtbl);
	this = (output_memory_ty *)result;
	this->buffer = 0;
	this->size = 0;
	this->maximum = 0;
	return result;
}


void
output_memory_forward(fp, deeper)
	output_ty	*fp;
	output_ty	*deeper;
{
	output_memory_ty *this;

	if (fp->vptr != &vtbl)
		return;
	this = (output_memory_ty *)fp;
	output_write(deeper, this->buffer, this->size);
}
