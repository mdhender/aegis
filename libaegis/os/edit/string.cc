//
//	aegis - project change supervisor
//	Copyright (C) 2002-2006, 2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/errno.h>
#include <common/ac/stdio.h>

#include <libaegis/file.h>
#include <libaegis/fopen_nfs.h>
#include <libaegis/glue.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>


string_ty *
os_edit_string(string_ty *subject, edit_ty et)
{
    nstring filename(os_edit_filename(0));
    os_become_orig();
    FILE *fp = fopen_with_stale_nfs_retry(filename.c_str(), "w");
    if (!fp)
    {
	int errno_old = errno;
	sub_context_ty  sc;
	sc.errno_setx(errno_old);
	sc.var_set_string("File_Name", filename);
	sc.fatal_intl(i18n("open $filename: $errno"));
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
    nstring result(read_whole_file(filename));
    os_unlink(filename);
    os_become_undo();
    return str_copy(result.get_ref());
}
