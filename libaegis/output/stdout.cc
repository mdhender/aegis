//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003, 2004 Peter Miller;
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
// MANIFEST: functions to deliver output to stdout
//

#include <ac/errno.h>
#include <ac/stddef.h>
#include <ac/stdio.h>
#include <ac/unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <output/private.h>
#include <output/stdout.h>
#include <page.h>
#include <sub.h>


typedef struct output_stdout_ty output_stdout_ty;
struct output_stdout_ty
{
	output_ty	inherited;
	int		bol;
};


static string_ty *
standard_output(void)
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


static void
output_stdout_destructor(output_ty *this_thing)
{
}


static string_ty *
output_stdout_filename(output_ty *this_thing)
{
	return standard_output();
}


static long
output_stdout_ftell(output_ty *fp)
{
	return lseek(fileno(stdout), 0L, SEEK_CUR);
}


static void
output_stdout_write(output_ty *fp, const void *data, size_t len)
{
	output_stdout_ty *this_thing;

	this_thing = (output_stdout_ty *)fp;
	if (write(fileno(stdout), data, len) < 0)
	{
		sub_context_ty	*scp;
		int             errno_old;

		errno_old = errno;
		scp = sub_context_new();
		sub_errno_setx(scp, errno_old);
		sub_var_set_string(scp, "File_Name", standard_output());
		fatal_intl(scp, i18n("write $filename: $errno"));
		// NOTREACHED
	}
	if (len > 0)
		this_thing->bol = (((const char *)data)[len - 1] == '\n');
}


static int
output_stdout_page_width(output_ty *fp)
{
	struct stat	st;

	if (fstat(fileno(stdout), &st) == 0 && S_ISREG(st.st_mode))
		return page_width_get(DEFAULT_PRINTER_WIDTH);
	return page_width_get(-1) - 1;
}


static int
output_stdout_page_length(output_ty *fp)
{
	struct stat	st;

	if (fstat(fileno(stdout), &st) == 0 && S_ISREG(st.st_mode))
		return page_length_get(DEFAULT_PRINTER_LENGTH);
	return page_length_get(-1);
}


static void
output_stdout_eoln(output_ty *fp)
{
	output_stdout_ty *this_thing;

	this_thing = (output_stdout_ty *)fp;
	if (!this_thing->bol)
		output_fputc(fp, '\n');
}


static output_vtbl_ty vtbl =
{
	sizeof(output_stdout_ty),
	output_stdout_destructor,
	output_stdout_filename,
	output_stdout_ftell,
	output_stdout_write,
	output_generic_flush,
	output_stdout_page_width,
	output_stdout_page_length,
	output_stdout_eoln,
	"stdout",
};


output_ty *
output_stdout(void)
{
	output_ty	*result;
	output_stdout_ty *this_thing;

	result = output_new(&vtbl);
	this_thing = (output_stdout_ty *)result;
	this_thing->bol = 1;
	return result;
}
