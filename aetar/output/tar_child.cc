//
//	aegis - project change supervisor
//	Copyright (C) 2002-2004 Peter Miller;
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
// MANIFEST: functions to manipulate tar_childs
//

#include <ac/string.h>

#include <error.h>
#include <header.h>
#include <now.h>
#include <output/tar_child.h>
#include <output/private.h>
#include <str.h>
#include <sub.h>


struct output_tar_child_ty
{
    output_ty	    inherited;
    output_ty	    *deeper;
    output_ty	    *this_thing;
    string_ty	    *name;
    long	    length;
    long	    pos;
    int		    bol;
};


static void
changed_size(output_tar_child_ty *this_thing)
{
    sub_context_ty  *scp;

    scp = sub_context_new();
    sub_var_set_format
    (
	scp,
	"File_Name",
	"%s(%s)",
	output_filename(this_thing->deeper)->str_text,
	this_thing->name->str_text
    );
    fatal_intl(scp, i18n("archive member $filename changed size"));
}


static void
padding(output_tar_child_ty *this_thing)
{
    int		    n;

    n = output_ftell(this_thing->deeper) % TBLOCK;
    if (n == 0)
	return;
    while (n++ < TBLOCK)
	output_fputc(this_thing->deeper, '\n');
}


static void
header(output_tar_child_ty *this_thing, int executable)
{
    char	    buffer[TBLOCK];
    header_ty	    *hp;
    static string_ty *root;

    if (!root)
	root = str_from_c("root");

    //
    // Long names get special treatment.
    //
    if (this_thing->name->str_length >= sizeof(hp->name))
    {
	memset(buffer, 0, sizeof(buffer));
	hp = (header_ty *)buffer;
	strcpy(hp->name, "././@LongLink");
	header_size_set(hp, this_thing->name->str_length + 1);
	header_mode_set(hp, 0);
	header_uid_set(hp, 0);
	header_uname_set(hp, root);
	header_gid_set(hp, 0);
	header_gname_set(hp, root);
	header_mtime_set(hp, 0);
	header_linkflag_set(hp, LF_LONGNAME);
	header_checksum_set(hp, header_checksum_calculate(hp));
	output_write(this_thing->deeper, buffer, TBLOCK);

	//
	// This write, and the leangth in the header, include the
	// terminating NUL on the end of the file name.
       	//
	output_write
	(
	    this_thing->deeper,
	    this_thing->name->str_text,
	    this_thing->name->str_length + 1
	);
	padding(this_thing);
    }

    memset(buffer, 0, sizeof(buffer));
    hp = (header_ty *)buffer;
    header_name_set(hp, this_thing->name);
    header_size_set(hp, this_thing->length);
    header_mode_set(hp, 0100644 | (executable ? 0111 : 0));
    header_uid_set(hp, 0);
    header_uname_set(hp, root);
    header_gid_set(hp, 0);
    header_gname_set(hp, root);
    header_mtime_set(hp, now());
    header_linkflag_set(hp, LF_NORMAL);
    header_checksum_set(hp, header_checksum_calculate(hp));
    output_write(this_thing->deeper, buffer, TBLOCK);
}


static void
output_tar_child_destructor(output_ty *fp)
{
    output_tar_child_ty *this_thing;

    this_thing = (output_tar_child_ty *)fp;
    if (this_thing->pos != this_thing->length)
    {
	changed_size(this_thing);
    }
    padding(this_thing);
    str_free(this_thing->name);
    //
    // DO NOT output_delete(this_thing->deeper);
    // this is output_tar::destructor's job.
    //
}


static string_ty *
output_tar_child_filename(output_ty *fp)
{
    output_tar_child_ty *this_thing;

    this_thing = (output_tar_child_ty *)fp;
    return output_filename(this_thing->deeper);
}


static long
output_tar_child_ftell(output_ty *fp)
{
    output_tar_child_ty *this_thing;

    this_thing = (output_tar_child_ty *)fp;
    return this_thing->pos;
}


static void
output_tar_child_write(output_ty *fp, const void *data, size_t len)
{
    output_tar_child_ty *this_thing;

    this_thing = (output_tar_child_ty *)fp;
    output_write(this_thing->deeper, data, len);
    this_thing->pos += len;
    if (len > 0)
	this_thing->bol = (((const char *)data)[len - 1] == '\n');
}


static void
output_tar_child_eoln(output_ty *fp)
{
    output_tar_child_ty *this_thing;

    this_thing = (output_tar_child_ty *)fp;
    if (!this_thing->bol)
	output_fputc(fp, '\n');
}


static output_vtbl_ty vtbl =
{
    sizeof(output_tar_child_ty),
    output_tar_child_destructor,
    output_tar_child_filename,
    output_tar_child_ftell,
    output_tar_child_write,
    output_generic_flush,
    output_generic_page_width,
    output_generic_page_length,
    output_tar_child_eoln,
    "tar child",
};


output_ty *
output_tar_child_open(output_ty *deeper, string_ty *name, long length,
    int executable)
{
    output_ty       *result;
    output_tar_child_ty *this_thing;

    // assert(length >= 0);
    result = output_new(&vtbl);
    this_thing = (output_tar_child_ty *)result;
    this_thing->deeper = deeper;
    this_thing->name = str_copy(name);
    this_thing->length = length;
    this_thing->pos = 0;
    this_thing->bol = 1;
    header(this_thing, executable);
    return result;
}
