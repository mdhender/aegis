//
//	aegis - project change supervisor
//	Copyright (C) 2003-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/stdio.h>
#include <common/ac/string.h>
#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h>

#include <common/error.h>
#include <common/now.h>
#include <common/str_list.h>
#include <libaegis/attribute.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/change.h>
#include <libaegis/dir_stack.h>
#include <libaegis/file/event.h>
#include <libaegis/file.h>
#include <libaegis/http.h>
#include <libaegis/os.h>
#include <libaegis/project/file.h>
#include <libaegis/project/file/roll_forward.h>
#include <libaegis/project.h>

#include <aeget/get/file/contents.h>


static void
no_such_file(string_ty *filename)
{
    http_fatal(http_error_not_found, "%s: no such file", filename->str_text);
}


static void
emit_path(fstate_src_ty *src, string_ty *from)
{
    //
    // Print the output header.
    //
    // We use the "from" name (even though it could be an uninformative
    // name in /tmp) so that the file can be opened and scanned.
    //
    // No need for a Content-Length header, because fork_and_watch will
    // add it automagically.
    //
    attributes_ty *ap = 0;
    if (src)
	ap = attributes_list_find(src->attribute, "Content-Type");
    if (ap)
	printf("Content-Type: %s\n", ap->value->str_text);
    else
	http_content_type_header(from);
    printf("\n");

    //
    // It is very important to flush the standard output at this point,
    // because copy_whole_file is going to write on file descriptor
    // zero, completely bypassing the stdio buffering.
    //
    fflush(stdout);

    //
    // Now copy the file contents to stdout.
    //
    // FIXME: what about modifiers for line numbering, syntax
    // highlighting, etc?
    //
    string_ty *to = str_from_c("");
    os_become_orig();
    copy_whole_file(from, to, 0);
    os_become_undo();
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
path_cat_3(string_ty *s2, string_ty *s3, string_ty *s4)
{
    string_ty *temp3 = os_path_cat(s2, s3);
    string_ty *result = os_path_cat(temp3, s4);
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


static int
hunt_tristate(const string_list_ty *modifier, const char *s)
{
    string_ty       *yes;
    string_ty       *no;
    string_ty       *no_;
    int             result;
    size_t          j;

    yes = str_from_c(s);
    no = str_format("no%s", s);
    no_ = str_format("no-%s", s);
    result = -1;
    for (j = 0; j < modifier->nstrings; ++j)
    {
	string_ty       *tmp;

	tmp = modifier->string[j];
	if (str_equal(tmp, yes))
	{
	    result = 1;
	    break;
	}
	if (str_equal(tmp, no) || str_equal(tmp, no_))
	{
	    result = 0;
	    break;
	}
    }
    str_free(yes);
    str_free(no);
    str_free(no_);
    return result;
}


static void
emit_dir(change::pointer cp, string_list_ty *search_path, string_ty *filename,
    string_list_ty *modifier)
{
    string_list_ty  gizzards;
    size_t          j;
    time_t          time_split;
    string_ty       *idx;
    int             long_flag;
    int             links_flag;
    int             derived_flag;
    char            modbuf[50];
    unsigned long   nfiles;

    //
    // Read the directory contents.
    //
    os_become_orig();
    dir_stack_readdir(search_path, filename, &gizzards);
    os_become_undo();
    gizzards.sort();

    //
    // Look for an index file.
    //
    idx = 0;
    if (hunt_tristate(modifier, "index"))
    {
	idx = hunt(&gizzards, "index.html");
	if (!idx)
	    idx = hunt(&gizzards, "index.htm");
	if (!idx)
	    idx = hunt(&gizzards, "welcome.html");
	if (!idx)
	    idx = hunt(&gizzards, "welcome.htm");
    }
    if (idx)
    {
	string_ty       *path;
	string_ty       *abs_path;
	struct stat     st;

	path = os_path_cat(filename, idx);
	os_become_orig();
	abs_path = dir_stack_find(search_path, 0, path, &st, 0, 1);
	os_become_undo();
	str_free(path);
	if (abs_path)
	{
	    if (cp->bogus)
	    {
		idx = path_cat_3(project_name_get(cp->pp), filename, idx);
	    }
	    else
	    {
		string_ty       *s;

		s =
		    str_format
		    (
			"%s.C%ld",
			project_name_get(cp->pp)->str_text,
			magic_zero_decode(cp->number)
		    );
		idx = path_cat_3(s, filename, idx);
		str_free(s);
	    }
	    printf("Location: %s/", http_script_name());
	    html_escape_string(idx);
	    str_free(idx);
	    printf("\n\n");
	    return;
	}
    }

    //
    // Establish the listing options.
    //
    long_flag = hunt_tristate(modifier, "long") != 0;
    links_flag = hunt_tristate(modifier, "links") != 0;
    derived_flag = hunt_tristate(modifier, "derived") != 0;
    snprintf
    (
	modbuf,
	sizeof(modbuf),
	"file+contents+%s+%s+%s",
	(long_flag ? "long" : "nolong"),
	(links_flag ? "links" : "nolinks"),
	(derived_flag ? "derived" : "noderived")
    );

    //
    // Print the output header.
    //
    html_header(0, cp);
    printf("<title>Project ");
    html_encode_string(project_name_get(cp->pp));
    if (!cp->bogus)
	printf(",\nChange %ld", magic_zero_decode(cp->number));
    printf(",\nDirectory ");
    html_encode_string(filename);
    printf("</title></head><body>\n");
    html_header_ps(0, cp);
    printf("<h1 align=center>\n");
    if (links_flag)
	emit_change(cp);
    else
    {
	printf("Project ");
	html_encode_string(project_name_get(cp->pp));
       	if (!cp->bogus)
	    printf(",\nChange %ld", magic_zero_decode(cp->number));
    }
    printf(",<br>\nDirectory &ldquo;");
    string_list_ty dirs;
    dirs.split(filename, "/");
    size_t dirs_max = dirs.size();
    while (dirs_max > 0 && dirs[dirs_max - 1]->str_length == 0)
	--dirs_max;
    for (size_t dn = 0; dn + 1 < dirs_max; ++dn)
    {
	string_ty *s = dirs[dn];
	if (s->str_length)
	{
	    string_ty *s2 = dirs.unsplit(0, dn, "/");
	    emit_file_href(cp, s2, 0);
	    str_free(s2);
	    html_encode_string(s);
	    printf("</a\n>");
	}
	printf("/");
    }
    if (dirs_max)
	html_encode_string(dirs[dirs_max - 1]);
    printf("&rdquo;</h1>\n");

    printf("<table>\n");
    printf("<tr class=\"even-group\">");
    if (long_flag)
	printf("<th>Mode</th><th colspan=2>Date</th><th>Size</th>");
    printf("<th>Filename</th>");
    if (links_flag)
	printf("<th>&nbsp;</th>");
    printf("</tr>\n");

    time_split = now() - 6*30*24*60*60;
    nfiles = 0;
    int rownum = 0;
    for (j = 0; j < gizzards.nstrings; ++j)
    {
	string_ty       *deeper;
	string_ty       *ref;
	string_ty       *abs_path;
	struct stat     st;
	string_ty       *s;
	int             is_a_change_file;
	int             is_a_project_file;
	int             is_a_directory;

	deeper = os_path_cat(filename, gizzards.string[j]);
	os_become_orig();
	abs_path = dir_stack_find(search_path, 0, deeper, &st, 0, 1);
	os_become_undo();
	if (!abs_path)
	    continue;
	str_free(abs_path);

	//
	// Work out what kind of source file it is.
	//
	is_a_change_file = 0;
	is_a_project_file = 0;
	is_a_directory = 0;
	bool is_a_file = false;
	s = os_path_cat(filename, gizzards.string[j]);
	fstate_src_ty *src = 0;
	if (!cp->bogus && 0 != (src = change_file_find(cp, s, view_path_first)))
	{
	    is_a_change_file = 1;
	    is_a_file = true;
	}
	else if (0 != (src = project_file_find(cp->pp, s, view_path_simple)))
	{
	    is_a_project_file = 1;
	    is_a_file = true;
	}
	else if ((st.st_mode & S_IFMT) == S_IFDIR)
	{
	    is_a_directory = 1;
	}
	else if ((st.st_mode & S_IFMT) == S_IFREG)
	{
	    is_a_file = true;
	}
	str_free(s);
	bool has_been_removed = (src && src->action == file_action_remove);

	//
	// We may have been asked to omit non-source files.
	//
	if
	(
	    !derived_flag
	&&
	    (
		(
		    !is_a_change_file
		&&
		    !is_a_project_file
		&&
		    !is_a_directory
		)
	    ||
		has_been_removed
	    )
	)
	    continue;

	printf
	(
	    "<tr class=\"%s-group\">\n",
	    ((rownum++ % 6 < 3) ? "odd" : "even" )
	);
	if (!is_a_directory)
	    ++nfiles;

	if (long_flag)
	{
	    if (!has_been_removed)
	    {
		printf("<td valign=top>");
		printf("<tt>");
		html_encode_charstar(mode_string(st.st_mode));
		printf("</tt>");
		printf("</td>\n<td valign=top>");
		struct tm *the_time = localtime(&st.st_mtime);
		char buffer[100];
		strftime(buffer, sizeof(buffer), "%b %e", the_time);
		html_encode_charstar(buffer);
		printf("</td>\n<td valign=top>");
		if (st.st_mtime < time_split)
		    strftime(buffer, sizeof(buffer), "%Y", the_time);
		else
		    strftime(buffer, sizeof(buffer), "%H:%M", the_time);
		html_encode_charstar(buffer);
		printf("</td>\n<td valign=\"top\" align=\"right\">");
		printf("%ld", (long)st.st_size);
		printf("</td>\n");
	    }
	    else
	    {
		printf("<td colspan=4 align=\"CENTER\"><i>removed</i></td>\n");
	    }
	}

	printf("<td valign=\"top\">\n");
	if (is_a_directory || (is_a_file && !has_been_removed))
	{
	    printf("<a href=\"%s/", http_script_name());
	    if (cp->bogus)
	    {
		ref =
		    path_cat_3
		    (
			project_name_get(cp->pp),
			filename,
			gizzards.string[j]
		    );
	    }
	    else
	    {
		s =
		    str_format
		    (
			"%s.C%ld",
			project_name_get(cp->pp)->str_text,
			magic_zero_decode(cp->number)
		    );
		ref = path_cat_3(s, filename, gizzards.string[j]);
		str_free(s);
	    }
	    html_escape_string(ref);
	    str_free(ref);
	    if (is_a_directory)
		printf("/?%s", modbuf);
	    printf("\">\n");
	}
	html_encode_string(gizzards.string[j]);
	if (is_a_directory || (is_a_file && !has_been_removed))
	    printf("</a>");
	if (is_a_directory)
	    printf("/");
	printf("</td>\n");

	if (links_flag)
	{
	    if (is_a_change_file)
	    {
		s = os_path_cat(filename, gizzards.string[j]);
		printf("<td valign=\"top\">(");
		emit_file_href(cp, s, "file+menu");
		printf("Menu</a>)</td>\n");
		str_free(s);
	    }
	    else if (is_a_project_file)
	    {
		int             hold_bogus;

		s = os_path_cat(filename, gizzards.string[j]);
		hold_bogus = cp->bogus;
		cp->bogus = 1;
		printf("<td valign=\"top\">(");
		emit_file_href(cp, s, "file+menu");
		printf("Menu</a>)</td>\n");
		cp->bogus = hold_bogus;
		str_free(s);
	    }
	    else
		printf("<td>&nbsp;</td>");
	}
	printf("</tr>\n");
    }
    printf
    (
	"<tr class=\"even-group\"><td colspan=%d>",
	(long_flag ? 4 : 0) + 1 + (links_flag ? 1 : 0)
    );
    printf("Listed %lu files.</td></tr>\n", nfiles);

    printf("</table>\n");

    if (links_flag)
    {
        printf("<hr>A similar listing may be obtained from the command\n");
        printf("line with the following command:\n<blockquote><pre>");
	printf("aels -p ");
	html_encode_string(project_name_get(cp->pp));
	if (!cp->bogus)
	    printf(" -c %ld", magic_zero_decode(cp->number));
	if (long_flag)
	    printf(" --long");
	printf(" ");
	html_encode_string(filename);
	printf("</pre></blockquote>\n");

	printf("<hr>\n");
	printf("<p align=center class=\"navbar\">[\n");
	printf("<a href=\"%s/\">Project List</a> |\n", http_script_name());
	emit_project_href(cp->pp, "menu");
	printf("Project Menu</a> |\n");
	emit_project_href(cp->pp, "changes");
	printf("Change List</a>\n");
	if (!cp->bogus)
	{
	    printf("| ");
	    emit_change_href(cp, "menu");
	    printf("Change Menu</a>\n");
	}
	printf("]<br>[\n");
	snprintf
	(
	    modbuf,
	    sizeof(modbuf),
	    "file+contents+%s+%s+%s",
	    (long_flag ? "nolong" : "long"),
	    (links_flag ? "links" : "nolinks"),
	    (derived_flag ? "derived" : "noderived")
	);
	emit_file_href(cp, filename, modbuf);
	printf("%s Details</a> |\n", (long_flag ? "Short" : "Long"));
	snprintf
	(
	    modbuf,
	    sizeof(modbuf),
	    "file+contents+%s+%s+%s",
	    (long_flag ? "long" : "nolong"),
	    (links_flag ? "nolinks" : "links"),
	    (derived_flag ? "derived" : "noderived")
	);
	emit_file_href(cp, filename, modbuf);
	printf("%s Navigation Links</a> |\n", (links_flag ? "No" : ""));
	snprintf
	(
	    modbuf,
	    sizeof(modbuf),
	    "file+contents+%s+%s+%s",
	    (long_flag ? "long" : "nolong"),
	    (links_flag ? "links" : "nolinks"),
	    (derived_flag ? "noderived" : "derived")
	);
	emit_file_href(cp, filename, modbuf);
	printf("%s Derived Files</a>\n", (derived_flag ? "No" : "Include"));
	printf("]</p>\n");
    }

    html_footer(0, cp);
}


void
get_file_contents(change::pointer cp, string_ty *filename,
    string_list_ty *modifier)
{
    string_ty       *absolute_path;
    struct stat     st;
    fstate_src_ty   *src = 0;

    //
    // Make sure if the file actually exists in the given change
    // (project) that you fetch the appropriate historical version,
    // not necessarily the one in the file system.
    //
    if (!cp->bogus)
    {
	src = change_file_find(cp, filename, view_path_first);
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
		if (cp->is_completed())
		{
		    int             delete_me;
		    string_ty       *s;

		    //
		    // We need historical version.  We could get lucky,
		    // it could be in the baseline or one of the ancestor
		    // baselines.
		    //
		    s = project_file_version_path(cp->pp, src, &delete_me);
		    assert(s);
		    if (!s)
		    {
			no_such_file(filename);
			return;
		    }
		    emit_path(src, s);
		    if (delete_me)
		    {
			os_become_orig();
			os_unlink_errok(s);
			os_become_undo();
		    }
		    str_free(s);
		}
		else
		{
		    string_ty       *s;

		    //
		    // We can use the version in the development directory.
		    //
		    s = change_file_path(cp, filename);
		    emit_path(src, s);
		    str_free(s);
		}
		break;
	    }
	    return;
	}

	//
	// If the change is completed, look for the file as it would
	// have appeared at the time the change was integrated.
	//
	if
	(
	    cp->is_completed()
	&&
	    project_file_find(cp->pp, filename, view_path_simple)
	)
	{
	    time_t          when;
	    file_event   *fep;
	    string_ty       *s;
	    int             delete_me;

	    when = change_completion_timestamp(cp);
	    project_file_roll_forward historian(cp->pp, when, 0);
	    fep = historian.get_last(filename);
	    if (!fep)
	    {
		//
		// The file doesn't exist yet at this
		// delta.  Omit it.
		//
		no_such_file(filename);
		return;
	    }
	    assert(fep->get_src());

	    s = project_file_version_path(cp->pp, fep->get_src(), &delete_me);
	    assert(s);
	    if (!s)
	    {
		no_such_file(filename);
		return;
	    }
	    emit_path(fep->get_src(), s);
	    if (delete_me)
	    {
		os_become_orig();
		os_unlink_errok(s);
		os_become_undo();
	    }
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
		emit_path(src, s);
		if (delete_me)
		{
		    os_become_orig();
		    os_unlink_errok(s);
		    os_become_undo();
		}
		str_free(s);
	    }
	    break;
	}
	return;
    }

    //
    // Build the search path.
    //
    string_list_ty search_path;
    if (cp->bogus)
    {
	project_search_path_get(cp->pp, &search_path, 0);
    }
    else
    {
	change_search_path_get(cp, &search_path, 0);
    }

    os_become_orig();
    absolute_path = dir_stack_find(&search_path, 0, filename, &st, 0, 1);
    os_become_undo();
    if (!absolute_path)
	no_such_file(filename);
    switch (st.st_mode & S_IFMT)
    {
    case S_IFDIR:
	emit_dir(cp, &search_path, filename, modifier);
	break;

    case S_IFREG:
	emit_path(0, absolute_path);
	break;

    default:
	http_fatal(http_error_not_acceptable, "Not an appropriate file.");
    }
    str_free(absolute_path);
}
