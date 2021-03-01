//
//	aegis - project change supervisor
//	Copyright (C) 1991-2005 Peter Miller;
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

#include <common/ac/errno.h>
#include <common/ac/stddef.h>
#include <common/ac/sys/types.h>
#include <sys/stat.h>
#include <common/ac/unistd.h>

#include <libaegis/glue.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <common/trace.h>


void
os_unlink(const nstring &path)
{
    trace(("os_unlink(path = \"%s\")\n{\n", path.c_str()));
    os_become_must_be_active();

    //
    // We must check that we are not unlinking a directory,
    // because we are set-uid-root, and root can unlink directories!
    //
    struct stat st;
#ifdef S_IFLNK
    int oret = glue_lstat(path.c_str(), &st);
#else
    int oret = glue_stat(path.c_str(), &st);
#endif
    if (oret)
    {
	int errno_old = errno;
	if (errno_old == ENOENT)
	{
	    // Don't complain if it's not there.
	    trace(("}\n"));
	    return;
	}
	sub_context_ty *scp = sub_context_new();
	sub_errno_setx(scp, errno_old);
	sub_var_set_string(scp, "File_Name", path);
	fatal_intl(scp, i18n("stat $filename: $errno"));
	// NOTREACHED
    }
    if ((st.st_mode & S_IFMT) == S_IFDIR)
    {
	sub_context_ty *scp = sub_context_new();
	sub_errno_setx(scp, EISDIR);
	sub_var_set_string(scp, "File_Name", path);
	fatal_intl(scp, i18n("unlink $filename: $errno"));
	// NOTREACHED
    }
    if (glue_unlink(path.c_str()))
    {
	int errno_old = errno;
	sub_context_ty *scp = sub_context_new();
	sub_errno_setx(scp, errno_old);
	sub_var_set_string(scp, "File_Name", path);
	fatal_intl(scp, i18n("unlink $filename: $errno"));
	// NOTREACHED
    }
    trace(("}\n"));
}


void
os_unlink(string_ty *path)
{
    os_unlink(nstring(str_copy(path)));
}
