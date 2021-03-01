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
 * MANIFEST: functions to deliver output to stdout
 */

#include <ac/stdio.h>

#include <output/private.h>
#include <output/stdout.h>
#include <sub.h>


static string_ty *standard_output _((void));

static string_ty *
standard_output()
{
	static string_ty *name;
	sub_context_ty	*scp;

	if (!name)
	{
		scp = sub_context_new();
		name = subst_intl(scp, i18n("standard output"));
		sub_context_delete(scp);
	}
	return name;
}


static void destructor _((output_ty *));

static void
destructor(this)
	output_ty	*this;
{
	if (fflush(stdout))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", standard_output());
		fatal_intl(scp, i18n("write $filename: $errno"));
		/* NOTREACHED */
	}
}


static const char *filename _((output_ty *));

static const char *
filename(this)
	output_ty	*this;
{
	return standard_output()->str_text;
}


static long otell _((output_ty *));

static long
otell(this)
	output_ty	*this;
{
	return ftell(stdout);
}


static void oputc _((output_ty *, int));

static void
oputc(this, c)
	output_ty	*this;
	int		c;
{
	if (putchar(c) == EOF)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", standard_output());
		fatal_intl(scp, i18n("write $filename: $errno"));
		/* NOTREACHED */
	}
}


static void oputs _((output_ty *, const char *));

static void
oputs(this, s)
	output_ty	*this;
	const char	*s;
{
	if (fputs(s, stdout) == EOF)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", standard_output());
		fatal_intl(scp, i18n("write $filename: $errno"));
		/* NOTREACHED */
	}
}


static void owrite _((output_ty *, const void *, size_t));

static void
owrite(this, data, len)
	output_ty	*this;
	const void	*data;
	size_t		len;
{
	if (fwrite(data, 1, len, stdout) == EOF)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", standard_output());
		fatal_intl(scp, i18n("write $filename: $errno"));
		/* NOTREACHED */
	}
}


static output_vtbl_ty vtbl =
{
	sizeof(output_ty),
	"stdout",
	destructor,
	filename,
	otell,
	oputc,
	oputs,
	owrite,
};


output_ty *
output_stdout()
{
	return output_new(&vtbl);
}
