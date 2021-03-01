/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001-2003 Peter Miller;
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
 * MANIFEST: functions to manipulate lists
 */

#include <ac/errno.h>
#include <ac/stddef.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <change/file.h>
#include <change.h>
#include <col.h>
#include <error.h>
#include <fstate.h>
#include <getgr_cache.h>
#include <getpw_cache.h>
#include <glue.h>
#include <list.h>
#include <now.h>
#include <os.h>
#include <output.h>
#include <project/file.h>
#include <project.h>
#include <stack.h>
#include <sub.h>

int		show_removed_files = -1;
int		show_dot_files = -1;
int		recursive_flag;
int		long_flag = 0;
int		mode_flag = -1;
static output_ty *mode_col;
int		attr_flag = -1;
static output_ty *attr_col;
int		user_flag = -1;
static output_ty *user_col;
int		group_flag = -1;
static output_ty *group_col;
int		size_flag = -1;
static output_ty *size_col;
int		when_flag = -1;
static output_ty *when_col;
static output_ty *name_col;
static col_ty	*col_ptr;
static string_list_ty dirs;
static change_ty *cp;
static project_ty *pp;
static time_t	oldest;
static time_t	youngest;


/*
 * NAME
 *	path_cat
 *
 * SYNOPSIS
 *	string_ty *path_cat(string_ty *lhs, string_ty *rhs);
 *
 * DESCRIPTION
 *	The path_cat function is used to join two strings together as
 *	a file name path.  If either is dot (".") the other is used.
 *	Only if both are not the current directory ore they glued together
 *	with a slash.
 */

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


/*
 * NAME
 *	stat_stack
 *
 * SYNOPSIS
 *	string_ty *stat_stack(string_ty *path, struct stat *st);
 *
 * DESCRIPTION
 *	The stat_stack function is used to walk the directory search
 *	stack, looking for the named file.  On success, the stat structure
 *	is filled in, and the resolved file name is returned.
 *
 *	It is a fatal error if the file cannot be found.
 */

static string_ty *
stat_stack(string_ty *path, struct stat *st)
{
    size_t	    j;
    string_ty	    *dir;
    string_ty	    *resolved_path;
    sub_context_ty  *scp;

    for (j = 0;; ++j)
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


/*
 * NAME
 *	readdir_stack
 *
 * SYNOPSIS
 *	void readdir_stack(string_ty *path, string_list_ty *result);
 *
 * DESCRIPTION
 *	The readdir_stack function is used to walk the directory search
 *	stack reading the named directory at each level.  The results are
 *	"unioned" together to provide a single view.
 */

static void
readdir_stack(string_ty *path, string_list_ty *result)
{
    size_t	    j;
    string_ty	    *s;
    string_ty	    *dir;
    char	    *data;
    long	    data_len;
    char	    *dp;

    string_list_constructor(result);
    for (j = 0;; ++j)
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

	dp = data;
	while (dp < data + data_len)
	{
	    s = str_from_c(dp);
	    dp += s->str_length + 1;
	    string_list_append_unique(result, s);
	    str_free(s);
	}
    }
}


static void
print_mode_column(struct stat *st)
{
    /*
     * First the type indicator
     */
    if (S_ISDIR(st->st_mode))
	output_fputc(mode_col, 'd');
    else if (S_ISCHR(st->st_mode))
	output_fputc(mode_col, 'c');
    else if (S_ISBLK(st->st_mode))
	output_fputc(mode_col, 'b');
#ifdef S_ISFIFO
    else if (S_ISFIFO(st->st_mode))
	output_fputc(mode_col, 'p');
#endif
#ifdef S_ISLNK
    else if (S_ISLNK(st->st_mode))
	output_fputc(mode_col, 'l');
#endif
#ifdef S_ISSOCK
    else if (S_ISSOCK(st->st_mode))
	output_fputc(mode_col, 's');
#endif
    else
	output_fputc(mode_col, '-');

    /*
     * Now the user bits
     */
    if (st->st_mode & S_IRUSR)
	output_fputc(mode_col, 'r');
    else
	output_fputc(mode_col, '-');
    if (st->st_mode & S_IWUSR)
	output_fputc(mode_col, 'w');
    else
	output_fputc(mode_col, '-');
    if (st->st_mode & S_IXUSR)
    {
	if (st->st_mode & S_ISUID)
	    output_fputc(mode_col, 's');
	else
	    output_fputc(mode_col, 'x');
    }
    else
    {
	if (st->st_mode & S_ISUID)
	    output_fputc(mode_col, 'S');
	else
	    output_fputc(mode_col, '-');
    }

    /*
     * Now the group bits
     */
    if (st->st_mode & S_IRGRP)
	output_fputc(mode_col, 'r');
    else
	output_fputc(mode_col, '-');
    if (st->st_mode & S_IWGRP)
	output_fputc(mode_col, 'w');
    else
	output_fputc(mode_col, '-');
    if (st->st_mode & S_IXGRP)
    {
	if (st->st_mode & S_ISGID)
	    output_fputc(mode_col, 's');
	else
	    output_fputc(mode_col, 'x');
    }
    else
    {
	if (st->st_mode & S_ISGID)
	    output_fputc(mode_col, 'S');
	else
	    output_fputc(mode_col, '-');
    }

    /*
     * Now the other bits
     */
    if (st->st_mode & S_IROTH)
	output_fputc(mode_col, 'r');
    else
	output_fputc(mode_col, '-');
    if (st->st_mode & S_IWOTH)
	output_fputc(mode_col, 'w');
    else
	output_fputc(mode_col, '-');
    if (st->st_mode & S_IXOTH)
    {
#ifdef S_ISVTX
	if (st->st_mode & S_ISVTX)
	    output_fputc(mode_col, 't');
	else
#endif
	    output_fputc(mode_col, 'x');
    }
    else
    {
#ifdef S_ISVTX
	if (st->st_mode & S_ISVTX)
	    output_fputc(mode_col, 'T');
	else
#endif
	    output_fputc(mode_col, '-');
    }
}


