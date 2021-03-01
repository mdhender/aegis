/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993 Peter Miller.
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
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: functions for pushing files around
 */

#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#include <file.h>
#include <error.h>
#include <glue.h>
#include <mem.h>
#include <os.h>
#include <trace.h>


/*
 * NAME
 *	copy_whole_file
 *
 * SYNOPSIS
 *	void copy_whole_file(string_ty *from, string_ty *to, int cmt);
 *
 * DESCRIPTION
 *	The copy_whole_file function is used to
 *	copy a file from one place to another.
 *
 *	The time-last-modified is preserved, but not the permissions.
 *	Permissions will be 0666 and modified by the active umask.
 *
 * ARGUMENTS
 *	from	- source path
 *	to	- destination path
 *	cmt	- copy modify time of file?
 *
 * CAVEAT
 *	Assumes that the user has already been set,
 *	uid, gid and umask.
 */

void
copy_whole_file(from, to, cmt)
	string_ty	*from;
	string_ty	*to;
	int		cmt;
{
	trace(("copy_whole_file(from = %08lX, to = %08lX)\n{\n"/*}*/, from, to));
	os_become_must_be_active();
	trace_string(from->str_text);
	trace_string(to->str_text);
	if (to->str_length)
	{
		if (glue_copyfile(from->str_text, to->str_text))
		{
			nfatal
			(
				"cp \"%s\" \"%s\"",
				from->str_text,
				to->str_text
			);
		}
	}
	else
	{
		if (glue_catfile(from->str_text))
			nfatal("cat \"%s\"", from->str_text);
	}

	/*
	 * copy last-time-modified of the file
	 */
	if (cmt && to->str_length)
	{
		time_t		mtime;

		mtime = os_mtime(from);
		os_mtime_set(to, mtime);
	}
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	read_whole_file
 *
 * SYNOPSIS
 *	string_ty *read_whole_file(char *path);
 *
 * DESCRIPTION
 *	The read_whole_file function is used to
 *	read a file into a string.  The file is assumed to be a text file.
 *
 * ARGUMENTS
 *	path	- pathname of file to be read
 *
 * RETURNS
 *	Pointer to string in dynamic memory containing text of file.
 *	Any trailing white space will have been removed.
 *
 * CAVEAT
 *	Assumes the user has already been set.
 */

string_ty *
read_whole_file(fn)
	char		*fn;
{
	size_t		length_max;
	size_t		length;
	char		*text;
	string_ty	*s;
	int		c;
	FILE		*fp;

	os_become_must_be_active();
	length_max = 1000;
	length = 0;
	text = mem_alloc(length_max);
	fp = glue_fopen(fn, "r");
	if (!fp)
		nfatal("open(\"%s\")", fn);
	while ((c = glue_fgetc(fp)) != EOF)
	{
		if (length >= length_max)
		{
			length_max *= 2;
			mem_change_size(&text, length_max);
		}
		text[length++] = c;
	}
	if (glue_ferror(fp))
		nfatal("read(\"%s\")", fn);
	while (length > 0 && isspace(text[length - 1]))
		--length;
	glue_fclose(fp);
	s = str_n_from_c(text, length);
	mem_free(text);
	return s;
}
