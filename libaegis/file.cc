//
//	aegis - project change supervisor
//	Copyright (C) 1991-1995, 1998, 1999, 2001-2005 Peter Miller;
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
// MANIFEST: functions for pushing files around
//

#include <ac/stdio.h>
#include <ac/ctype.h>
#include <ac/errno.h>

#include <error.h>
#include <file.h>
#include <fopen_nfs.h>
#include <glue.h>
#include <input/file_text.h>
#include <mem.h>
#include <nstring/accumulator.h>
#include <os.h>
#include <sub.h>
#include <trace.h>


//
// NAME
//	copy_whole_file
//
// SYNOPSIS
//	void copy_whole_file(string_ty *from, string_ty *to, int cmt);
//
// DESCRIPTION
//	The copy_whole_file function is used to
//	copy a file from one place to another.
//
//	The time-last-modified is preserved, but not the permissions.
//	Permissions will be 0666 and modified by the active umask.
//
// ARGUMENTS
//	from	- source path
//	to	- destination path
//	cmt	- copy modify time of file?
//
// CAVEAT
//	Assumes that the user has already been set,
//	uid, gid and umask.
//

void
copy_whole_file(string_ty *from, string_ty *to, int cmt)
{
    trace(("copy_whole_file(from = %08lX, to = %08lX)\n{\n",
	(long)from, (long)to));
    os_become_must_be_active();
    trace_string(from->str_text);
    trace_string(to->str_text);
    if (to->str_length)
    {
	if (glue_copyfile(from->str_text, to->str_text))
	{
	    sub_context_ty  *scp;
	    int             errno_old;

	    errno_old = errno;
	    scp = sub_context_new();
	    sub_errno_setx(scp, errno_old);
	    sub_var_set_string(scp, "File_Name1", from);
	    sub_var_set_string(scp, "File_Name2", to);
	    fatal_intl(scp, i18n("cp $filename1 $filename2: $errno"));
	    // NOTREACHED
	}
    }
    else
    {
	if (glue_catfile(from->str_text))
	{
	    sub_context_ty  *scp;
	    int             errno_old;

	    errno_old = errno;
	    scp = sub_context_new();
	    sub_errno_setx(scp, errno_old);
	    sub_var_set_string(scp, "File_Name", from);
	    fatal_intl(scp, i18n("cat $filename: $errno"));
	    // NOTREACHED
	}
    }

    //
    // copy last-time-modified of the file
    //
    if (cmt && to->str_length)
    {
	os_mtime_set(to, os_mtime_actual(from));
    }
    trace(("}\n"));
}


string_ty *
read_whole_file(string_ty *fn)
{
    nstring result(read_whole_file(nstring(fn)));
    return str_copy(result.get_ref());
}


nstring
read_whole_file(const nstring &fn)
{
    nstring_accumulator acc;
    os_become_must_be_active();
    input_ty *fp = input_file_text_open(fn.get_ref());
    for (;;)
    {
	int c = fp->getc();
	if (c == EOF)
	    break;
	acc.push_back(c);
    }
    while (acc.size() > 0 && isspace((unsigned char)acc.back()))
	acc.pop_back();
    delete fp;
    return acc.mkstr();
}


//
// NAME
//	files_are_different
//
// SYNOPSIS
//	int files_are_different(string_ty *, string_ty *);
//
// DESCRIPTION
//	The files_are_different function is used to compare the
//	contents of two files.	The files to compare are given by the
//	two arguments.	It is assumed that os_become is active.
//
// RETURNS
//	int;	zero if the files are the same
//		non-zero if the file are different
//

int
files_are_different(string_ty *s1, string_ty *s2)
{
    int		    result;

    os_become_must_be_active();
    result = glue_file_compare(s1->str_text, s2->str_text);
    if (result < 0)
    {
	sub_context_ty	*scp;
	int             errno_old;

	errno_old = errno;
	scp = sub_context_new();
	sub_errno_setx(scp, errno_old);
	sub_var_set_string(scp, "File_Name1", s1);
	sub_var_set_string(scp, "File_Name2", s2);
	fatal_intl(scp, i18n("cmp $filename1 $filename2: $errno"));
	// NOTREACHED
    }
    return result;
}
