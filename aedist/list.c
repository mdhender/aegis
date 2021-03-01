/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999 Peter Miller;
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
#include <cstate.h>
#include <col.h>
#include <error.h> /* for assert */
#include <help.h>
#include <input/cpio.h>
#include <list.h>
#include <open.h>
#include <os.h>
#include <str.h>
#include <str_list.h>
#include <sub.h>


void
list_main(usage)
	void		(*usage)_((void));
{
	char		*ifn;
	input_ty	*ifp;
	input_ty	*cpio_p;
	string_ty	*archive_name;
	string_ty	*s;
	cstate		change_set;
	size_t		j;
	char		*ofn;
	int		head_col;
	int		body_col;
	int		left;
	int		usage_col;
	int		action_col;
	int		file_name_col;

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
			if (arglex() != arglex_token_string)
				option_needs_file(arglex_token_file, usage);
			ifn = arglex_value.alv_string;
			break;

		case arglex_token_output:
			if (ofn)
				duplicate_option(usage);
			if (arglex() != arglex_token_string)
				option_needs_file(arglex_token_file, usage);
			ofn = arglex_value.alv_string;
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
	col_open(ofn);
	head_col = col_create(0, 0);
	body_col = col_create(INDENT_WIDTH, 0);
	col_title("Distribution Change Set", s->str_text);
	str_free(s);

	/*
	 * read the project name from the archive,
	 * and use it to default the project if not given
	 */
	os_become_orig();
	archive_name = 0;
	ifp = input_cpio_child(cpio_p, &archive_name);
	if (!ifp)
		input_format_error(cpio_p);
	assert(archive_name);
	s = str_from_c("etc/project-name");
	if (!str_equal(archive_name, s))
		input_format_error(ifp);
	str_free(s);
	str_free(archive_name);

	s = input_one_line(ifp);
	if (!s || !s->str_length)
		input_format_error(ifp);
	input_delete(ifp);
	os_become_undo();

	col_puts(head_col, "PROJECT");
	col_eoln();
	col_puts(body_col, s->str_text);
	str_free(s);
	col_eoln();

	/*
	 * get the change details from the input
	 */
	os_become_orig();
	archive_name = 0;
	ifp = input_cpio_child(cpio_p, &archive_name);
	if (!ifp)
		input_format_error(cpio_p);
	assert(archive_name);
	s = str_from_c("etc/change-set");
	if (!str_equal(s, archive_name))
		input_format_error(ifp);
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
		input_format_error(cpio_p);
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
			input_format_error(cpio_p);
	}

	col_need(1);
	col_puts(head_col, "SUMMARY");
	col_eoln();
	col_puts(body_col, change_set->brief_description->str_text);
	col_eoln();

	col_need(5);
	col_puts(head_col, "DESCRIPTION");
	col_eoln();
	col_puts(body_col, change_set->description->str_text);
	col_eoln();

	/*
	 * cause
	 */
	col_need(5);
	col_puts(head_col, "CAUSE");
	col_eoln();
	col_printf
	(
		body_col,
		"This change was caused by %s.",
		change_cause_ename(change_set->cause)
	);
	col_eoln();

	/*
	 * files
	 */
	col_need(5);
	col_puts(head_col, "FILES");
	col_eoln();

	left = INDENT_WIDTH;
	usage_col = col_create(left, left + USAGE_WIDTH);
	left += USAGE_WIDTH + 1;
	col_heading(usage_col, "Type\n-------");

	action_col = col_create(left, left + ACTION_WIDTH);
	left += ACTION_WIDTH + 1;
	col_heading(action_col, "Action\n--------");

	file_name_col = col_create(left, 0);
	col_heading(file_name_col, "File Name\n-----------");
	for (j = 0; j < change_set->src->length; ++j)
	{
		cstate_src	src_data;

		src_data = change_set->src->list[j];
		assert(src_data->file_name);
		col_puts(usage_col, file_usage_ename(src_data->usage));
		col_puts(action_col, file_action_ename(src_data->action));
		col_puts(file_name_col, src_data->file_name->str_text);
		col_eoln();
	}

	input_delete(cpio_p);
	col_close();
	cstate_type.free(change_set);
}
