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

#include <error.h>
#include <glue.h>
#include <input/curl.h>
#include <input/file.h>
#include <input/private.h>
#include <input/stdin.h>
#include <mem.h>
#include <os.h>
#include <str.h>
#include <sub.h>
#include <url.h>

struct input_file_ty
{
    input_ty	    inherited;
    int		    fd;
    string_ty	    *fn;
    int		    unlink_on_close;
    long	    pos;
};


static void
destruct(input_ty *p)
{
    input_file_ty   *this_thing;

    this_thing = (input_file_ty *)p;
    if (glue_close(this_thing->fd))
    {
	sub_context_ty	*scp;
	int             errno_old;

	errno_old = errno;
	scp = sub_context_new();
	sub_errno_setx(scp, errno_old);
	sub_var_set_string(scp, "File_Name", this_thing->fn);
	fatal_intl(scp, i18n("close $filename: $errno"));
	// NOTREACHED
    }
    if (this_thing->unlink_on_close)
	os_unlink_errok(this_thing->fn);
    str_free(this_thing->fn);
    this_thing->fd = -1;
    this_thing->fn = 0;
}


static long
input_file_read(input_ty *p, void *data, size_t len)
{
    input_file_ty   *this_thing;
    long	    result;

    os_become_must_be_active();
    if (len == 0)
	return 0;
    this_thing = (input_file_ty *)p;
    result = glue_read(this_thing->fd, data, len);
    if (result < 0)
    {
	sub_context_ty	*scp;
	int             errno_old;

	errno_old = errno;
	scp = sub_context_new();
	sub_errno_setx(scp, errno_old);
	sub_var_set_string(scp, "File_Name", this_thing->fn);
	fatal_intl(scp, i18n("read $filename: $errno"));
	// NOTREACHED
    }
    this_thing->pos += result;
    return result;
}


static long
input_file_ftell(input_ty *p)
{
    input_file_ty   *this_thing;

    this_thing = (input_file_ty *)p;
    return this_thing->pos;
}


static string_ty *
input_file_name(input_ty *p)
{
    input_file_ty   *this_thing;

    this_thing = (input_file_ty *)p;
    return this_thing->fn;
}


static long
input_file_length(input_ty *p)
{
    input_file_ty   *this_thing;

    this_thing = (input_file_ty *)p;
    return os_file_size(this_thing->fn);
}


static void
input_file_keepalive(input_ty *fp)
{
    input_file_ty   *ip;
    int             on;

    ip = (input_file_ty *)fp;
    on = 1;
    // ignore any error
    setsockopt(ip->fd, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, sizeof(on));
}


static input_vtbl_ty vtbl =
{
    sizeof(input_file_ty),
    destruct,
    input_file_read,
    input_file_ftell,
    input_file_name,
    input_file_length,
    input_file_keepalive,
};


static int
open_with_stale_nfs_retry(const char *path, int mode)
{
    int		    fd;
    int		    perms =	    0666;
#ifdef ESTALE
    int		    ntries;
    const int	    nsecs =	    5;
#endif

    //
    // Try to open the file.
    //
    errno = 0;
    fd = glue_open(path, mode, perms);

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
	fd = glue_open(path, mode, perms);
    }
#endif

    //
    // Return the result, both success and failure.
    // Errors are handled elsewhere.
    //
    return fd;
}


input_ty *
input_file_open(string_ty *fn)
{
    input_ty	    *result;
    input_file_ty   *this_thing;
    int		    fd;

    if (!fn || !fn->str_length)
	return input_stdin();

    os_become_must_be_active();
    fd = open_with_stale_nfs_retry(fn->str_text, O_RDONLY);
    if (fd < 0 && errno == ENOENT && input_curl_looks_likely(fn))
    {
        // Note: we use the input_curl_looks_likely function EVEN when
        // -lcurl is not available, .
#ifdef HAVE_LIBCURL
	return input_curl_open(fn);
#else
	// This fragment should probably be moved into input_curl_open
	// in the case when HAVE_LIBCURL is undefined.
	url temp = nstring(str_copy(fn));
	if (!temp.is_a_file())
	{
	    sub_context_ty sc(__FILE__, __LINE__);
	    sc.var_set_string("File_Name", fn);
	    sc.fatal_intl(i18n("open $filename: no curl library"));
	    // NOTREACHED
	}
	str_free(fn);
	fn = str_copy(temp.get_path().get_ref());
	fd = open_with_stale_nfs_retry(fn->str_text, O_RDONLY);
#endif
    }

    result = input_new(&vtbl);
    this_thing = (input_file_ty *)result;
    this_thing->fd = fd;
    if (this_thing->fd < 0)
    {
	int errno_old = errno;
	sub_context_ty sc(__FILE__, __LINE__);
	sc.errno_setx(errno_old);
	sc.var_set_string("File_Name", fn);
	sc.fatal_intl(i18n("open $filename: $errno"));
	// NOTREACHED
    }
    this_thing->fn = str_copy(fn);
    this_thing->unlink_on_close = 0;
    this_thing->pos = 0;
    return result;
}


void
input_file_unlink_on_close(input_ty *fp)
{
    input_file_ty   *this_thing;

    if (fp->vptr != &vtbl)
	return;
    this_thing = (input_file_ty *)fp;
    this_thing->unlink_on_close = 1;
}
