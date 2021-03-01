/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2002, 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate cpio_childs
 */

#include <error.h>
#include <output/cpio_child.h>
#include <output/private.h>
#include <str.h>
#include <sub.h>


typedef struct output_cpio_child_ty output_cpio_child_ty;
struct output_cpio_child_ty
{
	output_ty	inheroted;
	output_ty	*deeper;
	output_ty	*this_thing;
	string_ty	*name;
	long		length;
	long		pos;
	int		bol;
};


static void
changed_size(output_cpio_child_ty *this_thing)
{
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_format
	(
		scp,
		"File_Name",
		"%S(%S)",
		output_filename(this_thing->deeper),
		this_thing->name
	);
	fatal_intl(scp, i18n("archive member $filename changed size"));
}


static void
padding(output_cpio_child_ty *this_thing)
{
	int		n;

	n = output_ftell(this_thing->deeper) % 4;
	if (n == 0)
		return;
	while (n++ < 4)
		output_fputc(this_thing->deeper, '\n');
}


static void
output_hex8(output_ty *fp, long n)
{
	output_fprintf(fp, "%08lx", n);
}


static void
header(output_cpio_child_ty *this_thing)
{
	static string_ty *trailer;
	static int	ino;
	int		inode;

	if (!trailer)
		trailer = str_from_c("TRAILER!!!");
	if (str_equal(trailer, this_thing->name))
		inode = 0;
	else
		inode = ++ino;

	output_fputs(this_thing->deeper, "070701");	/* magic number */
	output_hex8(this_thing->deeper, inode);	/* inode */
	output_hex8(this_thing->deeper, 0100644);	/* mode */
	output_hex8(this_thing->deeper, 0);		/* uid */
	output_hex8(this_thing->deeper, 0);		/* gid */
	output_hex8(this_thing->deeper, 1);		/* nlinks */
	output_hex8(this_thing->deeper, 0);		/* mtime */
	output_hex8(this_thing->deeper, this_thing->length); /* size */
	output_hex8(this_thing->deeper, 0);		/* dev_major */
	output_hex8(this_thing->deeper, 0);		/* dev_minor */
	output_hex8(this_thing->deeper, 0);		/* rdev_major */
	output_hex8(this_thing->deeper, 0);		/* rdev_minor */
	output_hex8(this_thing->deeper, this_thing->name->str_length + 1);
	output_hex8(this_thing->deeper, 0);		/* no checksum */
	output_write
	(
	    this_thing->deeper,
	    this_thing->name->str_text, this_thing->name->str_length + 1
	);
	padding(this_thing);
}


static void
output_cpio_child_destructor(output_ty *fp)
{
	output_cpio_child_ty *this_thing;

	this_thing = (output_cpio_child_ty *)fp;
	if (this_thing->pos != this_thing->length)
	{
error_raw("%s: %d: this_thing->pos = %ld", __FILE__, __LINE__,
          this_thing->pos);
error_raw("%s: %d: this_thing->length = %ld", __FILE__, __LINE__,
          this_thing->length);
		changed_size(this_thing);
	}
	padding(this_thing);
	str_free(this_thing->name);
	/*
	 * DO NOT output_delete(this_thing->deeper);
	 * this is output_cpio::destructor's job.
	 */
}


static string_ty *
output_cpio_child_filename(output_ty *fp)
{
	output_cpio_child_ty *this_thing;

	this_thing = (output_cpio_child_ty *)fp;
	return output_filename(this_thing->deeper);
}


static long
output_cpio_child_ftell(output_ty *fp)
{
	output_cpio_child_ty *this_thing;

	this_thing = (output_cpio_child_ty *)fp;
	return this_thing->pos;
}


static void
output_cpio_child_write(output_ty *fp, const void *data, size_t len)
{
	output_cpio_child_ty *this_thing;

	this_thing = (output_cpio_child_ty *)fp;
	output_write(this_thing->deeper, data, len);
	this_thing->pos += len;
	if (len > 0)
		this_thing->bol = (((const char *)data)[len - 1] == '\n');
}


static void
output_cpio_child_eoln(output_ty *fp)
{
	output_cpio_child_ty *this_thing;

	this_thing = (output_cpio_child_ty *)fp;
	if (!this_thing->bol)
		output_fputc(fp, '\n');
}


static output_vtbl_ty vtbl =
{
	sizeof(output_cpio_child_ty),
	output_cpio_child_destructor,
	output_cpio_child_filename,
	output_cpio_child_ftell,
	output_cpio_child_write,
	output_generic_flush,
	output_generic_page_width,
	output_generic_page_length,
	output_cpio_child_eoln,
	"cpio child",
};


output_ty *
output_cpio_child_open(output_ty *deeper, string_ty *name, long length)
{
	output_ty	*result;
	output_cpio_child_ty *this_thing;

	/* assert(length >= 0); */
	result = output_new(&vtbl);
	this_thing = (output_cpio_child_ty *)result;
	this_thing->deeper = deeper;
	this_thing->name = str_copy(name);
	this_thing->length = length;
	this_thing->pos = 0;
	this_thing->bol = 1;
	header(this_thing);
	return result;
}
