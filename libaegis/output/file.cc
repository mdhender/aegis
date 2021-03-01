//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001, 2003-2005 Peter Miller;
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
#include <output/stdout.h>
#include <page.h>
#include <sub.h>


output_file::~output_file()
{
    //
    // Make sure all buffered data has been passed to our write_inner
    // method.
    //
    flush();

    if (glue_close(fd))
    {
	int errno_old = errno;
	sub_context_ty sc;
	sc.errno_setx(errno_old);
	sc.var_set_string("File_Name", file_name);
	sc.fatal_intl(i18n("close $filename: $errno"));
	// NOTREACHED
    }
    fd = -1;
    pos = 0;
}


string_ty *
output_file::filename()
    const
{
    return file_name.get_ref();
}


long
output_file::ftell_inner()
    const
{
    return pos;
}


void
output_file::write_inner(const void *data, size_t len)
{
    if (glue_write(fd, data, len) < 0)
    {
	int errno_old = errno;
	sub_context_ty sc;
	sc.errno_setx(errno_old);
	sc.var_set_string("File_Name", file_name);
	sc.fatal_intl(i18n("write $filename: $errno"));
	// NOTREACHED
    }
    if (len > 0)
	bol = (((char *)data)[len - 1] == '\n');
    pos += len;
}


int
output_file::page_width()
    const
{
    return page_width_get(DEFAULT_PRINTER_WIDTH);
}


int
output_file::page_length()
    const
{
    return page_length_get(DEFAULT_PRINTER_LENGTH);
}


void
output_file::end_of_line_inner()
{
    if (!bol)
	fputc('\n');
}


const char *
output_file::type_name()
    const
{
    return "file";
}


static int
open_with_stale_nfs_retry(const char *path, int mode)
{
    //
    // Try to open the file.
    //
    errno = 0;
    int fd = glue_open(path, mode, 0666);

    //
    // Keep trying for one minute if we get a Stale NFS file handle
    // error.  Some systems suffer from this in a Very Bad Way.
    //
#ifdef ESTALE
    const int nsecs = 5;
    for (int ntries = 0; ntries < 60; ntries += nsecs)
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


output_file::output_file(const nstring &fn, bool binary) :
    file_name(fn),
    fd(-1),
    bol(true),
    pos(0)
{
    os_become_must_be_active();
    int mode = O_WRONLY | O_CREAT | O_TRUNC | (binary ? O_BINARY : O_TEXT);
    fd = open_with_stale_nfs_retry(fn.c_str(), mode);
    if (fd < 0)
    {
	int errno_old = errno;
	sub_context_ty sc;
	sc.errno_setx(errno_old);
	sc.var_set_string("File_Name", fn);
	sc.fatal_intl(i18n("open $filename: $errno"));
	// NOTREACHED
    }
}


output_ty *
output_file_open(const nstring &fn, bool binary)
{
    if (fn.empty() || fn == "-")
	return new output_stdout();
    return new output_file(fn, binary);
}


output_ty *
output_file_open(string_ty *fn, bool binary)
{
    return output_file_open(nstring(fn), binary);
}
