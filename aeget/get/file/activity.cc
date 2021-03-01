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

#include <common/error.h> // for assert
#include <common/str_list.h>
#include <common/symtab.h>
#include <libaegis/change/file.h>
#include <libaegis/change.h>
#include <libaegis/change/list.h>
#include <libaegis/cstate.h>
#include <libaegis/emit/brief_descri.h>
#include <libaegis/emit/edit_number.h>
#include <libaegis/emit/project.h>
#include <libaegis/http.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>

#include <aeget/get/file/activity.h>


void
get_file_activity(change::pointer master_cp, string_ty *filename,
    string_list_ty *modifier)
{
    size_t          j;
    size_t          num;
    project_ty      *pp;
    string_list_ty  key;
    int             conflict;
    int             all;
    int             num_files;

    all = 0;
    if (!filename || !filename->str_length || !strcmp(filename->str_text, "."))
	all = 1;

    //
    // See if the is an activity listing (any changes using the file)
    // or a conflict report (two or more changes using the file).
    //
    conflict = 0;
    for (j = 0; j < modifier->nstrings; ++j)
	if (0 == strcasecmp(modifier->string[j]->str_text, "conflict"))
	    ++conflict;

    pp = master_cp->pp;
    html_header(pp, master_cp);
    printf("<title>Project ");
    html_encode_string(project_name_get(pp));
    printf(", Activity</title></head><body>\n");
    html_header_ps(pp, master_cp);
    printf("<h1 align=center>");
    emit_project(pp);
    printf(",<br>\nFile Activity</h1>\n");

    //
    // Create a symbol table, indexed by filename,
    // each row is a list of changes involving that file.
    //
    symtab_ty *stp = new symtab_ty(100);
    if (!all)
    {
	change_list_ty  *clp;

	clp = new change_list_ty();
	stp->assign(filename, clp);
    }

    //
    // Go through the project's changes,
    // remembering the relevant ones.
    //
    for (j = 0; ; ++j)
    {
	cstate_ty       *cstate_data;
	long            change_number;
	change::pointer cp;
	int             used;
	size_t          file_num;

	if (!project_change_nth(pp, j, &change_number))
    	    break;
	cp = change_alloc(pp, change_number);
	change_bind_existing(cp);
	cstate_data = cp->cstate_get();
	switch (cstate_data->state)
	{
	case cstate_state_awaiting_development:
	case cstate_state_completed:
	    // ignore these
	    break;

	case cstate_state_being_developed:
	case cstate_state_awaiting_review:
	case cstate_state_being_reviewed:
	case cstate_state_awaiting_integration:
	case cstate_state_being_integrated:
	    if (!all)
	    {
		//
		// If a filename was specified, only consider changes
		// which are operating on this file.
		//
		if (!change_file_find(cp, filename, view_path_first))
		    break;
	    }

	    used = 0;
	    for (file_num = 0; ; ++file_num)
	    {
		fstate_src_ty   *src;
		change_list_ty  *clp;

		src = change_file_nth(cp, file_num, view_path_first);
		if (!src)
		    break;
		if (all && !master_cp->bogus)
		{
		    //
		    // If a master change was specified, only consider files
		    // which are in the master change.
		    //
		    if
		    (
			!change_file_find
			(
			    master_cp,
			    src->file_name,
			    view_path_first
			)
		    )
			continue;
		}
		++used;

		//
		// We have a table of changes indexed by filename.
		//
		clp = (change_list_ty *)stp->query(src->file_name);
		if (!clp)
		{
		    clp = new change_list_ty();
		    stp->assign(src->file_name, clp);
		}
		clp->append(cp);
	    }

	    //
	    // Don't change_free this change is we used any of its files.
	    //
	    if (used)
		continue;
	    break;
	}
	change_free(cp);
    }

    //
    // For each file in the symbol table,
    // list the changes working on it at the moment.
    //
    stp->keys(&key);
    key.sort();
    printf("<div class=\"information\"><table align=\"center\">\n");
    num_files = 0;
    for (j = 0; j < key.nstrings; ++j)
    {
	size_t          k;
	change_list_ty  *clp;
	string_ty       *the_file_name;

	the_file_name = key.string[j];
	clp = (change_list_ty *)stp->query(the_file_name);
	assert(clp);
	if (!clp)
	    continue;
	if (conflict && clp->length < 2)
	    continue;

	++num_files;
	printf("<tr class=\"odd-group\"><td colspan=8>\n");
	emit_file_href(master_cp, the_file_name, "menu");
	html_encode_string(the_file_name);
	printf("</a></td></tr>\n");

	printf("<tr class=\"even-group\"><th>&nbsp;&nbsp;&nbsp;&nbsp;</ht>\n");
	printf("<th>Action</th><th>Type</th><th>Edit</th><th>Change</th>\n");
	printf("<th>State</th><th>Description</th><th>&nbsp;</ht></tr>\n");
	num = 0;
	for (k = 0; k < clp->length; ++k)
	{
	    cstate_ty       *cstate_data;
	    change::pointer cp;
	    fstate_src_ty   *src;
	    const char      *html_class;

	    cp = clp->item[k];
	    src = change_file_find(cp, the_file_name, view_path_first);
	    assert(src);
	    if (!src)
		continue;
	    cstate_data = cp->cstate_get();
	    html_class = (((num / 3) & 1) ?  "even-group" : "odd-group");
	    ++num;

	    printf("<tr class=\"%s\"><td></td>\n", html_class);
	    printf("<td valign=\"top\">\n");
	    printf("%s\n", file_action_ename(src->action));
	    printf("</td><td valign=\"top\">\n");
	    printf("%s\n", file_usage_ename(src->usage));
	    printf("</td><td valign=\"top\">\n");
	    emit_edit_number(cp, src, 0);
	    printf("</td><td valign=\"top\" align=\"right\">\n");
	    emit_change_href(cp, "menu");
	    printf("%ld</a>", magic_zero_decode(cp->number));
	    printf("</td>\n<td valign=\"top\">\n");
	    html_encode_charstar(cstate_state_ename(cstate_data->state));
	    if (cstate_data->state == cstate_state_being_developed)
	    {
		printf("<br>\n<i>");
		html_encode_string(change_developer_name(cp));
		printf("</i>");
	    }
	    if (cstate_data->state == cstate_state_being_integrated)
	    {
		printf("<br>\n<i>");
		html_encode_string(change_integrator_name(cp));
		printf("</i>");
	    }
	    printf("</td>\n<td valign=\"top\">\n");
	    emit_change_brief_description(cp);
	    printf("</td>\n<td valign=top>\n");
	    emit_change_href(cp, "download");
	    printf("Download</a>\n");
	    printf("</td></tr>\n");
	}

	printf("<tr class=\"odd-group\">\n");
	printf("<td colspan=4>&nbsp;</td>\n");
	printf("<td colspan=4>\n");
	printf
	(
	    "Listed %lu change%s.</td></tr>\n",
	    (unsigned long)num,
	    (num == 1 ? "" : "s")
	);
    }
    printf("<tr class=\"even-group\"><td colspan=8>Listed ");
    printf("%d file%s.</td></tr>\n", num_files, (num_files == 1 ? "" : "s"));
    printf("</table></div>\n");

    printf("<hr>\n");
    printf("A similar report may be obtained from the command line, with\n");
    printf("<blockquote><pre>aer file_activity -p ");
    html_encode_string(project_name_get(pp));
    printf("</pre></blockquote>\n");

    printf("<hr>\n");
    printf("<p align=\"center\" class=\"navbar\">[\n");
    printf("<a href=\"%s/\">Project List</a> |\n", http_script_name());
    emit_project_href(pp, "menu");
    printf("Project Menu</a> |\n");
    emit_project_href(pp, "changes");
    printf("Change List</a>\n");
    printf("]</p>\n");

    html_footer(pp, master_cp);
}
