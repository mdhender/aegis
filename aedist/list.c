/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2001, 2002 Peter Miller;
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

#include <ael/column_width.h>
#include <arglex3.h>
#include <col.h>
#include <cstate.h>
#include <error.h> /* for assert */
#include <help.h>
#include <input/cpio.h>
#include <list.h>
#include <open.h>
#include <os.h>
#include <output.h>
#include <str.h>
#include <str_list.h>
#include <sub.h>


void
list_main(usage)
	void		(*usage)_((void));
{
	string_ty	*ifn;
	input_ty	*ifp;
	input_ty	*cpio_p;
	string_ty	*archive_name;
	string_ty	*s;
	cstate		change_set;
	size_t		j;
	string_ty	*ofn;
	output_ty	*head_col;
	output_ty	*body_col;
	int		left;
	output_ty	*usage_col;
	output_ty	*action_col;
	output_ty	*file_name_col;
	col_ty		*colp;

	ifn = 0;
	ofn = 0;
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
				/*NOTREACHED*/

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

	/*
	 * Open the input file and verify the format.
	 */
	s = 0;
	cpio_p = aedist_open(ifn, &s);
	assert(cpio_p);
	assert(s);

	/*
	 * Set the listing title from the change set subject line.
	 */
	colp = col_open(ofn);
	head_col = col_create(colp, 0, 0, (const char *)0);
	body_col = col_create(colp, INDENT_WIDTH, 0, (const char *)0);
	col_title(colp, "Distribution Change Set", s->str_text);
	str_free(s);

	/*
	 * read the project name from the archive,
	 * and use it to default the project if not given
	 */
	os_become_orig();
	archive_name = 0;
	ifp = input_cpio_child(cpio_p, &archive_name);
	if (!ifp)
		input_fatal_error(cpio_p, "file missing");
	assert(archive_name);
	s = str_from_c("etc/project-name");
	if (!str_equal(archive_name, s))
		input_fatal_error(ifp, "wrong file");
	str_free(s);
	str_free(archive_name);

	s = input_one_line(ifp);
	if (!s || !s->str_length)
		input_fatal_error(ifp, "short file");
	input_delete(ifp);
	os_become_undo();

	output_fputs(head_col, "PROJECT");
	col_eoln(colp);
	output_put_str(body_col, s);
	str_free(s);

	/*
         * read the change number from the archive, and use it to default
         * the change number if not given, and if possible.
	 */
	os_become_orig();
	archive_name = 0;
	ifp = input_cpio_child(cpio_p, &archive_name);
	if (!ifp)
		input_fatal_error(cpio_p, "file missing");
	assert(archive_name);
	s = str_from_c("etc/change-number");
	if (str_equal(s, archive_name))
	{
	    str_free(s);
	    s = input_one_line(ifp);
	    input_delete(ifp);
	    os_become_undo();
	    str_free(archive_name);

	    output_fputs(body_col, ", change ");
	    output_put_str(body_col, s);
	    str_free(s);

	    os_become_orig();
	    archive_name = 0;
	    ifp = input_cpio_child(cpio_p, &archive_name);
	    if (!ifp)
		input_fatal_error(cpio_p, "file missing");
	    assert(archive_name);
	}
	col_eoln(colp);

	/*
	 * get the change details from the input
	 */
	s = str_from_c("etc/change-set");
	if (!str_equal(s, archive_name))
		input_fatal_error(ifp, "wrong file");
	str_free(s);
	change_set = parse_input(ifp, &cstate_type);
	ifp = 0; /* parse_input input_delete()ed it for us */
	str_free(archive_name);
	os_become_undo();

	/*
	 * Make sure we like the change set at a macro level.
	 */
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
		input_fatal_error(cpio_p, "bad change set");
	for (j = 0; j < change_set->src->length; ++j)
	{
		cstate_src	src_data;

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
			input_fatal_error(cpio_p, "bad file info");
	}

	col_need(colp, 3);
	output_fputs(head_col, "SUMMARY");
	col_eoln(colp);
	output_put_str(body_col, change_set->brief_description);
	col_eoln(colp);

	col_need(colp, 5);
	output_fputs(head_col, "DESCRIPTION");
	col_eoln(colp);
	output_put_str(body_col, change_set->description);
	col_eoln(colp);

	/*
	 * cause
	 */
	col_need(colp, 5);
	output_fputs(head_col, "CAUSE");
	col_eoln(colp);
	output_fprintf
	(
		body_col,
		"This change was caused by %s.",
		change_cause_ename(change_set->cause)
	);
	col_eoln(colp);

	/*
	 * files
	 */
	col_need(colp, 5);
	output_fputs(head_col, "FILES");
	col_eoln(colp);

	left = INDENT_WIDTH;
	usage_col =
		col_create
		(
			colp,
			left,
			left + USAGE_WIDTH,
			"Type\n-------"
		);
	left += USAGE_WIDTH + 1;

	action_col =
		col_create
		(
			colp,
			left,
			left + ACTION_WIDTH,
			"Action\n--------"
		);
	left += ACTION_WIDTH + 1;

	file_name_col =
		col_create
		(
			colp,
			left,
			0,
			"File Name\n-----------"
		);

	for (j = 0; j < change_set->src->length; ++j)
	{
		cstate_src	src_data;

		src_data = change_set->src->list[j];
		assert(src_data->file_name);
		output_fputs(usage_col, file_usage_ename(src_data->usage));
		output_fputs(action_col, file_action_ename(src_data->action));
		output_put_str(file_name_col, src_data->file_name);
		col_eoln(colp);
	}

	input_delete(cpio_p);
	col_close(colp);
	cstate_type.free(change_set);
}
