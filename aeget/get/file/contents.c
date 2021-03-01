/*
 *	aegis - project change supervisor
 *	Copyright (C) 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate contentss
 */

#include <ac/stdio.h>
#include <ac/string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <change.h>
#include <change/branch.h>
#include <change/file.h>
#include <dir_stack.h>
#include <error.h>
#include <file.h>
#include <get/file/contents.h>
#include <http.h>
#include <now.h>
#include <os.h>
#include <project/file.h>
#include <project/file/roll_forward.h>
#include <project.h>
#include <str_list.h>


static void
no_such_file(string_ty *filename)
{
    http_fatal("%s: no such file", filename->str_text);
}


static void
emit_path(string_ty *filename, string_ty *from)
{
    string_ty       *to;

    /*
     * Print the output header.
     *
     * We use the "from" name (even though it could be an uninformative
     * name in /tmp) so that the file can be opened and stanned.
     *
     * No need for a Content-Length header, because fork_and_watch will
     * add it automagically.
     */
    http_content_type_header(from);

    /*
     * Now copy the file contents to stdout.
     *
     * FIXME: what about modifiers for line numbering, syntax
     * highlighting, etc?
     */
    to = str_from_c("");
    copy_whole_file(from, to, 0);
    str_free(to);
}


static void
emit_file(string_ty *filename, struct stat *st)
{
    string_ty       *to;

    /*
     * Print the output header.
     */
    if (st)
	printf("Content-Length: %ld\n", st->st_size);
    http_content_type_header(filename);

    /*
     * Now copy the file contents to stdout.
     *
     * FIXME: what about modifiers for line numbering, syntax
     * highlighting, etc?
     */
    to = str_from_c("");
    copy_whole_file(filename, to, 0);
    str_free(to);
}


static string_ty *
hunt(string_list_ty *gizzards, const char *name)
{
    size_t          j;
    string_ty       *s;

    for (j = 0; j < gizzards->nstrings; ++j)
    {
	s = gizzards->string[j];
	if (0 == strcasecmp(s->str_text, name))
	    return s;
    }
    return 0;
}


static string_ty *
cat4(const char *s1, string_ty *s2, string_ty *s3, string_ty *s4)
{
    string_ty       *temp1;
    string_ty       *temp2;
    string_ty       *temp3;
    string_ty       *result;

    temp1 = str_from_c(s1);
    temp2 = os_path_cat(temp1, s2);
    temp3 = os_path_cat(temp2, s3);
    result = os_path_cat(temp3, s4);
    str_free(temp1);
    str_free(temp2);
    str_free(temp3);
    return result;
}


static const char *
mode_string(int mode)
{
    static char     buffer[11];

    buffer[0] = '?';
    switch (mode & S_IFMT)
    {
    case S_IFDIR: buffer[0] = 'd'; break;
    case S_IFREG: buffer[0] = '-'; break;
    case S_IFLNK: buffer[0] = 'l'; break;
    }
    buffer[1] = (mode & 0400) ? 'r' : '-';
    buffer[2] = (mode & 0200) ? 'w' : '-';
    buffer[3] = "-xSs"[((mode >> 6) & 1) | ((mode >> 10) & 2)];
    buffer[4] = (mode & 0040) ? 'r' : '-';
    buffer[5] = (mode & 0020) ? 'w' : '-';
    buffer[6] = "-xSs"[((mode >> 3) & 1) | ((mode >> 9) & 2)];
    buffer[7] = (mode & 0004) ? 'r' : '-';
    buffer[8] = (mode & 0002) ? 'w' : '-';
    buffer[9] = "-xTt"[(mode & 1) | ((mode >> 8) & 2)];
    buffer[10] = 0;
    return buffer;
}


