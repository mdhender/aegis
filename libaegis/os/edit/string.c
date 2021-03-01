/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate strings
 */

#include <ac/stdio.h>

#include <file.h>
#include <fopen_nfs.h>
#include <glue.h>
#include <os.h>
#include <sub.h>


string_ty *
os_edit_string(subject, et)
    string_ty       *subject;
    edit_ty         et;
{
    string_ty       *filename;
    string_ty       *result;
    FILE            *fp;

    filename = os_edit_filename(0);
    os_become_orig();
    fp = fopen_with_stale_nfs_retry(filename->str_text, "w");
    if (!fp)
    {
	sub_context_ty  *scp;

	scp = sub_context_new();
	sub_errno_set(scp);
	sub_var_set_string(scp, "File_Name", filename);
	fatal_intl(scp, i18n("open $filename: $errno"));
	sub_context_delete(scp);
    }
    if (subject)
    {
	glue_fwrite(subject->str_text, 1, subject->str_length, fp);
	if
	(
	    subject->str_length
	&&
	    subject->str_text[subject->str_length - 1] != '\n'
	)
	    glue_fputc('\n', fp);
    }
    glue_fclose(fp);
    os_become_undo();
    os_edit(filename, et);
    os_become_orig();
    result = read_whole_file(filename);
    os_unlink(filename);
    os_become_undo();
    return result;
}
