//
//	aegis - project change supervisor
//	Copyright (C) 1991-2006, 2008 Peter Miller
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
//	along with this program; if not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/errno.h>
#include <common/ac/stddef.h>
#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h>
#include <common/ac/unistd.h>

#include <libaegis/glue.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <common/trace.h>


void
os_mkdir(const nstring &path, int mode)
{
    trace(("os_mkdir(path = \"%s\", mode = 0%o)\n{\n", path.c_str(), mode));
    os_become_must_be_active();
    if (glue_mkdir(path.c_str(), mode))
    {
	int errno_old = errno;
	sub_context_ty *scp = sub_context_new();
	sub_errno_setx(scp, errno_old);
	sub_var_set_string(scp, "File_Name", path);
	sub_var_set_format(scp, "Argument", "%5.5o", mode);
	fatal_intl(scp, i18n("mkdir(\"$filename\", $arg): $errno"));
	// NOTREACHED
    }

    //
    // There could be Berkeley semantics about the group
    // of the newly created directory, so make sure it is
    // the one intended (egid).
    //
    int uid;
    int gid;
    int um;
    os_become_query(&uid, &gid, &um);
    if (glue_chown(path.c_str(), uid, gid))
    {
	int errno_old = errno;
	sub_context_ty *scp = sub_context_new();
	sub_errno_setx(scp, errno_old);
	sub_var_set_string(scp, "File_Name", path);
	sub_var_set_long(scp, "Argument", gid);
	fatal_intl(scp, i18n("chgrp(\"$filename\", $arg): $errno"));
	// NOTREACHED
    }

    //
    // The set-group-id bit is ignored by a Berkeley semantics mkdir
    // (and it would be nuked the the chgrp, anyway)
    // so set it explicitly.
    //
    mode &= ~um;
    if (glue_chmod(path.c_str(), mode))
    {
	int errno_old = errno;
	sub_context_ty *scp = sub_context_new();
	sub_errno_setx(scp, errno_old);
	sub_var_set_string(scp, "File_Name", path);
	sub_var_set_format(scp, "Argument", "%5.5o", mode);
	fatal_intl(scp, i18n("chmod(\"$filename\", $arg): $errno"));
	// NOTREACHED
    }
    trace(("}\n"));
}


void
os_mkdir(string_ty *path, int mode)
{
    os_mkdir(nstring(path), mode);
}