/*
 * NAME
 *	list_file
 *
 * SYNOPSIS
 *	void list_file(string_ty *long_name, string-ty *short_name,
 *		struct stat *st);
 *
 * DESCRIPTION
 *	The list_file function is used to print the information about
 *	the file onto the columnised output.
 */

static void
list_file(string_ty *long_name, string_ty *short_name, struct stat *st,
    string_ty *resolved_name)
{
    fstate_src	    c_src;
    fstate_src	    p_src;
    string_ty	    *link =	    0;

    c_src = cp ? change_file_find(cp, long_name) : 0;
    if (c_src && c_src->about_to_be_created_by)
	c_src = 0;
    if
    (
	show_removed_files <= 0
    &&
	c_src
    &&
	(c_src->deleted_by || c_src->action == file_action_remove)
    )
	return;

    p_src = project_file_find(pp, long_name, view_path_simple);
    if (p_src && p_src->about_to_be_created_by)
	p_src = 0;
    if
    (
	show_removed_files <= 0
    &&
	p_src
    &&
	(p_src->deleted_by || p_src->action == file_action_remove)
    )
	return;

    if (mode_col)
    {
	if (S_ISLNK(st->st_mode))
	{
	    string_ty	    *s2;

	    /*
	     * If a link points to a relative file of exactly
	     * the same name, then it's a link to a baseline.
	     * Pretend it isn't a line.
	     */
	    os_become_orig();
	    link = os_readlink(resolved_name);
	    os_become_undo();

	    if (link->str_text[0] != '/')
	    {
		string_ty	*s3;

		os_become_orig();
		s2 = os_dirname(resolved_name);
		os_become_undo();
		s3 = str_format("%S/%S", s2, link);
		str_free(s2);
		str_free(link);
		link = s3;
	    }
	    s2 = stack_relative(link);
	    if (s2)
	    {
		if (str_equal(s2, long_name))
		{
		    /* nuke the link-ness */
		    st->st_mode = (st->st_mode & ~S_IFMT) | S_IFREG;
		    str_free(link);
		    link = 0;
		}
		/* FIXME: need to re-relative the link, too */
		str_free(s2);
	    }
	}
	print_mode_column(st);
    }

    if (attr_col)
    {
	fstate_src	src;

	if (c_src)
	    output_fputc(attr_col, 'C');
	else if (p_src)
	    output_fputc(attr_col, 'P');
	else
	    output_fputc(attr_col, '-');

	if (c_src)
	{
	    if (c_src->action == file_action_create)
		output_fputc(attr_col, 'c');
	    else if (c_src->action == file_action_modify)
		output_fputc(attr_col, 'm');
	    else if (c_src->action == file_action_remove)
		output_fputc(attr_col, 'r');
	    else if (c_src->action == file_action_insulate)
		output_fputc(attr_col, 'i');
	    else
		output_fputc(attr_col, '?');
	}
	else
	    output_fputc(attr_col, '-');

	src = c_src ? c_src : p_src;
	if (src)
	{
	    if (src->usage == file_usage_build)
		output_fputc(attr_col, 'b');
	    else if (src->usage == file_usage_source)
		output_fputc(attr_col, 's');
	    else if (src->usage == file_usage_test)
		output_fputc(attr_col, 't');
	    else if (src->usage == file_usage_manual_test)
		output_fputc(attr_col, 'T');
	    else
		output_fputc(attr_col, '?');
	}
	else
	    output_fputc(attr_col, '-');
    }
    if (user_col)
    {
	struct passwd	*pw;

	pw = getpwuid_cached(st->st_uid);
	if (pw)
	    output_fputs(user_col, pw->pw_name);
	else
	    output_fprintf(user_col, "%d", (int)st->st_uid);
    }
    if (group_col)
    {
	struct group	*gr;

	gr = getgrgid_cached(st->st_uid);
	if (gr)
	    output_fputs(group_col, gr->gr_name);
	else
	    output_fprintf(group_col, "%d", (int)st->st_gid);
    }
    if (size_col)
    {
	output_fprintf(size_col, "%8ld", (long)st->st_size);
    }
    if (when_col)
    {
	char		buffer[100];
	struct tm	*tm;

	tm = localtime(&st->st_mtime);
	if (st->st_mtime < oldest || st->st_mtime > youngest)
	    strftime(buffer, sizeof(buffer), "%b %d  %Y", tm);
	else
	    strftime(buffer, sizeof(buffer), "%b %d %H:%M", tm);
	output_fputs(when_col, buffer);
    }

    /*
     * output the name
     */
    output_put_str(name_col, short_name);
    if (link)
    {
	if (long_flag)
	{
	    output_fputs(name_col, " -> ");
	    output_put_str(name_col, link);
	}
	str_free(link);
    }
    col_eoln(col_ptr);
}


