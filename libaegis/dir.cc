//
//      aegis - project change supervisor
//      Copyright (C) 1991-1995, 1997, 1999, 2002-2006, 2008, 2012 Peter Miller
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
//      along with this program; if not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>
#include <common/ac/errno.h>
#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h>

#include <common/nstring/list.h>
#include <common/trace.h>
#include <libaegis/dir.h>
#include <libaegis/dir/functor/callback.h>
#include <libaegis/glue.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>


void
dir_walk(string_ty *path, dir_walk_callback_ty callback, void *arg)
{
    trace(("dir_walk(path = %p, callback = %p, arg = %p)\n{\n",
        path, callback, arg));
    assert(path);
    trace_string(path->str_text);
    assert(callback);
    dir_functor_callback compatability(callback, arg);
    dir_walk(nstring(path), compatability);
    trace(("}\n"));
}


void
dir_walk(const nstring &path, dir_functor &doit)
{
    trace(("dir_walk(path = \"%s\", doit = %p)\n{\n", path.c_str(),
        &doit));
    assert(!path.empty());
    struct stat st;
#if defined(S_IFLNK) || defined(S_ISLNK)
    if (glue_lstat(path.c_str(), &st))
    {
        int errno_old = errno;
        sub_context_ty sc;
        sc.errno_setx(errno_old);
        sc.var_set_string("File_Name", path);
        sc.fatal_intl(i18n("lstat $filename: $errno"));
        // NOTREACHED
    }
#else
    if (glue_stat(path.c_str(), &st))
    {
        int errno_old = errno;
        sub_context_ty sc;
        sc.errno_setx(errno_old);
        sc.var_set_string("File_Name", path);
        sc.fatal_intl(i18n("stat $filename: $errno"));
        // NOTREACHED
    }
#endif
    switch (st.st_mode & S_IFMT)
    {
    case S_IFDIR:
        {
            doit(dir_functor::msg_dir_before, path, st);
            nstring_list wl;
            if (read_whole_dir__wl(path, wl))
            {
                int errno_old = errno;
                sub_context_ty sc;
                sc.errno_setx(errno_old);
                sc.var_set_string("File_Name", path);
                sc.fatal_intl(i18n("read $filename: $errno"));
                // NOTREACHED
            }

            for (size_t j = 0; j < wl.size(); ++j)
            {
                nstring s = os_path_join(path, wl[j]);
                dir_walk(s, doit);
            }
            doit(dir_functor::msg_dir_after, path, st);
        }
        break;

    case S_IFREG:
        doit(dir_functor::msg_file, path, st);
        break;

#if defined(S_IFLNK) || defined(S_ISLNK)
    case S_IFLNK:
        doit(dir_functor::msg_symlink, path, st);
        break;
#endif

    default:
        doit(dir_functor::msg_special, path, st);
        break;
    }
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
