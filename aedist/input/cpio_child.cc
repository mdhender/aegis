//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2004 Peter Miller;
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
// MANIFEST: functions to manipulate cpio_childs
//

#include <ac/ctype.h>

#include <input/cpio_child.h>
#include <input/private.h>
#include <error.h> // for assert
#include <mem.h>
#include <str.h>
#include <stracc.h>
#include <sub.h>


struct input_cpio_child_ty
{
    input_ty	    inherited;
    input_ty	    *deeper;
    string_ty	    *archive_name;
    string_ty	    *filename;
    long	    pos;
    long	    length;
};


static void
padding(input_cpio_child_ty *this_thing)
{
    int		    n;

    n = input_ftell(this_thing->deeper);
    n %= 4;
    if (n)
	input_skip(this_thing->deeper, 4 - n);
}


static void
input_cpio_child_destructor(input_ty *fp)
{
    input_cpio_child_ty *this_thing;

    //
    // read the rest of the input,
    // it wasn't all used.
    //
    this_thing = (input_cpio_child_ty *)fp;
    if (this_thing->pos < this_thing->length)
	input_skip(fp, this_thing->length - this_thing->pos);

    //
    // move to the next padding boundary
    //
    padding(this_thing);

    //
    // free the archive member name
    //
    if (this_thing->archive_name);
	str_free(this_thing->archive_name);
    if (this_thing->filename);
	str_free(this_thing->filename);

    // DO NOT input_delete(this_thing->deeper);
}


static long
input_cpio_child_read(input_ty *fp, void *data, size_t len)
{
    input_cpio_child_ty *this_thing;
    long	    result;

    this_thing = (input_cpio_child_ty *)fp;
    if (len <= 0)
	return 0;
    if (this_thing->pos >= this_thing->length)
	return 0;
    if (this_thing->pos + (long)len > this_thing->length)
	len = this_thing->length - this_thing->pos;
    assert(len > 0);
    result = input_read(this_thing->deeper, data, len);
    if (result <= 0)
	input_fatal_error(fp, "cpio: short file");
    this_thing->pos += result;
    return result;
}


static long
input_cpio_child_ftell(input_ty *fp)
{
    input_cpio_child_ty *this_thing;

    this_thing = (input_cpio_child_ty *)fp;
    return this_thing->pos;
}


static string_ty *
input_cpio_child_name(input_ty *fp)
{
    input_cpio_child_ty *this_thing;

    this_thing = (input_cpio_child_ty *)fp;
    if (this_thing->filename)
	return this_thing->filename;
    return input_name(this_thing->deeper);
}


static long
input_cpio_child_length(input_ty *fp)
{
    input_cpio_child_ty *this_thing;

    this_thing = (input_cpio_child_ty *)fp;
    return this_thing->length;
}


static input_vtbl_ty vtbl =
{
    sizeof(input_cpio_child_ty),
    input_cpio_child_destructor,
    input_cpio_child_read,
    input_cpio_child_ftell,
    input_cpio_child_name,
    input_cpio_child_length,
};


static int
hex_digit(input_cpio_child_ty *this_thing, int *first_p)
{
    int		    c;

    c = input_getc(this_thing->deeper);
    switch (c)
    {
    default:
	input_fatal_error((input_ty *)this_thing, "cpio: invalid hex digit");
	// NOTREACHED

    case ' ':
	if (*first_p)
    	    return 0;
	input_fatal_error((input_ty *)this_thing, "cpio: invalid hex number");
	// NOTREACHED

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


static long
hex8(input_cpio_child_ty *this_thing)
{
    long	    result;
    int		    j;
    int		    first;

    result = 0;
    first = 1;
    for (j = 0; j < 8; ++j)
    {
	int c = hex_digit(this_thing, &first);
	result = (result << 4) + c;
    }
    if (first)
	input_fatal_error((input_ty *)this_thing, "cpio: invalid hex number");
    return result;
}


static string_ty *
get_name(input_cpio_child_ty *this_thing, long namlen)
{
    static stracc_t buffer;
    long	    j;

    //
    // make sure out buffer is big enough.
    //
    if (namlen < 2)
	input_fatal_error((input_ty *)this_thing, "cpio: invalid name length");
    --namlen;

    //
    // Read the filename, checking each character.
    //
    buffer.clear();
    for (j = 0; j < namlen; ++j)
    {
	int c = input_getc(this_thing->deeper);
	if (c <= 0)
	    input_fatal_error((input_ty *)this_thing, "cpio: short file");
	if (isspace((unsigned char)c))
	{
	    input_fatal_error
	    (
		(input_ty *)this_thing,
		"cpio: invalid name (white space)"
	    );
	}
	if (!isprint((unsigned char)c))
	{
	    input_fatal_error
	    (
		(input_ty *)this_thing,
		"cpio: invalid name (unprintable)"
	    );
	}
	buffer.push_back(c);
    }

    //
    // Must has a NUL on the end.
    //
    if (input_getc(this_thing->deeper) != 0)
	input_fatal_error((input_ty *)this_thing, "cpio: invalid character");

    //
    // Build the result and return.
    //
    return buffer.mkstr();
}


input_ty *
input_cpio_child_open(input_ty *deeper, string_ty **archive_name_p)
{
    input_ty	    *result;
    input_cpio_child_ty *this_thing;
    long	    namlen;
    static string_ty *trailer;
    const char	    *magic;

    result = input_new(&vtbl);
    this_thing = (input_cpio_child_ty *)result;
    this_thing->deeper = deeper;
    this_thing->pos = 0;
    this_thing->length = 0;
    this_thing->archive_name = 0;
    this_thing->filename = 0;

    //
    // read the file header
    //
    for (magic = "070701"; *magic; ++magic)
    {
	int c = input_getc(deeper);
	if (c != *magic)
    	    input_fatal_error((input_ty *)this_thing,
                              "cpio: wrong magic number");
    }
    hex8(this_thing);	// inode
    hex8(this_thing);	// mode
    hex8(this_thing);	// uid
    hex8(this_thing);	// gid
    hex8(this_thing);	// nlinks
    hex8(this_thing);	// mtime
    this_thing->length = hex8(this_thing);
    hex8(this_thing);	// dev_major
    hex8(this_thing);	// dev_minor
    hex8(this_thing);	// rdev_major
    hex8(this_thing);	// rdev_minor
    namlen = hex8(this_thing);
    hex8(this_thing);	// no checksum
    this_thing->archive_name = get_name(this_thing, namlen);
    this_thing->filename =
	str_format
	(
	    "%s(%s)",
	    input_name(deeper)->str_text,
	    this_thing->archive_name->str_text
	);
    padding(this_thing);

    //
    // The trailer record tells us when to stop.
    //
    if (!trailer)
	trailer = str_from_c("TRAILER!!!");
    if (str_equal(this_thing->archive_name, trailer))
    {
	input_delete(result);
	return 0;
    }

    //
    // the child will read everything.
    //
    if (archive_name_p)
	*archive_name_p = str_copy(this_thing->archive_name);
    return result;
}
