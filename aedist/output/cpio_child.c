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
 * MANIFEST: functions to manipulate cpio_childs
 */

#include <output/cpio_child.h>
#include <output/private.h>
#include <str.h>
#include <sub.h>


typedef struct output_cpio_child_ty output_cpio_child_ty;
struct output_cpio_child_ty
{
	output_ty	inheroted;
	output_ty	*deeper;
	output_ty	*this;
	string_ty	*name;
	long		length;
	long		pos;
};


static void changed_size _((output_cpio_child_ty *));

static void
changed_size(this)
	output_cpio_child_ty *this;
{
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set
	(
		scp,
		"File_Name",
		"%s(%S)",
		output_filename(this->deeper),
		this->name
	);
	fatal_intl(scp, i18n("archive member $filename changed size"));
}


static void padding _((output_cpio_child_ty *));

static void
padding(this)
	output_cpio_child_ty *this;
{
	int		n;

	n = output_ftell(this->deeper) % 4;
	if (n == 0)
		return;
	while (n++ < 4)
		output_fputc(this->deeper, '\n');
}


static void output_hex8 _((output_ty *, long));

static void
output_hex8(fp, n)
	output_ty	*fp;
	long		n;
{
	output_fprintf(fp, "%08lx", n);
}


static void header _((output_cpio_child_ty *));

static void
header(this)
	output_cpio_child_ty *this;
{
	static string_ty *trailer;
	static int	ino;
	int		inode;

	if (!trailer)
		trailer = str_from_c("TRAILER!!!");
	if (str_equal(trailer, this->name))
		inode = 0;
	else
		inode = ++ino;

	output_fputs(this->deeper, "070701");	/* magic number */
	output_hex8(this->deeper, inode);	/* inode */
	output_hex8(this->deeper, 0100644);	/* mode */
	output_hex8(this->deeper, 0);		/* uid */
	output_hex8(this->deeper, 0);		/* gid */
	output_hex8(this->deeper, 1);		/* nlinks */
	output_hex8(this->deeper, 0);		/* mtime */
	output_hex8(this->deeper, this->length); /* size */
	output_hex8(this->deeper, 0);		/* dev_major */
	output_hex8(this->deeper, 0);		/* dev_minor */
	output_hex8(this->deeper, 0);		/* rdev_major */
	output_hex8(this->deeper, 0);		/* rdev_minor */
	output_hex8(this->deeper, this->name->str_length + 1);
	output_hex8(this->deeper, 0);		/* no checksum */
	output_write(this->deeper, this->name->str_text, this->name->str_length + 1);
	padding(this);
}


static void destructor _((output_ty *));

static void
destructor(fp)
	output_ty	*fp;
{
	output_cpio_child_ty *this;

	this = (output_cpio_child_ty *)fp;
	if (this->pos != this->length)
		changed_size(this);
	padding(this);
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
	output_cpio_child_ty *this;

	this = (output_cpio_child_ty *)fp;
	return output_filename(this->deeper);
}


static long otell _((output_ty *));

static long
otell(fp)
	output_ty	*fp;
{
	output_cpio_child_ty *this;

	this = (output_cpio_child_ty *)fp;
	return this->pos;
}


static void oputc _((output_ty *, int));

static void
oputc(fp, c)
	output_ty	*fp;
	int		c;
{
	output_cpio_child_ty *this;

	this = (output_cpio_child_ty *)fp;
	output_fputc(this->deeper, c);
	this->pos++;
}


static void owrite _((output_ty *, const void *, size_t));

static void
owrite(fp, data, len)
	output_ty	*fp;
	const void	*data;
	size_t		len;
{
	output_cpio_child_ty *this;

	this = (output_cpio_child_ty *)fp;
	output_write(this->deeper, data, len);
	this->pos += len;
}


static output_vtbl_ty vtbl =
{
	sizeof(output_cpio_child_ty),
	"cpio child",
	destructor,
	filename,
	otell,
	oputc,
	output_generic_fputs,
	owrite,
};


output_ty *
output_cpio_child_open(deeper, name, length)
	output_ty	*deeper;
	string_ty	*name;
	long		length;
{
	output_ty	*result;
	output_cpio_child_ty *this;

	/* assert(length >= 0); */
	result = output_new(&vtbl);
	this = (output_cpio_child_ty *)result;
	this->deeper = deeper;
	this->name = str_copy(name);
	this->length = length;
	this->pos = 0;
	header(this);
	return result;
}
