//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2005 Peter Miller;
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
// MANIFEST: functions to manipulate lists
//

#include <ael/column_width.h>
#include <arglex3.h>
#include <col.h>
#include <cstate.h>
#include <error.h> // for assert
#include <help.h>
#include <input/cpio.h>
#include <list.h>
#include <open.h>
#include <os.h>
#include <output.h>
#include <str.h>
#include <str_list.h>
#include <sub.h>
#include <usage.h>


void
list_main(void)
{
    string_ty       *ifn = 0;
    input_ty        *ifp;
    cstate_ty       *change_set;
    size_t          j;
    string_ty       *ofn = 0;
    output_ty       *head_col;
    output_ty       *body_col;
    int             left;
    output_ty       *usage_col;
    output_ty       *action_col;
    output_ty       *file_name_col;
    col_ty          *colp;
    arglex();
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(usage);
	    continue;

	case arglex_token_file:
	    if (ifn)
		duplicate_option(usage);
	    switch (arglex())
	    {
	    default:
		option_needs_file(arglex_token_file, usage);
		// NOTREACHED

	    case arglex_token_string:
		ifn = str_from_c(arglex_value.alv_string);
		break;

	    case arglex_token_stdio:
		ifn = str_from_c("");
		break;
	    }
	    break;

	case arglex_token_output:
	    if (ofn)
		duplicate_option(usage);
	    switch (arglex())
	    {
	    default:
		option_needs_file(arglex_token_file, usage);

	    case arglex_token_string:
		ofn = str_from_c(arglex_value.alv_string);
		break;

	    case arglex_token_stdio:
		ofn = str_from_c("");
		break;
	    }
	    break;
	}
	arglex();
    }

    //
    // Open the input file and verify the format.
    //
    string_ty *s = 0;
    input_cpio *cpio_p = aedist_open(ifn, &s);
    assert(cpio_p);
    assert(s);

    //
    // Set the listing title from the change set subject line.
    //
    colp = col_open(ofn);
    head_col = col_create(colp, 0, 0, (const char *)0);
    body_col = col_create(colp, INDENT_WIDTH, 0, (const char *)0);
    col_title(colp, "Distribution Change Set", s->str_text);
    str_free(s);
    s = 0;

    //
    // read the project name from the archive,
    // and use it to default the project if not given
    //
    os_become_orig();
    nstring archive_name;
    ifp = cpio_p->child(archive_name);
    if (!ifp)
	cpio_p->fatal_error("file missing");
    if (archive_name != "etc/project-name")
	ifp->fatal_error("wrong file");
    archive_name.clear();

    nstring pname;
    if (!ifp->one_line(pname) || pname.empty())
	ifp->fatal_error("short file");
    delete ifp;
    os_become_undo();

    head_col->fputs("PROJECT");
    col_eoln(colp);
    body_col->fputs(pname);

    //
    // read the change number from the archive, and use it to default
    // the change number if not given, and if possible.
    //
    os_become_orig();
    archive_name.clear();
    ifp = cpio_p->child(archive_name);
    if (!ifp)
	cpio_p->fatal_error("file missing");
    if (archive_name == "etc/change-number")
    {
	nstring chnum;
	ifp->one_line(chnum);
	delete ifp;
	os_become_undo();
	body_col->fputs(", change ");
	body_col->fputs(chnum);

	os_become_orig();
	archive_name.clear();
	ifp = cpio_p->child(archive_name);
	if (!ifp)
	    cpio_p->fatal_error("file missing");
    }
    col_eoln(colp);

    //
    // get the change details from the input
    //
    if (archive_name != "etc/change-set")
	ifp->fatal_error("wrong file");
    change_set = (cstate_ty *)parse_input(ifp, &cstate_type);
    ifp = 0; // parse_input input_delete()ed it for us
    os_become_undo();

    //
    // Make sure we like the change set at a macro level.
    //
    if
    (
	!change_set->brief_description
    ||
	!change_set->description
    ||
	!change_set->src
    ||
	!change_set->src->length
    )
	cpio_p->fatal_error("bad change set");
    for (j = 0; j < change_set->src->length; ++j)
    {
	cstate_src_ty   *src_data;

	src_data = change_set->src->list[j];
	if
	(
	    !src_data->file_name
	||
	    !src_data->file_name->str_length
	||
	    !(src_data->mask & cstate_src_action_mask)
	||
	    !(src_data->mask & cstate_src_usage_mask)
	)
	    cpio_p->fatal_error("bad file info");
    }

    col_need(colp, 3);
    head_col->fputs("SUMMARY");
    col_eoln(colp);
    body_col->fputs(change_set->brief_description);
    col_eoln(colp);

    col_need(colp, 5);
    head_col->fputs("DESCRIPTION");
    col_eoln(colp);
    body_col->fputs(change_set->description);
    col_eoln(colp);

    //
    // cause
    //
    col_need(colp, 5);
    head_col->fputs("CAUSE");
    col_eoln(colp);
    body_col->fprintf
    (
	"This change was caused by %s.",
	change_cause_ename(change_set->cause)
    );
    col_eoln(colp);

    //
    // files
    //
    col_need(colp, 5);
    head_col->fputs("FILES");
    col_eoln(colp);

    left = INDENT_WIDTH;
    usage_col = col_create(colp, left, left + USAGE_WIDTH, "Type\n-------");
    left += USAGE_WIDTH + 1;

    action_col =
	col_create(colp, left, left + ACTION_WIDTH, "Action\n--------");
    left += ACTION_WIDTH + 1;

    file_name_col = col_create(colp, left, 0, "File Name\n-----------");

    for (j = 0; j < change_set->src->length; ++j)
    {
	cstate_src_ty   *src_data;

	src_data = change_set->src->list[j];
	assert(src_data->file_name);
	usage_col->fputs(file_usage_ename(src_data->usage));
	action_col->fputs(file_action_ename(src_data->action));
	file_name_col->fputs(src_data->file_name);
	if (src_data->move)
	{
	    switch (src_data->action)
	    {
	    case file_action_create:
		file_name_col->fputs("\nMoved from ");
		file_name_col->fputs(src_data->move);
		break;

	    case file_action_remove:
		file_name_col->fputs("\nMoved to ");
		file_name_col->fputs(src_data->move);
		break;

	    case file_action_modify:
	    case file_action_insulate:
	    case file_action_transparent:
		assert(0);
		break;
	    }
	}
	col_eoln(colp);
    }

    delete cpio_p;
    col_close(colp);
    cstate_type.free(change_set);
}
