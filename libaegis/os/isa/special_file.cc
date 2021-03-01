//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003-2005 Peter Miller;
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
// MANIFEST: functions to manipulate special_files
//

#include <common/ac/errno.h>

#include <common/ac/sys/types.h>
#include <sys/stat.h>

#include <libaegis/glue.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>


int
os_isa_special_file(string_ty *path)
{
    struct stat	st;
    int		err;

    os_become_must_be_active();
    if (path->str_length == 0)
	return 1;
#ifdef S_IFLNK
    err = glue_lstat(path->str_text, &st);
#else
    err = glue_stat(path->str_text, &st);
#endif
    if (err < 0)
    {
	sub_context_ty  *scp;
	int             errno_old;

	errno_old = errno;
	if (errno_old == ENOENT)
	    return 0;
	scp = sub_context_new();
	sub_errno_setx(scp, errno_old);
	sub_var_set_string(scp, "File_Name", path);
	fatal_intl(scp, i18n("stat $filename: $errno"));
	// NOTREACHED
    }

    //
    // This is used by aenf.
    // By special, in this context, I mean not a regular file.
    //
    return !S_ISREG(st.st_mode);
}
