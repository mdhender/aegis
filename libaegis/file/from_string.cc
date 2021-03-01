//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003-2006, 2008 Peter Miller
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

#include <common/ac/fcntl.h>
#include <common/ac/errno.h>
#include <common/ac/unistd.h>

#include <libaegis/file.h>
#include <libaegis/glue.h>
#include <libaegis/sub.h>
#include <libaegis/os.h>


void
file_from_string(string_ty *filename, string_ty *content, int mode)
{
    int             fd;

    fd = glue_creat(filename->str_text, (mode > 0 ? mode : 0666));
    if (fd < 0)
    {
	sub_context_ty	*scp;
	int             errno_old;

	errno_old = errno;
	scp = sub_context_new();
	sub_errno_setx(scp, errno_old);
	sub_var_set_string(scp, "File_Name", filename);
	fatal_intl(scp, i18n("create $filename: $errno"));
	// NOTREACHED
	sub_context_delete(scp);
    }
    if (content && content->str_length)
    {
	glue_write(fd, content->str_text, content->str_length);
	if (content->str_text[content->str_length - 1] != '\n')
    	    glue_write(fd, "\n", 1);
    }
    glue_close(fd);
    if (mode > 0)
	os_chmod(filename, mode);
}
