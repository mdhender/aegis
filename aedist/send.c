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
 * MANIFEST: functions to manipulate sends
 */

#include <ac/stdlib.h>
#include <ac/time.h>

#include <arglex3.h>
#include <change.h>
#include <change/file.h>
#include <error.h> /* for assert */
#include <help.h>
#include <input/file.h>
#include <mem.h>
#include <os.h>
#include <output/base64.h>
#include <output/cpio.h>
#include <output/file.h>
#include <output/gzip.h>
#include <output/indent.h>
#include <project.h>
#include <project/file.h>
#include <project_hist.h>
#include <send.h>
#include <str.h>
#include <sub.h>
#include <undo.h>
#include <user.h>


static int have_it_already _((cstate, fstate_src));

static int
have_it_already(change_set, src_data)
	cstate		change_set;
	fstate_src	src_data;
{
	size_t		j;
	cstate_src	dst_data;

	if (!change_set->src)
		return 0;
	for (j = 0; j < change_set->src->length; ++j)
	{
		dst_data = change_set->src->list[j];
		if (str_equal(dst_data->file_name, src_data->file_name))
			return 1;
	}
	return 0;
}


static void one_more_src _((cstate, fstate_src));

static void
one_more_src(change_set, src_data)
	cstate		change_set;
	fstate_src	src_data;
{
	cstate_src	*dst_data_p;
	cstate_src	dst_data;
	type_ty		*type_p;

	if (!change_set->src)
		change_set->src = cstate_src_list_type.alloc();
	dst_data_p = cstate_src_list_type.list_parse(change_set->src, &type_p);
	assert(type_p == &cstate_src_type);
	dst_data = cstate_src_type.alloc();
	*dst_data_p = dst_data;
	dst_data->file_name = str_copy(src_data->file_name);
	dst_data->action = src_data->action;
	dst_data->usage = src_data->usage;
}


static int cmp _((const void *, const void *));

static int
cmp(va, vb)
	const void	*va;
	const void	*vb;
{
	cstate_src	a;
	cstate_src	b;

	a = *(cstate_src *)va;
	b = *(cstate_src *)vb;
	return strcmp(a->file_name->str_text, b->file_name->str_text);
}


