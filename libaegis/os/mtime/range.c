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

#include <ac/stddef.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <glue.h>
#include <os.h>
#include <sub.h>
#include <trace.h>


void
os_mtime_range(path, oldest_p, newest_p)
    string_ty       *path;
    time_t          *oldest_p;
    time_t          *newest_p;
{
    struct stat     st;
    int             oret;

    trace(("os_mtime_range(path = %08lX)\n{\n", (long)path));
    os_become_must_be_active();
    trace_string(path->str_text);

#ifdef S_IFLNK
    oret = glue_lstat(path->str_text, &st);
#else
    oret = glue_stat(path->str_text, &st);
#endif
    if (oret)
    {
	sub_context_ty  *scp;

	scp = sub_context_new();
	sub_errno_set(scp);
	sub_var_set_string(scp, "File_Name", path);
	fatal_intl(scp, i18n("stat $filename: $errno"));
	/* NOTREACHED */
    }

    /*
     * This is a bit tacky.  The codafs leaves the ctime at 0
     * until a stat change is made.  Compensate for this.
     */
    if (st.st_ctime == 0)
	st.st_ctime = st.st_mtime;

    /*
     * Return the last modified time.
     * They may try to fake us out by using utime,
     * so check the inode change time, too.
     */
    *newest_p = (st.st_ctime > st.st_mtime ? st.st_ctime : st.st_mtime);
    *oldest_p = (st.st_ctime < st.st_mtime ? st.st_ctime : st.st_mtime);
    trace(("}\n"));
}