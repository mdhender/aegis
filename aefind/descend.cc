//
// aegis - project change supervisor
// Copyright (C) 1997, 1999, 2002-2006, 2008, 2011, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/errno.h>
#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h>

#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/glue.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>

#include <aefind/cmdline.h>
#include <aefind/descend.h>
#include <aefind/tree.h>


string_ty *
stat_stack(string_ty *path, struct stat *st)
{
    size_t          j;
    string_ty       *dir;
    string_ty       *resolved_path;
    sub_context_ty  *scp;

    for (j = 0; ; ++j)
    {
        dir = stack_nth(j);
        if (!dir)
            break;
        resolved_path = os_path_cat(dir, path);

#ifdef S_IFLNK
        if (!glue_lstat(resolved_path->str_text, st))
            return resolved_path;

        if (errno != ENOENT)
        {
            int             errno_old;

            errno_old = errno;
            scp = sub_context_new();
            sub_errno_setx(scp, errno_old);
            sub_var_set_string(scp, "File_Name", resolved_path);
            fatal_intl(scp, i18n("lstat $filename: $errno"));
            // NOTREACHED
        }
#else
        if (!glue_stat(resolved_path->str_text, st))
            return resolved_path;

        if (errno != ENOENT)
        {
            int             errno_old;

            errno_old = errno;
            scp = sub_context_new();
            sub_errno_setx(scp, errno_old);
            sub_var_set_string(scp, "File_Name", resolved_path);
            fatal_intl(scp, i18n("stat $filename: $errno"));
            // NOTREACHED
        }
#endif
        str_free(resolved_path);
    }
    scp = sub_context_new();
    sub_errno_setx(scp, ENOENT);
    sub_var_set_string(scp, "File_Name", path);
    fatal_intl(scp, i18n("stat $filename: $errno"));
    // NOTREACHED
    return 0;
}


static void
readdir_stack(string_ty *path, string_list_ty *result)
{
    size_t          j;
    string_ty       *s;
    string_ty       *dir;

    result->clear();
    for (j = 0; ; ++j)
    {
        dir = stack_nth(j);
        if (!dir)
            break;
        s = os_path_cat(dir, path);
        if (read_whole_dir__wla(s->str_text, result))
        {
            sub_context_ty  *scp;
            int             errno_old;

            errno_old = errno;
            if (errno_old == ENOENT)
            {
                str_free(s);
                continue;
            }
            scp = sub_context_new();
            sub_errno_setx(scp, errno_old);
            sub_var_set_string(scp, "File_Name", path);
            fatal_intl(scp, i18n("read $filename: $errno"));
            // NOTREACHED
        }
        str_free(s);
    }
}


void
descend(string_ty *path, int resolve, descend_callback_ty callback, void *arg)
{
    struct stat     st;
    size_t          j;
    string_ty       *s;
    string_ty       *resolved_path;

    trace(("descend(path = %08lX, callback = %08lX, arg = %08lX)\n{\n",
        (long)path, (long)callback, (long)arg));
    trace_string(path->str_text);
    resolved_path = stat_stack(path, &st);
    if ((st.st_mode & S_IFMT) == S_IFDIR)
    {
        trace(("mark\n"));
        callback
        (
            arg,
            descend_message_dir_before,
            path,
            (resolve ? resolved_path : path),
            resolved_path,
            &st
        );
        trace(("mark\n"));
        string_list_ty wl;
        readdir_stack(path, &wl);
        for (j = 0; j < wl.nstrings; ++j)
        {
            s = os_path_cat(path, wl.string[j]);
            if (!stack_eliminate(s))
                descend(s, resolve, callback, arg);
            str_free(s);
        }
        trace(("mark\n"));
        callback
        (
            arg,
            descend_message_dir_after,
            path,
            (resolve ? resolved_path : path),
            resolved_path,
            &st
        );
    }
    else
    {
        trace(("mark\n"));
        callback
        (
            arg,
            descend_message_file,
            path,
            (resolve ? resolved_path : path),
            resolved_path,
            &st
        );
    }
    str_free(resolved_path);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
