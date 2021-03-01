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

#include <ac/errno.h>
#include <ac/stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ac/unistd.h>

#include <glue.h>
#include <os.h>
#include <sub.h>
#include <trace.h>


void
os_unlink_errok(string_ty *path)
{
    struct stat     st;
    int             oret;

    trace(("os_unlink_errok(path = %08lX)\n{\n", (long)path));
    os_become_must_be_active();
    trace_string(path->str_text);

    /*
     * We must check that we are not unlinking a directory,
     * because we are set-uid-root, and root can unlink directories!
     */
#ifdef S_IFLNK
    oret = glue_lstat(path->str_text, &st);
#else
    oret = glue_stat(path->str_text, &st);
#endif
    if (oret && errno == ENOENT)
    {
	/* Don't complaint if it's not there. */
	trace(("}\n"));
	return;
    }
    if (oret)
    {
	sub_context_ty  *scp;

	scp = sub_context_new();
	sub_errno_set(scp);
	sub_var_set_string(scp, "File_Name", path);
	if (errno != ENOENT && errno != ENOTDIR)
	    fatal_intl(scp, i18n("stat $filename: $errno"));
	error_intl(scp, i18n("warning: stat $filename: $errno"));
	sub_context_delete(scp);
    }
    else if ((st.st_mode & S_IFMT) == S_IFDIR)
    {
	sub_context_ty  *scp;

	scp = sub_context_new();
	sub_errno_setx(scp, EISDIR);
	sub_var_set_string(scp, "File_Name", path);
	error_intl(scp, i18n("warning: unlink $filename: $errno"));
	sub_context_delete(scp);
    }
    else if (glue_unlink(path->str_text))
    {
	sub_context_ty  *scp;

	scp = sub_context_new();
	sub_errno_set(scp);
	sub_var_set_string(scp, "File_Name", path);
	error_intl(scp, i18n("warning: unlink $filename: $errno"));
	sub_context_delete(scp);
    }
    trace(("}\n"));
}
