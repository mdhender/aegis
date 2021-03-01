//
//	aegis - project change supervisor
//	Copyright (C) 2007 Peter Miller
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
// MANIFEST: implementation of the user_creat_by_dir class
//

#include <common/ac/errno.h>
#include <common/ac/sys/types.h>
#include <sys/stat.h>

#include <libaegis/glue.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>


user_ty::pointer
user_ty::create_by_directory(const nstring &path)
{
    struct stat st;
    os_become_orig();
    if (glue_lstat(path.c_str(), &st) < 0)
    {
        int errno_old = errno;
        sub_context_ty sc;
        sc.errno_setx(errno_old);
        sc.var_set_string("File_Name", path);
        sc.fatal_intl(i18n("stat $filename: $errno"));
        // NOTREACHED
    }
    os_become_undo();

    //
    // Calculate the umask from bits NOT set in the directory's
    // permission mode bits, sanitized as required.
    //
    int umask = (st.st_mode & 0777) ^ 0777;
    umask |= 022;
    if (umask & 1)
        umask |= 4;
    umask &= 027;

    user_ty::pointer up = user_ty::create(st.st_uid, st.st_gid);
    up->umask_set(umask);
    return up;
}
