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
 * MANIFEST: functions to output to cpio archivess
 */

#include <error.h>
#include <output/cpio.h>
#include <output/cpio_child.h>
#include <output/cpio_child2.h>
#include <output/private.h>
#include <str.h>


typedef struct output_cpio_ty output_cpio_ty;
struct output_cpio_ty
{
	output_ty	inherited;
	output_ty	*deeper;
};


static void output_cpio_destructor _((output_ty *));

static void
output_cpio_destructor(fp)
	output_ty	*fp;
{
	output_cpio_ty	*this;
	string_ty	*trailer;

	/*
	 * Emit the archive trailer.
	 * (An empty file with a magic name.)
	 */
	this = (output_cpio_ty *)fp;
	trailer = str_from_c("TRAILER!!!");
	output_delete(output_cpio_child_open(this->deeper, trailer, 0));
	str_free(trailer);

	/*
	 * Finish writing the archive file.
	 */
	output_delete(this->deeper);
}


static string_ty *output_cpio_filename _((output_ty *));

static string_ty *
output_cpio_filename(fp)
	output_ty	*fp;
{
	output_cpio_ty	*this;

	this = (output_cpio_ty *)fp;
	return output_filename(this->deeper);
}


static long output_cpio_ftell _((output_ty *));

static long
output_cpio_ftell(fp)
	output_ty	*fp;
{
	return 0;
}


static void output_cpio_write _((output_ty *, const void *, size_t));

static void
output_cpio_write(fp, data, len)
	output_ty	*fp;
	const void	*data;
	size_t		len;
{
	this_is_a_bug();
}


static void output_cpio_eoln _((output_ty *));

static void
output_cpio_eoln(fp)
	output_ty	*fp;
{
	this_is_a_bug();
}


static output_vtbl_ty vtbl =
{
	sizeof(output_cpio_ty),
	output_cpio_destructor,
	output_cpio_filename,
	output_cpio_ftell,
	output_cpio_write,
	output_generic_flush,
	output_generic_page_width,
	output_generic_page_length,
	output_cpio_eoln,
	"cpio archive",
};


output_ty *
output_cpio(deeper)
	output_ty	*deeper;
{
	output_ty	*result;
	output_cpio_ty	*this;

	result = output_new(&vtbl);
	this = (output_cpio_ty *)result;
	this->deeper = deeper;
	return result;
}


output_ty *
output_cpio_child(fp, name, len)
	output_ty	*fp;
	string_ty	*name;
	long		len;
{
	output_cpio_ty	*this;

	if (fp->vptr != &vtbl)
		this_is_a_bug();
	this = (output_cpio_ty *)fp;
	if (len < 0)
		return output_cpio_child2_open(this->deeper, name);
	return output_cpio_child_open(this->deeper, name, len);
}
