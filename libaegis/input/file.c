/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1995, 1998, 1999, 2001 Peter Miller;
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
 * MANIFEST: functions for reading input from files
 */

#include <ac/errno.h>
#include <ac/fcntl.h>
#include <ac/unistd.h>

#include <error.h>
#include <glue.h>
#include <input/file.h>
#include <input/private.h>
#include <input/stdin.h>
#include <mem.h>
#include <os.h>
#include <str.h>
#include <sub.h>

typedef struct input_file_ty input_file_ty;
struct input_file_ty
{
	input_ty	inherited;
	int		fd;
	string_ty	*fn;
	int		unlink_on_close;
	long		pos;
};


static void destruct _((input_ty *));

static void
destruct(p)
	input_ty	*p;
{
	input_file_ty	*this;

	this = (input_file_ty *)p;
	if (glue_close(this->fd))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set_string(scp, "File_Name", this->fn);
		fatal_intl(scp, i18n("close $filename: $errno"));
		/* NOTREACHED */
	}
	if (this->unlink_on_close)
		os_unlink_errok(this->fn);
	str_free(this->fn);
	this->fd = -1;
	this->fn = 0;
}


static long input_file_read _((input_ty *, void *, size_t));

static long
input_file_read(p, data, len)
	input_ty	*p;
	void		*data;
	size_t		len;
{
	input_file_ty	*this;
	long		result;

	os_become_must_be_active();
	if (len < 0)
		return 0;
	this = (input_file_ty *)p;
	result = glue_read(this->fd, data, len);
	if (result < 0)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set_string(scp, "File_Name", this->fn);
		fatal_intl(scp, i18n("read $filename: $errno"));
		/* NOTREACHED */
	}
	this->pos += result;
	return result;
}


static long input_file_ftell _((input_ty *));

static long
input_file_ftell(p)
	input_ty	*p;
{
	input_file_ty	*this;

	this = (input_file_ty *)p;
	return this->pos;
}


static string_ty *input_file_name _((input_ty *));

static string_ty *
input_file_name(p)
	input_ty	*p;
{
	input_file_ty	*this;

	this = (input_file_ty *)p;
	return this->fn;
}


static long input_file_length _((input_ty *));

static long
input_file_length(p)
	input_ty	*p;
{
	input_file_ty	*this;

	this = (input_file_ty *)p;
	return os_file_size(this->fn);
}


static input_vtbl_ty vtbl =
{
	sizeof(input_file_ty),
	destruct,
	input_file_read,
	input_file_ftell,
	input_file_name,
	input_file_length,
};


static int open_with_stale_nfs_retry _((const char *, int));

static int
open_with_stale_nfs_retry(path, mode)
	const char	*path;
	int		mode;
{
	int		fd;
	int		perms = 0666;
#ifdef ESTALE
	int		ntries;
	const int	nsecs = 5;
#endif

	/*
	 * Try to open the file.
	 */
	errno = 0;
	fd = glue_open(path, mode, perms);

	/*
	 * Keep trying for one minute if we get a Stale NFS file handle
	 * error.  Some systems suffer from this in a Very Bad Way.
	 */
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

	/*
	 * Return the result, both success and failure.
	 * Errors are handled elsewhere.
	 */
	return fd;
}


input_ty *
input_file_open(fn)
	string_ty	*fn;
{
	input_ty	*result;
	input_file_ty	*this;

	if (!fn || !fn->str_length)
		return input_stdin();
	result = input_new(&vtbl);
	this = (input_file_ty *)result;
	os_become_must_be_active();
	this->fd = open_with_stale_nfs_retry(fn->str_text, O_RDONLY);
	if (this->fd < 0)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set_string(scp, "File_Name", fn);
		fatal_intl(scp, i18n("open $filename: $errno"));
		/* NOTREACHED */
	}
	this->fn = str_copy(fn);
	this->unlink_on_close = 0;
	this->pos = 0;
	return result;
}


void
input_file_unlink_on_close(fp)
	input_ty	*fp;
{
	input_file_ty	*this;

	if (fp->vptr != &vtbl)
		return;
	this = (input_file_ty *)fp;
	this->unlink_on_close = 1;
}
