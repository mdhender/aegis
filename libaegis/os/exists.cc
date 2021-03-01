//
//	aegis - project change supervisor
//	Copyright (C) 1991-2004 Peter Miller;
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
// MANIFEST: wrappers around operating system functions
//

#include <ac/errno.h>
#include <ac/stddef.h>
#include <ac/sys/types.h>
#include <sys/stat.h>

#include <glue.h>
#include <os.h>
#include <sub.h>


bool
os_exists(const nstring &path)
{
    os_become_must_be_active();
    struct stat st;
#ifdef S_IFLNK
    int oret = glue_lstat(path.c_str(), &st);
#else
    int oret = glue_stat(path.c_str(), &st);
#endif
    if (oret)
    {
	int errno_old = errno;
	if (errno_old != ENOENT && errno_old != ENOTDIR)
	{
	    sub_context_ty *scp = sub_context_new();
	    sub_errno_setx(scp, errno_old);
	    sub_var_set_string(scp, "File_Name", path);
	    fatal_intl(scp, i18n("stat $filename: $errno"));
	    // NOTREACHED
	}
	return false;
    }
    return true;
}


int
os_exists(string_ty *path)
{
    return os_exists(nstring(str_copy(path)));
}
