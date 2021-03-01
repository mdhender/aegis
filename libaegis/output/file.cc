//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001, 2003, 2004 Peter Miller;
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
// MANIFEST: functions to deliver output to files
//

#include <ac/errno.h>
#include <ac/fcntl.h>
#include <ac/unistd.h>

#include <glue.h>
#include <mem.h>
#include <os.h>
#include <output/file.h>
#include <output/private.h>
#include <output/stdout.h>
#include <page.h>
#include <sub.h>


struct output_file_ty
{
    output_ty       inherited;
    string_ty       *filename;
    int             fd;
    int             bol;
    size_t          pos;
};


static void
output_file_destructor(output_ty *fp)
{
    output_file_ty	*this_thing;

    this_thing = (output_file_ty *)fp;
    if (glue_close(this_thing->fd))
    {
	sub_context_ty	*scp;
	int             errno_old;

	errno_old = errno;
	scp = sub_context_new();
	sub_errno_setx(scp, errno_old);
	sub_var_set_string(scp, "File_Name", this_thing->filename);
	fatal_intl(scp, i18n("close $filename: $errno"));
	// NOTREACHED
    }
    this_thing->fd = -1;
    str_free(this_thing->filename);
    this_thing->filename = 0;
    this_thing->pos = 0;
}


static string_ty *
output_file_filename(output_ty *fp)
{
    output_file_ty  *this_thing;

    this_thing = (output_file_ty *)fp;
    return this_thing->filename;
}


static long
output_file_ftell(output_ty *fp)
{
    output_file_ty  *this_thing;

    this_thing = (output_file_ty *)fp;
    return this_thing->pos;
}


static void
output_file_write(output_ty *fp, const void *data, size_t len)
{
    output_file_ty  *this_thing;

    this_thing = (output_file_ty *)fp;
    if (glue_write(this_thing->fd, data, len) < 0)
    {
	sub_context_ty	*scp;
	int             errno_old;

	errno_old = errno;
	scp = sub_context_new();
	sub_errno_setx(scp, errno_old);
	sub_var_set_string(scp, "File_Name", this_thing->filename);
	fatal_intl(scp, i18n("write $filename: $errno"));
	// NOTREACHED
    }
    if (len > 0)
	this_thing->bol = (((char *)data)[len - 1] == '\n');
    this_thing->pos += len;
}


static int
output_file_page_width(output_ty *fp)
{
    return page_width_get(DEFAULT_PRINTER_WIDTH);
}


static int
output_file_page_length(output_ty *fp)
{
    return page_length_get(DEFAULT_PRINTER_LENGTH);
}


static void
output_file_eoln(output_ty *fp)
{
    output_file_ty	*this_thing;

    this_thing = (output_file_ty *)fp;
    if (!this_thing->bol)
	output_fputc(fp, '\n');
}



static output_vtbl_ty vtbl =
{
    sizeof(output_file_ty),
    output_file_destructor,
    output_file_filename,
    output_file_ftell,
    output_file_write,
    output_generic_flush,
    output_file_page_width,
    output_file_page_length,
    output_file_eoln,
    "file",
};


static int
open_with_stale_nfs_retry(const char *path, int mode)
{
    int             fd;
#ifdef ESTALE
    int             ntries;
    const int       nsecs = 5;
#endif

    //
    // Try to open the file.
    //
    errno = 0;
    fd = glue_open(path, mode, 0666);

    //
    // Keep trying for one minute if we get a Stale NFS file handle
    // error.  Some systems suffer from this in a Very Bad Way.
    //
#ifdef ESTALE
    for (ntries = 0; ntries < 60; ntries += nsecs)
    {
	if (fd >= 0)
	    break;
	if (errno != ESTALE)
	    break;
	sleep(nsecs);
	errno = 0;
	fd = glue_open(path, mode);
    }
#endif

    //
    // Return the result, both success and failure.
    // Errors are handled elsewhere.
    //
    return fd;
}


static output_ty *
output_file_open(string_ty *fn, int binary)
{
    output_ty       *result;
    output_file_ty  *this_thing;
    int             mode;

    if (!fn || !fn->str_length)
	return output_stdout();
    os_become_must_be_active();
    result = output_new(&vtbl);
    this_thing = (output_file_ty *)result;
    mode = O_WRONLY | O_CREAT | O_TRUNC | (binary ? O_BINARY : O_TEXT);
    this_thing->fd = open_with_stale_nfs_retry(fn->str_text, mode);
    if (this_thing->fd < 0)
    {
	sub_context_ty	*scp;
	int             errno_old;

	errno_old = errno;
	scp = sub_context_new();
	sub_errno_setx(scp, errno_old);
	sub_var_set_string(scp, "File_Name", fn);
	fatal_intl(scp, i18n("open $filename: $errno"));
	// NOTREACHED
    }
    this_thing->filename = str_copy(fn);
    this_thing->bol = 1;
    this_thing->pos = 0;
    return result;
}


output_ty *
output_file_text_open(string_ty *fn)
{
    return output_file_open(fn, 0);
}


output_ty *
output_file_binary_open(string_ty *fn)
{
    return output_file_open(fn, 1);
}
