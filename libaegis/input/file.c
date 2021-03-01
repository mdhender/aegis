/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1995, 1998, 1999 Peter Miller;
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

#include <error.h>
#include <fopen_nfs.h>
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
	FILE		*fp;
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
	if (glue_fclose(this->fp))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", this->fn);
		fatal_intl(scp, i18n("close $filename: $errno"));
		/* NOTREACHED */
	}
	if (this->unlink_on_close)
		os_unlink_errok(this->fn);
	str_free(this->fn);
	this->fp = 0;
	this->fn = 0;
}


static long iread _((input_ty *, void *, long));

static long
iread(p, data, len)
	input_ty	*p;
	void		*data;
	long		len;
{
	input_file_ty	*this;
	long		result;

	if (len < 0)
		return 0;
	this = (input_file_ty *)p;
	result = fread(data, (size_t)1, (size_t)len, this->fp);
	if (result <= 0 && glue_ferror(this->fp))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", this->fn);
		fatal_intl(scp, i18n("read $filename: $errno"));
		/* NOTREACHED */
	}
	this->pos += result;
	return result;
}


static int get _((input_ty *));

static int
get(p)
	input_ty	*p;
{
	input_file_ty	*this;
	int		c;

	this = (input_file_ty *)p;
	c = glue_fgetc(this->fp);
	if (c == EOF && glue_ferror(this->fp))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", this->fn);
		fatal_intl(scp, i18n("read $filename: $errno"));
		/* NOTREACHED */
	}
	if (c != EOF)
		this->pos++;
	return c;
}


static long itell _((input_ty *));

static long
itell(p)
	input_ty	*p;
{
	input_file_ty	*this;

	this = (input_file_ty *)p;
	return this->pos;
}


static const char *name _((input_ty *));

static const char *
name(p)
	input_ty	*p;
{
	input_file_ty	*this;

	this = (input_file_ty *)p;
	return this->fn->str_text;
}


static long length _((input_ty *));

static long
length(p)
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
	iread,
	get,
	itell,
	name,
	length,
};


input_ty *
input_file_open(fn)
	const char	*fn;
{
	input_ty	*result;
	input_file_ty	*this;

	if (!fn || !*fn)
		return input_stdin();
	result = input_new(&vtbl);
	this = (input_file_ty *)result;
	os_become_must_be_active();
	this->fp = fopen_with_stale_nfs_retry(fn, "rb");
	if (!this->fp)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%s", fn);
		fatal_intl(scp, i18n("open $filename: $errno"));
		/* NOTREACHED */
	}
	this->fn = str_from_c(fn);
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
