/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1995, 1998 Peter Miller;
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
 * MANIFEST: functions to get lex input from files
 */

#include <error.h>
#include <fopen_nfs.h>
#include <glue.h>
#include <lex/file.h>
#include <mem.h>
#include <sub.h>

typedef struct lex_input_file_ty lex_input_file_ty;
struct lex_input_file_ty
{
	lex_input_method_ty method;
	FILE		*fp;
	char		*fn;
};


static void destruct _((lex_input_ty *));

static void
destruct(p)
	lex_input_ty	*p;
{
	lex_input_file_ty *this;

	this = (lex_input_file_ty *)p;
	if (glue_fclose(this->fp))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%s", this->fn);
		fatal_intl(scp, i18n("close $filename: $errno"));
		/* NOTREACHED */
	}
	mem_free(this->fn);
	mem_free(this);
}


static int get _((lex_input_ty *));

static int
get(p)
	lex_input_ty	*p;
{
	lex_input_file_ty *this;
	int		c;

	this = (lex_input_file_ty *)p;
	c = glue_fgetc(this->fp);
	if (c == EOF && glue_ferror(this->fp))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%s", this->fn);
		fatal_intl(scp, i18n("read $filename: $errno"));
		/* NOTREACHED */
	}
	return c;
}


static void unget _((lex_input_ty *, int));

static void
unget(p, c)
	lex_input_ty	*p;
	int		c;
{
	lex_input_file_ty *this;

	this = (lex_input_file_ty *)p;
	if (c != EOF)
		glue_ungetc(c, this->fp);
}


static char *name _((lex_input_ty *));

static char *
name(p)
	lex_input_ty	*p;
{
	lex_input_file_ty *this;

	this = (lex_input_file_ty *)p;
	return this->fn;
}


lex_input_ty *
lex_input_file_open(fn)
	char	*fn;
{
	lex_input_file_ty *result;

	result = mem_alloc(sizeof(lex_input_file_ty));
	result->method.destruct = destruct;
	result->method.get = get;
	result->method.unget = unget;
	result->method.name = name;
	result->fp = fopen_with_stale_nfs_retry(fn, "r");
	if (!result->fp)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%s", fn);
		fatal_intl(scp, i18n("open $filename: $errno"));
		/* NOTREACHED */
	}
	result->fn = mem_copy_string(fn);
	return (lex_input_ty *)result;
}
