/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate dir_stacks
 */

#include <ac/errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <dir_stack.h>
#include <error.h>
#include <glue.h>
#include <os.h>
#include <sub.h>
#include <trace.h>
#include <str_list.h>


static string_ty *path_cat _((string_ty *, string_ty *));

static string_ty *
path_cat(s1, s2)
    string_ty       *s1;
    string_ty       *s2;
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
dir_stack_find(stack, start_pos, path, st, depth_p, ignore_symlinks)
    string_list_ty  *stack;
    size_t          start_pos;
    string_ty       *path;
    struct stat     *st;
    int             *depth_p;
    int             ignore_symlinks;
{
    size_t          j;
    string_ty       *dir;
    string_ty       *resolved_path;
    sub_context_ty  *scp;
    struct stat     bogus;
    string_ty       *maybe_pathname = 0;
    int             maybe_depth = 0;
    struct stat     maybe_statbuf;

    trace(("dir_stack_find(path = \"%s\")\n{\n", path->str_text));
    if (!st)
	st = &bogus;
    for (j = start_pos; j < stack->nstrings; ++j)
    {
	dir = stack->string[j];
	resolved_path = path_cat(dir, path);

#ifdef S_IFLNK
	if (!glue_lstat(resolved_path->str_text, st))
	{
	    if (S_ISLNK(st->st_mode) && ignore_symlinks)
	    {
		/*
		 * This is a symbolic link, and we have been told to
		 * ignore symbolic links.
		 *
		 * If this is the first symlink of this name (i.e. the
		 * shallowest in the view path) remember it, otherwise
		 * simply ignore it.
		 *
		 * Either way, keep looking down the view path.
		 */
		if (!maybe_pathname)
		{
		    maybe_statbuf = *st;
		    maybe_pathname = str_copy(resolved_path);
		    maybe_depth = j;
		}
		str_free(resolved_path);
		continue;
	    }
	    if (depth_p)
	    {
		int depth = j;
		if (maybe_pathname && maybe_depth == 0)
		    depth |= TOP_LEVEL_SYMLINK;
		*depth_p = depth;
	    }
	    if (maybe_pathname)
		str_free(maybe_pathname);
	    trace(("}\n"));
	    return resolved_path;
	}

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
	{
	    if (depth_p)
		*depth_p = j;
	    trace(("}\n"));
	    return resolved_path;
	}

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
    if (maybe_pathname)
    {
	/*
	 * We didn't find a regular file, but we did find
	 * a symlink.  Return the symlink.
	 */
	*st = maybe_statbuf;
	if (depth_p)
	    *depth_p = maybe_depth;
	trace(("}\n"));
	return maybe_pathname;
    }
    trace(("}\n"));
    return 0;
}


void
dir_stack_stat(stack, path, st, depth_p, ignore_symlinks)
    string_list_ty  *stack;
    string_ty       *path;
    struct stat     *st;
    int             *depth_p;
    int             ignore_symlinks;
{
    string_ty       *result;

    trace(("dir_stack_stat(path = \"%s\")\n{\n", path->str_text));
    result = dir_stack_find(stack, 0, path, st, depth_p, ignore_symlinks);
    if (!result)
    {
	sub_context_ty *scp;

	scp = sub_context_new();
	sub_errno_setx(scp, ENOENT);
	sub_var_set_string(scp, "File_Name", path);
	fatal_intl(scp, i18n("stat $filename: $errno"));
	/* NOTREACHED */
    }
    str_free(result);
    trace(("}\n"));
}


void
dir_stack_readdir(stack, path, result)
    string_list_ty  *stack;
    string_ty       *path;
    string_list_ty  *result;
{
    size_t          j;
    string_ty       *s;
    string_ty       *dir;
    char            *data;
    long            data_len;
    char            *cp;

    trace(("dir_stack_readdir(path = \"%s\")\n{\n", path->str_text));
    string_list_constructor(result);
    for (j = 0; j < stack->nstrings; ++j)
    {
	dir = stack->string[j];
	s = path_cat(dir, path);
	trace(("s = \"%s\";\n", s->str_text));
	if (glue_read_whole_dir(s->str_text, &data, &data_len))
	{
	    sub_context_ty *scp;

	    if (errno == ENOENT)
	    {
		str_free(s);
		trace(("mark\n"));
		continue;
	    }

	    scp = sub_context_new();
	    sub_errno_set(scp);
	    sub_var_set_string(scp, "File_Name", path);
	    fatal_intl(scp, i18n("read $filename: $errno"));
	    /* NOTREACHED */
	}
	str_free(s);
	trace(("data = %lX\n", (long)data));
	trace(("data_len = %ld\n", (long)data_len));

	cp = data;
	while (cp < data + data_len)
	{
	    s = str_from_c(cp);
	    trace(("s = \"%s\";\n", s->str_text));
	    cp += s->str_length + 1;
	    string_list_append_unique(result, s);
	    str_free(s);
	}
	/* do NOT free what data is pointing to */
	trace(("mark\n"));
    }
    trace(("}\n"));
}


void
dir_stack_walk(stack, path, callback, arg, ignore_symlinks)
    string_list_ty  *stack;
    string_ty       *path;
    dir_stack_walk_callback_t callback;
    void            *arg;
    int             ignore_symlinks;
{
    string_list_ty  wl;
    struct stat     st;
    int             j;
    string_ty       *s;
    int             depth;

    trace(("dir_stack_walk(path = %08lX, callback = %08lX, arg = %08lX, "
	"ign = %d)\n{\n", (long)path, (long)callback, (long)arg,
	ignore_symlinks));
    trace_string(path->str_text);
    dir_stack_stat(stack, path, &st, &depth, ignore_symlinks);
    switch (st.st_mode & S_IFMT)
    {
    case S_IFDIR:
	callback(arg, dir_stack_walk_dir_before, path, &st, depth,
	    ignore_symlinks);
	string_list_constructor(&wl);
	dir_stack_readdir(stack, path, &wl);
	trace(("mark\n"));
	for (j = 0; j < wl.nstrings; ++j)
	{
	    s = path_cat(path, wl.string[j]);
	    trace(("s = \"%s\";\n", s->str_text));
	    dir_stack_walk(stack, s, callback, arg, ignore_symlinks);
	    str_free(s);
	    trace(("mark\n"));
	}
	string_list_destructor(&wl);
	trace(("mark\n"));
	callback(arg, dir_stack_walk_dir_after, path, &st, depth,
	    ignore_symlinks);
	trace(("mark\n"));
	break;

    case S_IFREG:
	trace(("mark\n"));
	callback(arg, dir_stack_walk_file, path, &st, depth, ignore_symlinks);
	break;

#if defined(S_IFLNK) || defined(S_ISLNK)
    case S_IFLNK:
	trace(("mark\n"));
	callback(arg, dir_stack_walk_symlink, path, &st, depth,
	    ignore_symlinks);
	break;
#endif

    default:
	trace(("mark\n"));
	callback(arg, dir_stack_walk_special, path, &st, depth,
	    ignore_symlinks);
	break;
    }
    trace(( /*{ */ "}\n"));
}


string_ty *
dir_stack_relative(stack, path)
    string_list_ty  *stack;
    string_ty       *path;
{
    size_t          j;
    string_ty       *s;

    trace(("dir_stack_relative(path = \"%s\")\n{\n", path->str_text));
    for (j = 0; j < stack->nstrings; ++j)
    {
	s = os_below_dir(stack->string[j], path);
	if (s)
	{
	    trace(("return \"%s\";\n", s->str_text));
	    trace(("}\n"));
	    return s;
	}
    }
    trace(("return NULL;\n"));
    trace(("}\n"));
    return 0;
}
