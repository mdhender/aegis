//
//	aegis - project change supervisor
//	Copyright (C) 1991-1995, 1997, 1999, 2002-2004 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to walk directory trees
//

#include <ac/errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <dir.h>
#include <error.h>
#include <glue.h>
#include <os.h>
#include <sub.h>
#include <trace.h>
#include <str_list.h>


void
dir_walk(string_ty *path, dir_walk_callback_ty callback, void *arg)
{
    string_list_ty  wl;
    struct stat     st;
    size_t          j;
    string_ty       *s;

    trace(("dir_walk(path = %08lX, callback = %08lX, arg = %08lX)\n{\n",
	(long)path, (long)callback, (long)arg));
    assert(path);
    assert(callback);
    trace_string(path->str_text);
#if defined(S_IFLNK) || defined(S_ISLNK)
    if (glue_lstat(path->str_text, &st))
    {
	sub_context_ty	*scp;
	int             errno_old;

	errno_old = errno;
	scp = sub_context_new();
	sub_errno_setx(scp, errno_old);
	sub_var_set_string(scp, "File_Name", path);
	fatal_intl(scp, i18n("lstat $filename: $errno"));
	// NOTREACHED
    }
#else
    if (glue_stat(path->str_text, &st))
    {
	sub_context_ty	*scp;
	int             errno_old;

	errno_old = errno;
	scp = sub_context_new();
	sub_errno_setx(scp, errno_old);
	sub_var_set_string(scp, "File_Name", path);
	fatal_intl(scp, i18n("stat $filename: $errno"));
	// NOTREACHED
    }
#endif
    switch (st.st_mode & S_IFMT)
    {
    case S_IFDIR:
	callback(arg, dir_walk_dir_before, path, &st);
	if (read_whole_dir__wl(path->str_text, &wl))
	{
	    sub_context_ty  *scp;
	    int             errno_old;

	    errno_old = errno;
	    scp = sub_context_new();
	    sub_errno_setx(scp, errno_old);
	    sub_var_set_string(scp, "File_Name", path);
	    fatal_intl(scp, i18n("read $filename: $errno"));
	    // NOTREACHED
	}

	for (j = 0; j < wl.nstrings; ++j)
	{
	    s = os_path_join(path, wl.string[j]);
	    dir_walk(s, callback, arg);
	    str_free(s);
	}
	string_list_destructor(&wl);
	callback(arg, dir_walk_dir_after, path, &st);
	break;

    case S_IFREG:
	callback(arg, dir_walk_file, path, &st);
	break;

#if defined(S_IFLNK) || defined(S_ISLNK)
    case S_IFLNK:
	callback(arg, dir_walk_symlink, path, &st);
	break;
#endif

    default:
	callback(arg, dir_walk_special, path, &st);
	break;
    }
    trace(("}\n"));
}
