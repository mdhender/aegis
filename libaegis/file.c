/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995, 1998, 1999 Peter Miller;
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
 * MANIFEST: functions for pushing files around
 */

#include <ac/stdio.h>
#include <ac/ctype.h>
#include <ac/errno.h>

#include <error.h>
#include <file.h>
#include <fopen_nfs.h>
#include <glue.h>
#include <input/file_text.h>
#include <mem.h>
#include <os.h>
#include <sub.h>
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
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_errno_set(scp);
			sub_var_set(scp, "File_Name1", "%S", from);
			sub_var_set(scp, "File_Name2", "%S", to);
			fatal_intl(scp, i18n("cp $filename1 $filename2: $errno"));
			/* NOTREACHED */
		}
	}
	else
	{
		if (glue_catfile(from->str_text))
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_errno_set(scp);
			sub_var_set(scp, "File_Name", "%S", from);
			fatal_intl(scp, i18n("cat $filename: $errno"));
			/* NOTREACHED */
		}
	}

	/*
	 * copy last-time-modified of the file
	 */
	if (cmt && to->str_length)
	{
		time_t		oldest;
		time_t		newest;

		os_mtime_range(from, &oldest, &newest);
		os_mtime_set(to, oldest);
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
	input_ty	*fp;

	os_become_must_be_active();
	length_max = 1000;
	length = 0;
	text = mem_alloc(length_max);
	fp = input_file_text_open(fn);
	for (;;)
	{
		c = input_getc(fp);
		if (c == EOF)
			break;
		if (length >= length_max)
		{
			length_max = length_max * 2 + 16;
			text = mem_change_size(text, length_max);
		}
		text[length++] = c;
	}
	while (length > 0 && isspace(text[length - 1]))
		--length;
	input_delete(fp);
	s = str_n_from_c(text, length);
	mem_free(text);
	return s;
}


/*
 * NAME
 *	files_are_different
 *
 * SYNOPSIS
 *	int files_are_different(string_ty *, string_ty *);
 *
 * DESCRIPTION
 *	The files_are_different function is used to compare the
 *	contents of two files.  The files to compare are given by the
 *	two arguments.  It is assumed that os_become is active.
 *
 * RETURNS
 *	int;	zero if the files are the same
 *		non-zero if the file are different
 */

int
files_are_different(s1, s2)
	string_ty	*s1;
	string_ty	*s2;
{
	int		result;
	os_become_must_be_active();
	result = glue_file_compare(s1->str_text, s2->str_text);
	if (result < 0)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name1", "%S", s1);
		sub_var_set(scp, "File_Name2", "%S", s2);
		fatal_intl(scp, i18n("cmp $filename1 $filename2: $errno"));
		/* NOTREACHED */
	}
	return result;
}