static void
emit_dir(change_ty *cp, string_list_ty *search_path, string_ty *filename)
{
    string_list_ty  gizzards;
    size_t          j;
    time_t          time_split;
    string_ty       *idx;

    /*
     * Read the directory contents.
     */
    dir_stack_readdir(search_path, filename, &gizzards);
    string_list_sort(&gizzards);

    /*
     * FIXME: this would benefit from a modifier to say that the index
     * file should not be looked for, but the raw directory given instead.
     */
    idx = hunt(&gizzards, "index.html");
    if (!idx)
	idx = hunt(&gizzards, "index.htm");
    if (!idx)
	idx = hunt(&gizzards, "welcome.html");
    if (!idx)
	idx = hunt(&gizzards, "welcome.htm");
    if (idx)
    {
	string_ty       *path;
	string_ty       *abs_path;
	struct stat     st;

	path = os_path_cat(filename, idx);
	abs_path = dir_stack_find(search_path, 0, path, &st, 0, 1);
	str_free(path);
	if (abs_path)
	{
	    if (cp->bogus)
	    {
		idx =
		    cat4
		    (
			http_script_name(),
			project_name_get(cp->pp),
			filename,
			idx
		    );
	    }
	    else
	    {
		string_ty       *s;

		s =
		    str_format
		    (
			"%S.C%ld",
			project_name_get(cp->pp),
			cp->number
		    );
		idx = cat4(http_script_name(), s, filename, idx);
		str_free(s);
	    }
	    printf("Location: ");
	    html_escape_string(idx);
	    str_free(idx);
	    printf("\n\n");

	    string_list_destructor(&gizzards);
	    return;
	}
    }

    /*
     * Print the output header.
     */
    html_header(cp->pp);
    printf("<title>Project ");
    html_encode_string(project_name_get(cp->pp));
    if (!cp->bogus)
	printf(",\nChange %ld", cp->number);
    printf(",\nDirectory ");
    html_encode_string(filename);
    printf("</title></head>\n<body><h1 align=center>\n");
    emit_change(cp);
    printf(",<br>\nDirectory \"");
    /* FIXME: link each part, too. */
    html_encode_string(filename);
    printf("\"\n</h1>\n");

    printf("<table>\n");
    printf("<tr><th>Mode</th><th colspan=2>Date</th><th>Size</th>");
    printf("<th>Filename</th></tr>\n");

    time_split = now() - 6*30*24*60*60;
    for (j = 0; j < gizzards.nstrings; ++j)
    {
	string_ty       *deeper;
	string_ty       *ref;
	string_ty       *abs_path;
	struct stat     st;
	string_ty       *s;

	deeper = os_path_cat(filename, gizzards.string[j]);
	abs_path = dir_stack_find(search_path, 0, deeper, &st, 0, 1);
	if (!abs_path)
	    continue;
	str_free(abs_path);

	printf("<tr>\n");

	printf("<td valign=top><tt>");
	html_encode_charstar(mode_string(st.st_mode));
	printf("</tt></td>\n");

	printf("<td valign=top>");
	{
	    struct tm *tm;
	    char buffer[100];

	    tm = localtime(&st.st_mtime);
	    strftime(buffer, sizeof(buffer), "%b %e", tm);
	    html_encode_charstar(buffer);
	    printf("</td><td valign=top>");
	    if (st.st_mtime < time_split)
		strftime(buffer, sizeof(buffer), "%Y", tm);
	    else
		strftime(buffer, sizeof(buffer), "%H:%M", tm);
	    html_encode_charstar(buffer);
	}
	printf("</td>\n");

	printf("<td valign=\"top\" align=\"right\">");
	printf("%ld", st.st_size);
	printf("</td>\n");

	printf("<td valign=\"top\">\n<a href=\"");
	if (cp->bogus)
	{
	    ref =
		cat4
		(
		    http_script_name(),
		    project_name_get(cp->pp),
		    filename,
		    gizzards.string[j]
		);
	}
	else
	{
	    s = str_format("%S.C%ld", project_name_get(cp->pp), cp->number);
	    ref = cat4(http_script_name(), s, filename, gizzards.string[j]);
	    str_free(s);
	}
	html_escape_string(ref);
	str_free(ref);
	if ((st.st_mode & S_IFMT) == S_IFDIR)
	    printf("/");
	/* FIXME: link to @@menu for project and/or change files? */
	printf("\">\n");
	html_encode_string(gizzards.string[j]);
	printf("</a>");
	if ((st.st_mode & S_IFMT) == S_IFDIR)
	    printf("/");
	printf("</td>\n");

	s = os_path_cat(filename, gizzards.string[j]);
	if (!cp->bogus && change_file_find(cp, s))
	{
	    printf("<td valign=\"top\">(");
	    emit_file_href(cp, s, "file@menu");
	    printf("Menu</a>)</td>\n");
	}
	else if (project_file_find(cp->pp, s, view_path_simple))
	{
	    int             hold_bogus;

	    hold_bogus = cp->bogus;
	    cp->bogus = 1;
	    printf("<td valign=\"top\">(");
	    emit_file_href(cp, s, "file@menu");
	    printf("Menu</a>)</td>\n");
	    cp->bogus = hold_bogus;
	}
	str_free(s);
	printf("</tr>\n");
    }

    printf("</table>\n");
    printf("Listed %lu files.\n", (unsigned long)gizzards.nstrings);
    string_list_destructor(&gizzards);
    html_footer();
}