void
send_main(usage)
	void		(*usage)_((void));
{
	string_ty	*project_name;
	long		change_number;
	char		*branch;
	int		grandparent;
	int		trunk;
	output_ty	*ofp;
	output_ty	*cpio_p;
	input_ty	*ifp;
	project_ty	*pp;
	change_ty	*cp;
	user_ty		*up;
	cstate		cstate_data;
	char		*output;
	string_ty	*s;
	cstate		change_set;
	time_t		now;
	size_t		j;
	char		*buffer;
	size_t		buffer_size;
	int		description_header;
	int		baseline;
	int		entire_source;
	int		ascii_armor;
	int		compress;

	branch = 0;
	change_number = 0;
	grandparent = 0;
	project_name = 0;
	trunk = 0;
	output = 0;
	description_header = -1;
	baseline = 0;
	entire_source = -1;
	ascii_armor = -1;
	compress = -1;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(usage);
			continue;

		case arglex_token_baseline:
			if (baseline)
				duplicate_option(usage);
			baseline = 1;
			break;

		case arglex_token_entire_source:
			if (entire_source > 0)
				duplicate_option(usage);
			if (entire_source >= 0)
			{
				mutually_exclusive_options
				(
					arglex_token_entire_source,
					arglex_token_entire_source_not,
					usage
				);
			}
			entire_source = 1;
			break;

		case arglex_token_entire_source_not:
			if (entire_source == 0)
				duplicate_option(usage);
			if (entire_source >= 0)
			{
				mutually_exclusive_options
				(
					arglex_token_entire_source,
					arglex_token_entire_source_not,
					usage
				);
			}
			entire_source = 0;
			break;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				option_needs_number(arglex_token_change, usage);
			/* fall throught... */

		case arglex_token_number:
			if (change_number)
				duplicate_option_by_name(arglex_token_change, usage);
			change_number = arglex_value.alv_number;
			if (!change_number)
				change_number = MAGIC_ZERO;
			else if (change_number < 0)
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set(scp, "Number", "%ld", change_number);
				fatal_intl(scp, i18n("change $number out of range"));
				/* NOTREACHED */
			}
			break;

		 case arglex_token_project:
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_project, usage);
			if (project_name)
				duplicate_option_by_name(arglex_token_project, usage);
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_branch:
			if (branch)
				duplicate_option(usage);
			switch (arglex())
			{
			default:
				option_needs_number(arglex_token_branch, usage);

			case arglex_token_number:
			case arglex_token_string:
				branch = arglex_value.alv_string;
				break;
			}
			break;

		case arglex_token_trunk:
			if (trunk)
				duplicate_option(usage);
			++trunk;
			break;

		case arglex_token_grandparent:
			if (grandparent)
				duplicate_option(usage);
			++grandparent;
			break;

		case arglex_token_output:
			if (output)
				duplicate_option(usage);
			switch (arglex())
			{
			default:
				option_needs_file(arglex_token_output, usage);
				/* NOTREACHED */

			case arglex_token_stdio:
				output = "";
				break;

			case arglex_token_string:
				output = arglex_value.alv_string;
				break;
			}
			break;

		case arglex_token_description_header:
			if (description_header == 1)
				duplicate_option(usage);
			else if (description_header >= 0)
			{
				head_desc_yuck:
				mutually_exclusive_options
				(
					arglex_token_description_header,
					arglex_token_description_header_not,
					usage
				);
			}
			description_header = 1;
			break;

		case arglex_token_description_header_not:
			if (description_header == 0)
				duplicate_option(usage);
			else if (description_header >= 0)
				goto head_desc_yuck;
			description_header = 0;
			break;

		case arglex_token_ascii_armor:
			if (ascii_armor == 1)
				duplicate_option(usage);
			else if (ascii_armor >= 0)
			{
				ascii_armor_yuck:
				mutually_exclusive_options
				(
					arglex_token_ascii_armor,
					arglex_token_ascii_armor_not,
					usage
				);
			}
			ascii_armor = 1;
			break;

		case arglex_token_ascii_armor_not:
			if (ascii_armor == 0)
				duplicate_option(usage);
			else if (ascii_armor >= 0)
				goto ascii_armor_yuck;
			ascii_armor = 0;
			break;

		case arglex_token_compress:
			if (compress > 0)
				duplicate_option(usage);
			else if (compress >= 0)
			{
				compress_yuck:
				mutually_exclusive_options
				(
					arglex_token_compress,
					arglex_token_compress_not,
					usage
				);
			}
			compress = 1;
			break;

		case arglex_token_compress_not:
			if (compress == 0)
				duplicate_option(usage);
			else if (compress >= 0)
				goto compress_yuck;
			compress = 0;
			break;
		}
		arglex();
	}
	if (entire_source < 0)
		entire_source = baseline;

	/*
	 * reject illegal combinations of options
	 */
	if (change_number && baseline)
	{
		mutually_exclusive_options
		(
			arglex_token_change,
			arglex_token_baseline,
			usage
		);
	}
	if (grandparent)
	{
		if (branch)
		{
			mutually_exclusive_options
			(
				arglex_token_branch,
				arglex_token_grandparent,
				usage
			);
		}
		if (trunk)
		{
			mutually_exclusive_options
			(
				arglex_token_trunk,
				arglex_token_grandparent,
				usage
			);
		}
		branch = "..";
	}
	if (trunk)
	{
		if (branch)
		{
			mutually_exclusive_options
			(
				arglex_token_branch,
				arglex_token_trunk,
				usage
			);
		}
		branch = "";
	}

	/*
	 * locate project data
	 */
	if (!project_name)
		project_name = user_default_project();
	pp = project_alloc(project_name);
	str_free(project_name);
	project_bind_existing(pp);

	/*
	 * locate the other branch
	 */
	if (branch)
		pp = project_find_branch(pp, branch);

	/*
	 * locate user data
	 */
	up = user_executing(pp);

	/*
	 * locate change data
	 */
	if (baseline)
		cp = change_copy(project_change_get(pp));
	else
	{
		if (!change_number)
			change_number = user_default_change(up);
		cp = change_alloc(pp, change_number);
		change_bind_existing(cp);
	}

	cstate_data = change_cstate_get(cp);
	switch (cstate_data->state)
	{
	case cstate_state_completed:
		if (!entire_source)
			break;
		/*
		 * You can't send the entire source of a completed
		 * change.  This is because it is VERY hard to figure
		 * out the correct versions of the project files, and
		 * it takes a long time (need to open lots and lots
		 * of change info).  See this file history report for
		 * how much fun this is.  The anticipated uses of this
		 * command don't justify the effort in figuring out
		 * which versions of each project file to send.
		 */
		/* fall through... */

	default:
		change_fatal(cp, 0, i18n("bad send state"));

	case cstate_state_being_integrated:
	case cstate_state_awaiting_integration:
	case cstate_state_being_reviewed:
	case cstate_state_being_developed:
		break;
	}

	/* open the output */
	os_become_orig();
	if (ascii_armor)
		ofp = output_file_text_open(output);
	else
		ofp = output_file_binary_open(output);
	output_fputs(ofp, "MIME-Version: 1.0\n");
	output_fputs(ofp, "Content-Type: application/aegis-change-set\n");
	if (ascii_armor)
		output_fputs(ofp, "Content-Transfer-Encoding: base64\n");
	if (entire_source)
		s = project_description_get(pp);
	else
		s = cstate_data->brief_description;
	output_fprintf
	(
		ofp,
		"Subject: %s - %.80s\n",
		project_name_get(pp)->str_text,
		s->str_text
	);
	if (change_number && !entire_source)
	{
		output_fprintf
		(
			ofp,
			"Content-Name: %s.C%3.3ld.ae\n",
			project_name_get(pp)->str_text,
			change_number
		);
	}
	else
	{
		output_fprintf
		(
			ofp,
			"Content-Name: %s.ae\n",
			project_name_get(pp)->str_text
		);
	}
	output_fputc(ofp, '\n');
	if (ascii_armor)
		ofp = output_base64(ofp, 1);
	if (compress)
		ofp = output_gzip(ofp);
	cpio_p = output_cpio(ofp);

	/*
	 * Add the project name to the archive.
	 */
	s = str_from_c("etc/project-name");
	ofp = output_cpio_child(cpio_p, s, -1);
	str_free(s);
	output_fprintf(ofp, "%s\n", project_name_get(pp)->str_text);
	output_delete(ofp);
	os_become_undo();

	/*
	 * Add the change details to the archive.
	 */
	change_set = cstate_type.alloc();
	if (entire_source)
	{
		change_set->brief_description =
			str_copy(project_description_get(pp));
	}
	else
	{
		change_set->brief_description =
			str_copy(cstate_data->brief_description);
	}
	if (description_header)
	{
		string_ty	*warning;

		warning = 0;
		if
		(
			entire_source
		?
			cstate_data->state < cstate_state_being_integrated
		:
			cstate_data->state != cstate_state_completed
		)
		{
			warning =
				str_format
				(
			"Warning: the original change was in the '%s' state\n",
					cstate_state_ename(cstate_data->state)
				);
		}
		time(&now);
		if (cstate_data->state > cstate_state_being_developed)
		{
			cstate_history_list hlp;

			hlp = cstate_data->history;
			assert(hlp);
			assert(hlp->length > 0);
			assert(hlp->list);
			if (hlp && hlp->length > 0 && hlp->list)
			{
				cstate_history	hp;

				hp = hlp->list[hlp->length - 1];
				assert(hp);
				now = hp->when;
			}
		}
		change_set->description =
			str_format
			(
				"From: %s\nDate: %.24s\n%s\n%S",
				user_email_address(up)->str_text,
				ctime(&now),
				(warning ? warning->str_text : ""),
				cstate_data->description
			);
		if (warning)
			str_free(warning);
	}
	else if (entire_source)
		change_set->description = str_copy(project_description_get(pp));
	else
		change_set->description = str_copy(cstate_data->description);
	change_set->cause = cstate_data->cause;
	change_set->test_exempt = cstate_data->test_exempt;
	change_set->test_baseline_exempt = cstate_data->test_baseline_exempt;
	change_set->regression_test_exempt =
		cstate_data->regression_test_exempt;
	/* architecture */
	/* copyright years */

	for (j = 0; ; ++j)
	{
		fstate_src	src_data;

		src_data = change_file_nth(cp, j);
		if (!src_data)
			break;
		if (src_data->action == file_action_insulate)
			continue;
		if
		(
			src_data->usage == file_usage_build
		&&
			src_data->action == file_action_modify
		)
			continue;
		one_more_src(change_set, src_data);
	}
	if (entire_source)
	{
		for (j = 0; ; ++j)
		{
			fstate_src	src_data;

			src_data = project_file_nth(pp, j);
			if (!src_data)
				break;
			if (src_data->about_to_be_created_by)
				continue;
			if (src_data->action == file_action_insulate)
				continue;
			if
			(
				src_data->usage == file_usage_build
			&&
				src_data->action == file_action_modify
			)
				continue;
			if (!have_it_already(change_set, src_data))
				one_more_src(change_set, src_data);
		}
	}
	if (!change_set->src || !change_set->src->length)
		change_fatal(cp, 0, i18n("bad send no files"));

	/*
	 * sort the files by name
	 */
	qsort
	(
		change_set->src->list,
		change_set->src->length,
		sizeof(change_set->src->list[0]),
		cmp
	);

	os_become_orig();
	s = str_format("etc/change-set");
	ofp = output_cpio_child(cpio_p, s, -1);
	ofp = output_indent(ofp);
	str_free(s);
	cstate_write(ofp, change_set);
	output_delete(ofp);
	os_become_undo();

	/*
	 * add each of the relevant source files to the archive
	 */
	buffer_size = (size_t)1 << 13;
	buffer = mem_alloc(buffer_size);
	for (j = 0; j < change_set->src->length; ++j)
	{
		cstate_src	csrc;
		fstate_src	c_src_data;
		fstate_src	p_src_data;
		long		len;

		csrc = change_set->src->list[j];
		if (csrc->usage == file_usage_build)
			continue;
		if (csrc->action == file_action_remove)
			continue;

		/*
		 * Find a source file.	Depending on the change state,
		 * it could be in the development directory, or in the
		 * baseline or in history.
		 */
		ifp = 0;
		switch (cstate_data->state)
		{
		case cstate_state_awaiting_development:
			assert(0);
			continue;

		case cstate_state_being_developed:
		case cstate_state_being_reviewed:
		case cstate_state_awaiting_integration:
		case cstate_state_being_integrated:
			s = change_file_path(cp, csrc->file_name);
			if (!s)
			{
				assert(entire_source);
				s = project_file_path(pp, csrc->file_name);
				if (!s)
					this_is_a_bug();
			}
			os_become_orig();
			ifp = input_file_open(s->str_text);
			str_free(s);
			break;

		case cstate_state_completed:
			c_src_data = change_file_find(cp, csrc->file_name);
			p_src_data = project_file_find(pp, csrc->file_name);
			if (!p_src_data)
				this_is_a_bug();
			if
			(
				!c_src_data
			||
				str_equal(p_src_data->edit_number, c_src_data->edit_number)
			)
			{
				s = project_file_path(pp, csrc->file_name);
				os_become_orig();
				ifp = input_file_open(s->str_text);
				str_free(s);
			}
			else
			{
				/*
				 * get the relevant version out of history
				 */
				s = os_edit_filename(0);
				os_become_orig();
				undo_unlink_errok(s);
				os_become_undo();

				/*
				 * get the file from history
				 */
				change_run_history_get_command
				(
					cp,
					c_src_data->file_name,
					c_src_data->edit_number,
					s,
					up
				);

				/*
				 * Open the file, remembering to unlink
				 * it when we are done.
				 */
				os_become_orig();
				ifp = input_file_open(s->str_text);
				input_file_unlink_on_close(ifp);
				str_free(s);
			}
			break;
		}
		assert(ifp);
		len = input_length(ifp);

		/*
		 * start a new archive file
		 */
		s = str_format("src/%S", csrc->file_name);
		ofp = output_cpio_child(cpio_p, s, len);
		str_free(s);
		input_to_output(ifp, ofp);
		input_delete(ifp);
		output_delete(ofp);
		os_become_undo();
	}
	cstate_type.free(change_set);

	/* finish writing the cpio archive */
	os_become_orig();
	output_delete(cpio_p);
	os_become_undo();

	/* clean up and go home */
	change_free(cp);
	project_free(pp);
}
