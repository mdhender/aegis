/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-2002 Peter Miller;
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

#include <glue.h>
#include <os.h>
#include <sub.h>


int
os_exists(path)
    string_ty       *path;
{
    struct stat     st;
    int             oret;

    os_become_must_be_active();
#ifdef S_IFLNK
    oret = glue_lstat(path->str_text, &st);
#else
    oret = glue_stat(path->str_text, &st);
#endif
    if (oret)
    {
	if (errno != ENOENT && errno != ENOTDIR)
	{
	    sub_context_ty *scp;

	    scp = sub_context_new();
	    sub_errno_set(scp);
	    sub_var_set_string(scp, "File_Name", path);
	    fatal_intl(scp, i18n("stat $filename: $errno"));
	    /* NOTREACHED */
	}
	return 0;
    }
    return 1;
}