void
get_file_contents(change_ty *cp, string_ty *filename, string_list_ty *modifier)
{
    string_list_ty  search_path;
    string_ty       *absolute_path;
    struct stat     st;
    fstate_src_ty   *src;

    /*
     * Make sure if the file actually exists in the given change
     * (project) that you fetch the appropriate historical version,
     * not necessarily the one in the file system.
     */
    if (!cp->bogus)
    {
	src = change_file_find(cp, filename);
	if (src)
	{
	    switch (src->action)
	    {
	    case file_action_remove:
		no_such_file(filename);
		break;

	    case file_action_create:
	    case file_action_modify:
	    case file_action_insulate:
	    case file_action_transparent:
#ifndef DEBUG
	    default:
#endif
		if (change_is_completed(cp))
		{
		    int             delete_me;
		    string_ty       *s;

		    /*
		     * We need historical version.  We could get lucky,
		     * it could be in the baseline or one of the ancestor
		     * baselines.
		     */
		    s = project_file_version_path(cp->pp, src, &delete_me);
		    assert(s);
		    if (!s)
		    {
			no_such_file(filename);
			return;
		    }
		    os_become_orig();
		    emit_path(filename, s);
		    if (delete_me)
			os_unlink_errok(s);
		    os_become_undo();
		    str_free(s);
		}
		else
		{
		    string_ty       *s;

		    /*
		     * We can use the version in the development directory.
		     */
		    s = change_file_path(cp, filename);
		    os_become_orig();
		    emit_path(filename, s);
		    os_become_undo();
		    str_free(s);
		}
		break;
	    }
	    return;
	}

	/*
	 * If the change is completed, look for the file as it would
	 * have appeared at the time the change was integrated.
	 */
	if
	(
	    change_is_completed(cp)
	&&
	    project_file_find(cp->pp, filename, view_path_simple)
	)
	{
	    time_t          when;
	    file_event_ty   *fep;
	    string_ty       *s;
	    int             delete_me;

	    when = change_completion_timestamp(cp);
	    project_file_roll_forward(cp->pp, when, 0);
	    fep = project_file_roll_forward_get_last(filename);
	    if (!fep)
	    {
		/*
		 * The file doesn't exist yet at this
		 * delta.  Omit it.
		 */
		no_such_file(filename);
		return;
	    }
	    src = change_file_find(fep->cp, filename);
	    assert(src);

	    s = project_file_version_path(cp->pp, src, &delete_me);
	    assert(s);
	    if (!s)
	    {
		no_such_file(filename);
		return;
	    }
	    os_become_orig();
	    emit_path(filename, s);
	    if (delete_me)
		os_unlink_errok(s);
	    os_become_undo();
	    str_free(s);
	    return;
	}
    }
    src = project_file_find(cp->pp, filename, view_path_simple);
    if (src)
    {
	switch (src->action)
	{
	case file_action_remove:
	    no_such_file(filename);
	    break;

	case file_action_create:
	case file_action_modify:
	case file_action_insulate:
	case file_action_transparent:
#ifndef DEBUG
	default:
#endif
	    {
		int             delete_me;
		string_ty       *s;

		s = project_file_version_path(cp->pp, src, &delete_me);
		assert(s);
		os_become_orig();
		emit_path(filename, s);
		if (delete_me)
		    os_unlink_errok(s);
		os_become_undo();
		str_free(s);
	    }
	    break;
	}
	return;
    }

    /*
     * Build the search path.
     */
    if (cp->bogus)
    {
	string_list_constructor(&search_path);
	project_search_path_get(cp->pp, &search_path, 0);
    }
    else
    {
	change_search_path_get(cp, &search_path, 0);
    }

    os_become_orig();
    absolute_path = dir_stack_find(&search_path, 0, filename, &st, 0, 1);
    if (!absolute_path)
	no_such_file(filename);
    switch (st.st_mode & S_IFMT)
    {
    case S_IFDIR:
	emit_dir(cp, &search_path, filename);
	break;

    case S_IFREG:
	emit_file(absolute_path, &st);
	break;

    default:
	http_fatal("Not an appropriate file.");
    }
    str_free(absolute_path);
    os_become_undo();
    string_list_destructor(&search_path);
}
