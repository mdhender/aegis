/*
 *	aegis - project change supervisor
 *	Copyright (C) 1997, 1999, 2002 Peter Miller;
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
 * MANIFEST: functions to walk parallel directory trees
 */

#include <ac/errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <cmdline.h>
#include <descend.h>
#include <glue.h>
#include <str_list.h>
#include <sub.h>
#include <trace.h>
#include <tree.h>


static string_ty *
path_cat(string_ty *s1, string_ty *s2)
{
    static string_ty *dot;

    if (!dot)
	dot = str_from_c(".");
    if (str_equal(s1, dot))
	return str_copy(s2);
    if (str_equal(s2, dot))
	return str_copy(s1);
    return str_format("%S/%S", s1, s2);
}


string_ty *
stat_stack(string_ty *path, struct stat *st)
{
    size_t	    j;
    string_ty	    *dir;
    string_ty	    *resolved_path;
    sub_context_ty  *scp;

    for (j = 0; ; ++j)
    {
	dir = stack_nth(j);
	if (!dir)
	    break;
	resolved_path = path_cat(dir, path);

#ifdef S_IFLNK
	if (!glue_lstat(resolved_path->str_text, st))
	    return resolved_path;

	if (errno != ENOENT)
	{
	    scp = sub_context_new();
	    sub_errno_set(scp);
	    sub_var_set_string(scp, "File_Name", resolved_path);
	    fatal_intl(scp, i18n("lstat $filename: $errno"));
	    /* NOTREACHED */
	}
#else
	if (!glue_stat(resolved_path->str_text, st))
	    return resolved_path;

	if (errno != ENOENT)
	{
	    scp = sub_context_new();
	    sub_errno_set(scp);
	    sub_var_set_string(scp, "File_Name", resolved_path);
	    fatal_intl(scp, i18n("stat $filename: $errno"));
	    /* NOTREACHED */
	}
#endif
	str_free(resolved_path);
    }
    scp = sub_context_new();
    sub_errno_setx(scp, ENOENT);
    sub_var_set_string(scp, "File_Name", path);
    fatal_intl(scp, i18n("stat $filename: $errno"));
    /* NOTREACHED */
    return 0;
}


static void
readdir_stack(string_ty *path, string_list_ty *result)
{
    size_t	    j;
    string_ty	    *s;
    string_ty	    *dir;
    char	    *data;
    long	    data_len;
    char	    *cp;

    string_list_constructor(result);
    for (j = 0; ; ++j)
    {
	dir = stack_nth(j);
	if (!dir)
	    break;
	s = path_cat(dir, path);
	if (glue_read_whole_dir(s->str_text, &data, &data_len))
	{
	    sub_context_ty  *scp;

	    if (errno == ENOENT)
	    {
		str_free(s);
		continue;
	    }

	    scp = sub_context_new();
	    sub_errno_set(scp);
	    sub_var_set_string(scp, "File_Name", path);
	    fatal_intl(scp, i18n("read $filename: $errno"));
	    /* NOTREACHED */
	}
	str_free(s);

	cp = data;
	while (cp < data + data_len)
	{
	    s = str_from_c(cp);
	    cp += s->str_length + 1;
	    string_list_append_unique(result, s);
	    str_free(s);
	}
    }
}


void
descend(string_ty *path, int resolve, descend_callback_ty callback, void *arg)
{
    string_list_ty  wl;
    struct stat     st;
    size_t	    j;
    string_ty	    *s;
    string_ty	    *resolved_path;

    trace(("descend(path = %08lX, callback = %08lX, arg = %08lX)\n{\n",
	(long)path, (long)callback, (long)arg));
    trace_string(path->str_text);
    resolved_path = stat_stack(path, &st);
    if ((st.st_mode & S_IFMT) == S_IFDIR)
    {
	callback
	(
	    arg,
	    descend_message_dir_before,
	    (resolve ? resolved_path : path),
	    &st
	);
	readdir_stack(path, &wl);
	for (j = 0; j < wl.nstrings; ++j)
	{
	    s = path_cat(path, wl.string[j]);
	    descend(s, resolve, callback, arg);
	    str_free(s);
	}
	string_list_destructor(&wl);
	callback
	(
	    arg,
	    descend_message_dir_after,
	    (resolve ? resolved_path : path),
	    &st
	);
    }
    else
    {
	callback
	(
    	    arg,
    	    descend_message_file,
    	    (resolve ? resolved_path : path),
    	    &st
	);
    }
    str_free(resolved_path);
    trace(("}\n"));
}
