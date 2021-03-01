//
//      aegis - project change supervisor
//      Copyright (C) 1991-2006, 2008, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/errno.h>
#include <common/ac/stddef.h>
#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h>

#include <libaegis/glue.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <common/trace.h>


void
os_mtime_range(string_ty *path, time_t *oldest_p, time_t *newest_p)
{
    struct stat     st;
    int             oret;

    trace(("os_mtime_range(path = s%p)\n{\n", path));
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
        int             errno_old;

        errno_old = errno;
        scp = sub_context_new();
        sub_errno_setx(scp, errno_old);
        sub_var_set_string(scp, "File_Name", path);
        fatal_intl(scp, i18n("stat $filename: $errno"));
        // NOTREACHED
    }

    //
    // This is a bit tacky.  The codafs leaves the ctime at 0
    // until a stat change is made.  Compensate for this.
    //
    if (st.st_ctime == 0)
        st.st_ctime = st.st_mtime;

    //
    // Return the last modified time.
    // They may try to fake us out by using utime,
    // so check the inode change time, too.
    //
    *newest_p = (st.st_ctime > st.st_mtime ? st.st_ctime : st.st_mtime);
    *oldest_p = (st.st_ctime < st.st_mtime ? st.st_ctime : st.st_mtime);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
