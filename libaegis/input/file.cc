//
//	aegis - project change supervisor
//	Copyright (C) 1994, 1995, 1998, 1999, 2001, 2003-2005 Peter Miller;
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
// MANIFEST: functions for reading input from files
//

#include <ac/errno.h>
#include <ac/fcntl.h>
#include <ac/unistd.h>

#include <ac/sys/types.h>
#include <sys/socket.h>

#include <glue.h>
#include <input/curl.h>
#include <input/file.h>
#include <input/stdin.h>
#include <os.h>
#include <sub.h>
#include <url.h>


input_file::~input_file()
{
    if (fd >= 0)
    {
	if (glue_close(fd))
	{
	    int errno_old = errno;
	    sub_context_ty *scp = sub_context_new();
	    sub_errno_setx(scp, errno_old);
	    sub_var_set_string(scp, "File_Name", path);
	    fatal_intl(scp, i18n("close $filename: $errno"));
	    // NOTREACHED
	}

	// The file only exists if fd>=0
	if (unlink_on_close_flag)
	    os_unlink_errok(path);

	fd = -1;
    }
}


static int
open_with_stale_nfs_retry(const char *path, int mode)
{
    //
    // Try to open the file.
    //
    errno = 0;
    int perms = 0666;
    int fd = glue_open(path, mode, perms);

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
	fd = glue_open(path, mode, perms);
    }
#endif

    //
    // Return the result, both success and failure.
    // Errors are handled elsewhere.
    //
    return fd;
}


input_file::input_file(const nstring &arg1, bool arg2, bool empty_if_absent) :
    path(arg1),
    fd(-1),
    unlink_on_close_flag(arg2),
    pos(0)
{
    os_become_must_be_active();
    fd = open_with_stale_nfs_retry(path.c_str(), O_RDONLY);
    if (fd < 0)
    {
	int errno_old = errno;
	if (errno_old != ENOENT)
	{
	    sub_context_ty sc(__FILE__, __LINE__);
	    sc.errno_setx(errno_old);
	    sc.var_set_string("File_Name", path);
	    sc.fatal_intl(i18n("open $filename: $errno"));
	    // NOTREACHED
	}
    }
}


long
input_file::read_inner(void *data, size_t len)
{
    os_become_must_be_active();
    if (len == 0)
	return 0;
    if (fd < 0)
	return 0;
    long result = glue_read(fd, data, len);
    if (result < 0)
    {
	int errno_old = errno;
	sub_context_ty sc;
	sc.errno_setx(errno_old);
	sc.var_set_string("File_Name", path);
	sc.fatal_intl(i18n("read $filename: $errno"));
	// NOTREACHED
    }
    pos += result;
    return result;
}


long
input_file::ftell_inner()
{
    return pos;
}


nstring
input_file::name()
{
    return path;
}


long
input_file::length()
{
    if (fd < 0)
	return 0;
    return os_file_size(path.get_ref());
}


void
input_file::keepalive()
{
    if (fd >= 0)
    {
	int on = 1;
	// ignore any error
	setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, sizeof(on));
    }
}


void
input_file::unlink_on_close()
{
    unlink_on_close_flag = true;
}


input_ty *
input_file_open(const nstring &fn, bool unlink_on_close, bool empty_if_absent)
{
    os_become_must_be_active();
    if (fn.empty() || fn == "-")
	return new input_stdin();
    if (input_curl::looks_likely(fn))
	return new input_curl(fn);
    return new input_file(fn, unlink_on_close, empty_if_absent);
}


input_ty *
input_file_open(string_ty *fn, bool unlink_on_close)
{
    return input_file_open(nstring(fn), unlink_on_close);
}
