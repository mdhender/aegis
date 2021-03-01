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
#include <utime.h>

#include <libaegis/glue.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <common/trace.h>


void
os_mtime_set_errok(string_ty *path, time_t when)
{
    struct utimbuf  utb;

    trace(("os_mtime_set_errok(path = %p, when = %ld)\n{\n",
        path, (long)when));
    os_become_must_be_active();
    trace_string(path->str_text);
    trace(("when = %s", ctime(&when)));
    utb.actime = when;
    utb.modtime = when;
    if (glue_utime(path->str_text, &utb))
    {
        sub_context_ty  *scp;
        int             errno_old;

        errno_old = errno;
        scp = sub_context_new();
        sub_errno_setx(scp, errno_old);
        sub_var_set_string(scp, "File_Name", path);
        error_intl(scp, i18n("warning: utime $filename: $errno"));
        sub_context_delete(scp);
    }
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
