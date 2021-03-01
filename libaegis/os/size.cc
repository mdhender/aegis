//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2002-2006, 2008, 2012 Peter Miller
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
#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h>

#include <libaegis/glue.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <common/trace.h>


off_t
os_file_size(string_ty *path)
{
    trace(("os_mtime_actual(path = %p)\n{\n", path));
    os_become_must_be_active();
    trace_string(path->str_text);

    struct stat st;
    int oret = glue_stat(path->str_text, &st);
    if (oret)
    {
        int errno_old = errno;
        sub_context_ty sc;
        sc.errno_setx(errno_old);
        sc.var_set_string("File_Name", path);
        sc.fatal_intl(i18n("stat $filename: $errno"));
        // NOTREACHED
    }

    trace(("return %ld;\n", (long)st.st_size));
    trace(("}\n"));
    return st.st_size;
}


// vim: set ts=8 sw=4 et :
