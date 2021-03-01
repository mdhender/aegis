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

#include <ac/ctype.h>

#include <input/cpio_child.h>
#include <input/private.h>
#include <error.h> /* for assert */
#include <mem.h>
#include <str.h>
#include <stracc.h>
#include <sub.h>


typedef struct input_cpio_child_ty input_cpio_child_ty;
struct input_cpio_child_ty
{
	input_ty	inherited;
	input_ty	*deeper;
	string_ty	*archive_name;
	string_ty	*filename;
	long		pos;
	long		length;
};


static void padding _((input_cpio_child_ty *));

static void
padding(this)
	input_cpio_child_ty *this;
{
	int		n;

	n = input_ftell(this->deeper);
	while (n & 3)
	{
		int c = input_getc(this->deeper);
		if (c < 0)
			input_format_error((input_ty *)this);
		++n;
	}
}


static void destructor _((input_ty *));

static void
destructor(fp)
	input_ty	*fp;
{
	input_cpio_child_ty *this;

	/*
	 * read the rest of the input,
	 * it wasn't all used.
	 */
	this = (input_cpio_child_ty *)fp;
	while (this->pos < this->length)
	{
		if (input_getc(this->deeper) < 0)
			input_format_error(fp);
	}

	/*
	 * move to the next padding boundary
	 */
	padding(this);

	/*
	 * free the archive member name
	 */
	if (this->archive_name);
		str_free(this->archive_name);
	if (this->filename);
		str_free(this->filename);

	/* DO NOT input_delete(this->deeper); */
}


static long iread _((input_ty *, void *, long));

static long
iread(fp, data, len)
	input_ty	*fp;
	void		*data;
	long		len;
{
	input_cpio_child_ty *this;
	long		result;

	this = (input_cpio_child_ty *)fp;
	if (len <= 0)
		return 0;
	if (this->pos >= this->length)
		return 0;
	if (this->pos + len > this->length)
		len = this->length - this->pos;
	assert(len > 0);
	result = input_read(this->deeper, data, len);
	if (result <= 0)
		input_format_error(fp);
	this->pos += result;
	return result;
}


static int iget _((input_ty *));

static int
iget(fp)
	input_ty	*fp;
{
	input_cpio_child_ty *this;
	int		c;

	this = (input_cpio_child_ty *)fp;
	if (this->pos >= this->length)
		return -1;
	c = input_getc(this->deeper);
	if (c < 0)
		input_format_error(fp);
	this->pos++;
	return c;
}


static long itell _((input_ty *));

static long
itell(fp)
	input_ty	*fp;
{
	input_cpio_child_ty *this;

	this = (input_cpio_child_ty *)fp;
	return this->pos;
}


static const char *iname _((input_ty *));

static const char *
iname(fp)
	input_ty	*fp;
{
	input_cpio_child_ty *this;

	this = (input_cpio_child_ty *)fp;
	if (this->filename)
		return this->filename->str_text;
	return input_name(this->deeper);
}


static long ilength _((input_ty *));

static long
ilength(fp)
	input_ty	*fp;
{
	input_cpio_child_ty *this;

	this = (input_cpio_child_ty *)fp;
	return this->length;
}


static input_vtbl_ty vtbl =
{
	sizeof(input_cpio_child_ty),
	destructor,
	iread,
	iget,
	itell,
	iname,
	ilength,
};


static int hex_digit _((input_cpio_child_ty *, int *));

static int
hex_digit(this, first_p)
	input_cpio_child_ty *this;
	int		*first_p;
{
	int		c;
	
	c = input_getc(this->deeper);
	switch (c)
	{
	default:
		input_format_error((input_ty *)this);
		/* NOTREACHED */

	case ' ':
		if (*first_p)
			return 0;
		input_format_error((input_ty *)this);
		/* NOTREACHED */

	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		*first_p = 0;
		return (c - '0');

	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		*first_p = 0;
		return (c - 'a' + 10);

	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		*first_p = 0;
		return (c - 'A' + 10);
	}
}


static long hex8 _((input_cpio_child_ty *));

static long
hex8(this)
	input_cpio_child_ty *this;
{
	long		result;
	int		j;
	int		first;

	result = 0;
	first = 1;
	for (j = 0; j < 8; ++j)
	{
		int c = hex_digit(this, &first);
		result = (result << 4) + c;
	}
	if (first)
		input_format_error((input_ty *)this);
	return result;
}


static string_ty *get_name _((input_cpio_child_ty *, long));

static string_ty *
get_name(this, namlen)
	input_cpio_child_ty *this;
	long		namlen;
{
	static stracc_t	buffer;
	long		j;

	/*
	 * make sure out buffer is big enough.
	 */
	if (namlen < 2)
		input_format_error((input_ty *)this);
	--namlen;

	/*
	 * Read the filename, checking each character.
	 */
	stracc_open(&buffer);
	for (j = 0; j < namlen; ++j)
	{
		int c = input_getc(this->deeper);
		if (c <= 0)
			input_format_error((input_ty *)this);
		if (isspace(c))
			input_format_error((input_ty *)this);
		if (!isprint(c))
			input_format_error((input_ty *)this);
		stracc_char(&buffer, c);
	}

	/*
	 * Must has a NUL on the end.
	 */
	if (input_getc(this->deeper) != 0)
		input_format_error((input_ty *)this);

	/*
	 * Build the result and return.
	 */
	return stracc_close(&buffer);
}


input_ty *
input_cpio_child_open(deeper, archive_name_p)
	input_ty	*deeper;
	string_ty	**archive_name_p;
{
	input_ty	*result;
	input_cpio_child_ty *this;
	long		namlen;
	static string_ty *trailer;
	const char	*magic;

	result = input_new(&vtbl);
	this = (input_cpio_child_ty *)result;
	this->deeper = deeper;
	this->pos = 0;
	this->length = 0;
	this->archive_name = 0;
	this->filename = 0;

	/*
	 * read the file header
	 */
	for (magic = "070701"; *magic; ++magic)
	{
		if (input_getc(deeper) != *magic)
			input_format_error((input_ty *)this);
	}
	hex8(this);	/* inode */
	hex8(this);	/* mode */
	hex8(this);	/* uid */
	hex8(this);	/* gid */
	hex8(this);	/* nlinks */
	hex8(this);	/* mtime */
	this->length = hex8(this);
	hex8(this);	/* dev_major */
	hex8(this);	/* dev_minor */
	hex8(this);	/* rdev_major */
	hex8(this);	/* rdev_minor */
	namlen = hex8(this);
	hex8(this);	/* no checksum */
	this->archive_name = get_name(this, namlen);
	this->filename =
		 str_format("%s(%S)", input_name(deeper), this->archive_name);
	padding(this);

	/*
	 * The trailer record tells us when to stop.
	 */
	if (!trailer)
		trailer = str_from_c("TRAILER!!!");
	if (str_equal(this->archive_name, trailer))
	{
		input_delete(result);
		return 0;
	}

	/*
	 * the child will read everything.
	 */
	if (archive_name_p)
		*archive_name_p = str_copy(this->archive_name);
	return result;
}
