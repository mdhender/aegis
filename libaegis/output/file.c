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
 * MANIFEST: functions to deliver output to files
 */

#include <ac/errno.h>
#include <ac/stdio.h>

#include <fopen_nfs.h>
#include <glue.h>
#include <mem.h>
#include <os.h>
#include <output/file.h>
#include <output/private.h>
#include <output/stdout.h>
#include <sub.h>


typedef struct output_file_ty output_file_ty;
struct output_file_ty
{
	output_ty	inherited;
	char		*filename;
	FILE		*deeper;
};


static void destructor _((output_ty *));

static void
destructor(fp)
	output_ty	*fp;
{
	output_file_ty	*this;

	this = (output_file_ty *)fp;
	if (glue_fflush(this->deeper))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%s", this->filename);
		fatal_intl(scp, i18n("write $filename: $errno"));
		/* NOTREACHED */
	}
	if (glue_fclose(this->deeper))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%s", this->filename);
		fatal_intl(scp, i18n("close $filename: $errno"));
		/* NOTREACHED */
	}
	mem_free(this->filename);
}


static const char *filename _((output_ty *));

static const char *
filename(fp)
	output_ty	*fp;
{
	output_file_ty	*this;

	this = (output_file_ty *)fp;
	return this->filename;
}


static long otell _((output_ty *));

static long
otell(fp)
	output_ty	*fp;
{
	output_file_ty	*this;

	this = (output_file_ty *)fp;
	return ftell(this->deeper);
}


static void oputc _((output_ty *, int));

static void
oputc(fp, c)
	output_ty	*fp;
	int		c;
{
	output_file_ty	*this;

	this = (output_file_ty *)fp;
	if (glue_fputc(c, this->deeper) == EOF)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%s", this->filename);
		fatal_intl(scp, i18n("write $filename: $errno"));
		/* NOTREACHED */
	}
}


static void owrite _((output_ty *, const void *s, size_t));

static void
owrite(fp, data, len)
	output_ty	*fp;
	const void	*data;
	size_t		len;
{
	output_file_ty	*this;

	this = (output_file_ty *)fp;
	if (glue_fwrite(data, 1, len, this->deeper) == EOF)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%s", this->filename);
		fatal_intl(scp, i18n("write $filename: $errno"));
		/* NOTREACHED */
	}
}


static output_vtbl_ty vtbl =
{
	sizeof(output_file_ty),
	"file",
	destructor,
	filename,
	otell,
	oputc,
	output_generic_fputs,
	owrite,
};


static output_ty *output_file_open _((const char *, const char *));

static output_ty *
output_file_open(fn, mode)
	const char	*fn;
	const char	*mode;
{
	output_ty	*result;
	output_file_ty	*this;

	if (!fn || !*fn)
		return output_stdout();
	os_become_must_be_active();
	result = output_new(&vtbl);
	this = (output_file_ty *)result;
	this->deeper = fopen_with_stale_nfs_retry(fn, mode);
	if (!this->deeper)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%s", fn);
		fatal_intl(scp, i18n("open $filename: $errno"));
		/* NOTREACHED */
	}
	this->filename = mem_copy_string(fn);
	return result;
}


output_ty *
output_file_text_open(fn)
	const char	*fn;
{
	return output_file_open(fn, "w");
}


output_ty *
output_file_binary_open(fn)
	const char	*fn;
{
	return output_file_open(fn, "wb");
}