/*
 * NAME
 *	list_dir
 *
 * SYNOPSIS
 *	void list_dir(string_ty *dirname);
 *
 * DESCRIPTION
 *	The list_dir function is used to list the contents of a directory
 *	onto the columnised output.
 */

static void
list_dir(string_ty *dirname)
{
    string_list_ty  wl;
    size_t	    j;
    string_list_ty  more_dirs;

    os_become_orig();
    readdir_stack(dirname, &wl);
    os_become_undo();
    string_list_sort(&wl);
    string_list_constructor(&more_dirs);
    for (j = 0; j < wl.nstrings; ++j)
    {
	string_ty	*s;
	struct stat	st;
	string_ty	*resolved_path;

	if (show_dot_files <= 0 && wl.string[j]->str_text[0] == '.')
	    continue;
	s = path_cat(dirname, wl.string[j]);
	os_become_orig();
	resolved_path = stat_stack(s, &st);
	os_become_undo();
	if (recursive_flag && (st.st_mode & S_IFMT) == S_IFDIR)
	    string_list_append(&more_dirs, s);
	list_file(s, wl.string[j], &st, resolved_path);
	str_free(resolved_path);
	str_free(s);
    }
    string_list_destructor(&wl);
    string_list_prepend_list(&dirs, &more_dirs);
    string_list_destructor(&more_dirs);
}


/*
 * NAME
 *	list
 *
 * SYNOPSIS
 *	void list(string_list_ty *paths);
 *
 * DESCRIPTION
 *	The list function is used to list the named files and the contents
 *	of the named directories onto the columnised output.
 */

void
list(string_list_ty *paths, project_ty *a_pp, change_ty *a_cp)
{
    size_t	    j;
    int		    need_eject;
    int		    column =	    0;
    int		    width;
    time_t	    when;

    when = now();
    oldest = when - 6L * 30 * 24 * 60 * 60;
    youngest = when + 6L * 30 * 24 * 60 * 60;
    pp = a_pp;
    cp = a_cp;

    if (long_flag > 0)
    {
	if (mode_flag < 0)
	    mode_flag = 1;
	if (attr_flag < 0)
	    attr_flag = 1;
	if (user_flag < 0)
	    user_flag = 1;
	if (group_flag < 0)
	    group_flag = 1;
	if (size_flag < 0)
	    size_flag = 1;
	if (when_flag < 0)
	    when_flag = 1;
    }

    col_ptr = col_open((string_ty *)0);
    if (mode_flag > 0)
    {
	width = 10;
	mode_col = col_create(col_ptr, column, column + width, "Mode");
	column += width + 1;
    }
    if (attr_flag)
    {
	width = 4;
	attr_col = col_create(col_ptr, column, column + width, "Attr");
	column += width + 1;
    }
    if (user_flag > 0)
    {
	width = 8;
	user_col =
	    col_create(col_ptr, column, column + width, "User\n--------");
	column += width + 1;
    }
    if (group_flag > 0)
    {
	width = 8;
	group_col =
	    col_create(col_ptr, column, column + width, "Group\n--------");
	column += width + 1;
    }
    if (size_flag > 0)
    {
	width = 8;
	size_col =
	    col_create(col_ptr, column, column + width, "Size\n--------");
	column += width + 1;
    }
    if (when_flag > 0)
    {
	width = 12;
	when_col =
	    col_create(col_ptr, column, column + width, "When\n------------");
	column += width + 1;
    }
    name_col = col_create(col_ptr, column, 0, "File Name\n-----------");
    col_title(col_ptr, "Annotated Listing", "");

    need_eject = 0;
    for (j = 0; j < paths->nstrings; ++j)
    {
	string_ty	*resolved_path;
	struct stat	st;
	string_ty	*path;

	path = paths->string[j];
	os_become_orig();
	resolved_path = stat_stack(path, &st);
	os_become_undo();
	if ((st.st_mode & S_IFMT) == S_IFDIR)
	    string_list_append(&dirs, path);
	else
	{
	    need_eject = 1;
	    list_file(path, path, &st, resolved_path);
	}
	str_free(resolved_path);
    }

    while (dirs.nstrings)
    {
	string_ty	*path;

	if (need_eject)
	    col_eject(col_ptr);
	path = str_copy(dirs.string[0]);
	string_list_remove(&dirs, path);
	col_title(col_ptr, "Annotated Directory Listing", path->str_text);
	list_dir(path);
	need_eject = 1;
    }
    col_close(col_ptr);
}
