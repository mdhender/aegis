/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-2003 Peter Miller;
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
 * MANIFEST: wrappers around operating system functions
 */

#include <ac/stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ac/unistd.h>

#include <glue.h>
#include <os.h>
#include <sub.h>
#include <trace.h>


void
os_mkdir(string_ty *path, int mode)
{
    int             uid;
    int             gid;
    int             um;

    trace(("os_mkdir(path = \"%s\", mode = 0%o)\n{\n", path->str_text, mode));
    os_become_must_be_active();
    if (glue_mkdir(path->str_text, mode))
    {
	sub_context_ty  *scp;

	scp = sub_context_new();
	sub_errno_set(scp);
	sub_var_set_string(scp, "File_Name", path);
	sub_var_set_format(scp, "Argument", "%5.5o", mode);
	fatal_intl(scp, i18n("mkdir(\"$filename\", $arg): $errno"));
	/* NOTREACHED */
    }

    /*
     * There could be Berkeley semantics about the group
     * of the newly created directory, so make sure it is
     * the one intended (egid).
     */
    os_become_query(&uid, &gid, &um);
    if (glue_chown(path->str_text, uid, gid))
    {
	sub_context_ty *scp;

	scp = sub_context_new();
	sub_errno_set(scp);
	sub_var_set_string(scp, "File_Name", path);
	sub_var_set_long(scp, "Argument", gid);
	fatal_intl(scp, i18n("chgrp(\"$filename\", $arg): $errno"));
	/* NOTREACHED */
    }

    /*
     * The set-group-id bit is ignored by a Berkeley semantics mkdir
     * (and it would be nuked the the chgrp, anyway)
     * so set it explicitly.
     */
    mode &= ~um;
    if (glue_chmod(path->str_text, mode))
    {
	sub_context_ty *scp;

	scp = sub_context_new();
	sub_errno_set(scp);
	sub_var_set_string(scp, "File_Name", path);
	sub_var_set_format(scp, "Argument", "%5.5o", mode);
	fatal_intl(scp, i18n("chmod(\"$filename\", $arg): $errno"));
	/* NOTREACHED */
    }
    trace(("}\n"));
}
