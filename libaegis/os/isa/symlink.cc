//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
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
// MANIFEST: implementation of the os_isa_symlink class
//

#include <ac/errno.h>

#include <ac/sys/types.h>
#include <sys/stat.h>

#include <glue.h>
#include <os.h>
#include <sub.h>


bool
os_isa_symlink(string_ty *path)
{
#ifndef S_IFLNK
    return 0;
#else
    if (path->str_length == 0)
	return 0;
    os_become_must_be_active();
    struct stat	st;
    int err = glue_lstat(path->str_text, &st);
    if (err < 0)
    {
	int errno_old = errno;
	if (errno_old == ENOENT)
	    return 0;
	sub_context_ty *scp = sub_context_new();
	sub_errno_setx(scp, errno_old);
	sub_var_set_string(scp, "File_Name", path);
	fatal_intl(scp, i18n("stat $filename: $errno"));
	// NOTREACHED
    }

    return (0 != S_ISLNK(st.st_mode));
#endif
}
