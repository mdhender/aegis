/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002, 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate tar_childs
 */

#include <error.h> /* for assert */
#include <header.h>
#include <input/tar_child.h>
#include <input/private.h>
#include <mem.h>


typedef struct input_tar_child_ty input_tar_child_ty;
struct input_tar_child_ty
{
    input_ty	    inherited;
    input_ty	    *deeper;
    string_ty	    *archive_name;
    string_ty	    *filename;
    long	    pos;
    long	    length;
};


static void
padding(input_ty *ip)
{
    int		    n;

    n = input_ftell(ip);
    n %= TBLOCK;
    if (n)
	input_skip(ip, TBLOCK - n);
}


static void
input_tar_child_destructor(input_ty *fp)
{
    input_tar_child_ty *this_thing;

    /*
     * read the rest of the input,
     * it wasn't all used.
     */
    this_thing = (input_tar_child_ty *)fp;
    if (this_thing->pos < this_thing->length)
	input_skip(this_thing->deeper, this_thing->length - this_thing->pos);

    /*
     * move to the next padding boundary
     */
    padding(this_thing->deeper);

    /*
     * free the archive member name
     */
    if (this_thing->archive_name);
	str_free(this_thing->archive_name);
    if (this_thing->filename);
	str_free(this_thing->filename);

    /* DO NOT input_delete(this_thing->deeper); */
}


static long
input_tar_child_read(input_ty *fp, void *data, size_t len)
{
    input_tar_child_ty *this_thing;

    this_thing = (input_tar_child_ty *)fp;
    if (len <= 0)
	return 0;
    if (this_thing->pos >= this_thing->length)
       	return 0;
    if (this_thing->pos + (long)len > this_thing->length)
       	len = this_thing->length - this_thing->pos;
    assert(len > 0);
    input_read_strictest(this_thing->deeper, data, len);
    this_thing->pos += len;
    return len;
}


static long
input_tar_child_ftell(input_ty *fp)
{
    input_tar_child_ty *this_thing;

    this_thing = (input_tar_child_ty *)fp;
    return this_thing->pos;
}


static string_ty *
input_tar_child_name(input_ty *fp)
{
    input_tar_child_ty *this_thing;

    this_thing = (input_tar_child_ty *)fp;
    return this_thing->filename;
}


static long
input_tar_child_length(input_ty *fp)
{
    input_tar_child_ty *this_thing;

    this_thing = (input_tar_child_ty *)fp;
    return this_thing->length;
}


static input_vtbl_ty vtbl =
{
    sizeof(input_tar_child_ty),
    input_tar_child_destructor,
    input_tar_child_read,
    input_tar_child_ftell,
    input_tar_child_name,
    input_tar_child_length,
};


static string_ty *
read_data_as_string(input_ty *ip, size_t size)
{
    static char	    *buffer;
    static size_t   maximum;

    if (size > maximum)
    {
	maximum = size;
	buffer = (char *)mem_change_size(buffer, maximum);
    }
    input_read_strictest(ip, buffer, size);
    padding(ip);
    return str_n_from_c(buffer, size);
}


static int
all_zero(const char *buf, size_t len)
{
    while (len-- > 0)
    {
	if (*buf++)
	    return 0;
    }
    return 1;
}


input_ty *
input_tar_child_open(input_ty *deeper, string_ty **archive_name_p)
{
    string_ty	    *longname;

    /*
     * Wade through the garbage until we find something interesting.
     */
    longname = 0;
    for (;;)
    {
	input_ty	*result;
	input_tar_child_ty *this_thing;
	char		header[TBLOCK];
	header_ty	*hp;
	int		hchksum;
	int		cs2;
	string_ty	*name;
	long		hsize;

	/*
	 * read the file header
	 */
	if
	(
	    !input_read_strict(deeper, header, sizeof(header))
	||
	    all_zero(header, sizeof(header))
	)
	{
	    if (longname)
		str_free(longname);
	    return 0;
	}
	hp = (header_ty *)header;

	/*
	 * Verify checksum.
	 */
	hchksum = header_checksum_get(hp);
	if (hchksum < 0)
	    input_fatal_error(deeper, "corrupted checksum field");
	cs2 = header_checksum_calculate(hp);
	if (hchksum != cs2)
	{
	    string_ty	*s;

	    header_dump(hp);
	    s =
		str_format
		(
		    "checksum does not match (calculated 0%o, file has 0%o)",
		    cs2,
		    hchksum
		);
	    input_fatal_error(deeper, s->str_text);
	}

	/*
	 * The essential information we want from the header is the
	 * file's name and the file's size.  All that other guff is ignored.
	 */
	if (longname)
	{
	    name = longname;
	    longname = 0;
	}
	else
	{
	    name = header_name_get(hp);
	    if (name->str_length == 0)
		input_fatal_error(deeper, "corrupted name field");
	}
	hsize = header_size_get(hp);
	if (hsize < 0)
	    input_fatal_error(deeper, "corrupted size field");

	/*
	 * Work out that to do with it.
	 */
	switch (header_linkflag_get(hp))
	{
	case LF_OLDNORMAL:
	case LF_NORMAL:
	    if (name->str_text[name->str_length - 1] == '/')
	    {
		/*
		 * Throw directories away.  Aegis only likes real files.
		 */
		goto toss;
	    }
	    break;

	case LF_CONTIG:
	    break;

	case LF_LINK:
	case LF_SYMLINK:
	case LF_CHR:
	case LF_BLK:
	case LF_DIR:
	case LF_FIFO:
	    /*
	     * Throw these away.  Aegis only likes real files.
	     */
	    toss:
	    str_free(name);
	    continue;

	case LF_LONGNAME:
	    /*
	     * The next real file gets this_thing as its name.
	     */
	    longname = read_data_as_string(deeper, hsize);
	    continue;

	case LF_LONGLINK:
	    /*
	     * The next file gets this_thing as its link name.
	     * (But we toss links, so toss the data).
	     */
	    input_skip(deeper, hsize);
	    padding(deeper);
	    continue;

	default:
	    {
		string_ty	*s;

		s =
		    str_format
		    (
			"file type \"%c\" unknown",
			header_linkflag_get(hp)
		    );
		input_fatal_error(deeper, s->str_text);
		str_free(s);
	    }
	    goto toss;
	}

	/*
	 * Create a new input instance.
	 */
	result = input_new(&vtbl);
	this_thing = (input_tar_child_ty *)result;
	this_thing->deeper = deeper;
	this_thing->pos = 0;
	this_thing->length = hsize;
	this_thing->archive_name = name;
	this_thing->filename = str_format("%S(%S)", input_name(deeper), name);

	/*
	 * the child will read everything.
	 */
	if (archive_name_p)
	    *archive_name_p = str_copy(name);
	return result;
    }
}
