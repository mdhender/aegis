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
 * MANIFEST: functions to manipulate stdins
 */

#include <ac/stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <input/private.h>
#include <input/stdin.h>
#include <sub.h>
#include <str.h>


static string_ty *standard_input _((void));

static string_ty *
standard_input()
{
	static string_ty *name;
	sub_context_ty	*scp;

	if (!name)
	{
		scp = sub_context_new();
		name = subst_intl(scp, i18n("standard input"));
		sub_context_delete(scp);
	}
	return name;
}


static void destruct _((input_ty *));

static void
destruct(this)
	input_ty	*this;
{
}


static long iread _((input_ty *, void *, long));

static long
iread(this, data, len)
	input_ty	*this;
	void		*data;
	long		len;
{
	long		result;

	if (len <= 0)
		return 0;
	result = fread(data, 1, len, stdin);
	if (result <= 0 && ferror(stdin))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", standard_input());
		fatal_intl(scp, i18n("read $filename: $errno"));
		/* NOTREACHED */
	}
	return result;
}


static int get _((input_ty *));

static int
get(this)
	input_ty	*this;
{
	int		c;

	c = getchar();
	if (c == EOF && ferror(stdin))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", standard_input());
		fatal_intl(scp, i18n("read $filename: $errno"));
		/* NOTREACHED */
	}
	return c;
}


static long itell _((input_ty *));

static long
itell(this)
	input_ty	*this;
{
	return ftell(stdin);
}


static const char *name _((input_ty *));

static const char *
name(this)
	input_ty	*this;
{
	return standard_input()->str_text;
}


static long length _((input_ty *));

static long
length(this)
	input_ty	*this;
{
	struct stat	st;

	if (fstat(fileno(stdin), &st) < 0)
		return -1;
	if (!S_ISREG(st.st_mode))
		return -1;
	return st.st_size;
}


static input_vtbl_ty vtbl =
{
	sizeof(input_ty),
	destruct,
	iread,
	get,
	itell,
	name,
	length,
};


input_ty *
input_stdin()
{
	return input_new(&vtbl);
}
