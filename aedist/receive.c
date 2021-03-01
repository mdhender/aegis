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
 * MANIFEST: functions to manipulate receives
 */

#include <arglex3.h>
#include <cattr.h>
#include <change.h>
#include <change_file.h>
#include <error.h> /* for assert */
#include <help.h>
#include <input/cpio.h>
#include <open.h>
#include <os.h>
#include <output/file.h>
#include <parse.h>
#include <project.h>
#include <project/file/trojan.h>
#include <project_file.h>
#include <project_hist.h>
#include <receive.h>
#include <str.h>
#include <str_list.h>
#include <sub.h>
#include <undo.h>
#include <user.h>


static long number_of_files _((string_ty *, long));

static long
number_of_files(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	project_ty	*pp;
	change_ty	*cp;
	long		result;

	pp = project_alloc(project_name);
	project_bind_existing(pp);
	cp = change_alloc(pp, change_number);
	change_bind_existing(cp);
	result = change_file_count(cp);
	change_free(cp);
	project_free(pp);
	return result;
}


void
receive_main(usage)
	void		(*usage)_((void));
{
	string_ty	*project_name;
	long		change_number;
	char		*ifn;
	input_ty	*ifp;
	input_ty	*cpio_p;
	string_ty	*archive_name;
	string_ty	*s;
	project_ty	*pp;
	change_ty	*cp;
	string_ty	*dd;
	cstate		change_set;
	size_t		j;
	cattr		cattr_data;
	string_ty	*attribute_file_name;
	string_list_ty	files_source;
	string_list_ty	files_build;
	string_list_ty	files_test_auto;
	string_list_ty	files_test_manual;
	int		need_to_test;
	string_ty	*s2;
	int		could_have_a_trojan;
	int		config_seen;
	string_ty	*the_config_file;
	int		uncopy;
	int		trojan;
	string_list_ty	wl;
	string_ty	*dot;
	const char	*delta;

	project_name = 0;
	change_number = 0;
	ifn = 0;
	trojan = -1;
	delta = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				option_needs_number(arglex_token_change, usage);
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

		case arglex_token_file:
			if (ifn)
				duplicate_option(usage);
			if (arglex() != arglex_token_string)
				option_needs_file(arglex_token_file, usage);
			ifn = arglex_value.alv_string;
			break;

		case arglex_token_trojan:
			if (trojan > 0)
				duplicate_option(usage);
			if (trojan >= 0)
			{
				too_many_trojans:
				mutually_exclusive_options
				(
					arglex_token_trojan,
					arglex_token_trojan_not,
					usage
				);
			}
			trojan = 1;
			break;

		case arglex_token_trojan_not:
			if (trojan == 0)
				duplicate_option(usage);
			if (trojan >= 0)
				goto too_many_trojans;
			trojan = 0;
			break;

		case arglex_token_delta:
			if (delta)
				duplicate_option(usage);
			switch (arglex())
			{
			default:
				option_needs_number(arglex_token_delta, usage);
				/*NOTREACHED*/
			
			case arglex_token_number:
			case arglex_token_string:
				delta = arglex_value.alv_string;
				break;
			}
		}
		arglex();
	}

	/*
	 * Open the input file and verify the format.
	 */
	cpio_p = aedist_open(ifn, (string_ty **)0);
	assert(cpio_p);

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
	s = input_one_line(ifp);
	if (!s || !s->str_length)
		input_format_error(ifp);
	if (!project_name)
		project_name = s;
	else
		str_free(s);
	input_delete(ifp);
	os_become_undo();
	str_free(archive_name);

	/*
	 * locate project data
	 *	(Even of we don't use it, this confirms it is a valid
	 *	project name.)
	 */
	pp = project_alloc(project_name);
	project_bind_existing(pp);

	/*
	 * default the change number
	 */
	if (!change_number)
		change_number = project_next_change_number(pp, 1);
	project_free(pp);
	pp = 0;

	/*
	 * get the change details from the input
	 */
	archive_name = 0;
	os_become_orig();
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
	os_become_undo();
	str_free(archive_name);

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

	/*
	 * construct change attributes from the change_set
	 */
	os_become_orig();
	attribute_file_name = os_edit_filename(0);
	undo_unlink_errok(attribute_file_name);
	cattr_data = cattr_type.alloc();
	cattr_data->brief_description = str_copy(change_set->brief_description);
	cattr_data->description = str_copy(change_set->description);
	cattr_data->cause = change_set->cause;
	cattr_write_file(attribute_file_name->str_text, cattr_data, 0);
	cattr_type.free(cattr_data);

	/*
	 * create the new change
	 */
	dot = os_curdir();
	s =
		str_format
		(
		      "aegis --new-change %ld --project=%S --file=%S --verbose",
			change_number,
			project_name,
			attribute_file_name
		);
	os_execute(s, OS_EXEC_FLAG_INPUT, dot);
	str_free(s);
	os_unlink_errok(attribute_file_name);
	str_free(attribute_file_name);

	/*
	 * Begin development of the new change.
	 */
	s =
		str_format
		(
			"aegis --develop-begin %ld --project=%S --verbose",
			change_number,
			project_name
		);
	os_execute(s, OS_EXEC_FLAG_INPUT, dot);
	str_free(s);
	os_become_undo();

	/*
	 * Change to the development directory, so that we can use
	 * relative filenames.  It makes things easier to read.
	 */
	pp = project_alloc(project_name);
	project_bind_existing(pp);
	cp = change_alloc(pp, change_number);
	change_bind_existing(cp);
	dd = change_development_directory_get(cp, 0);
	dd = str_copy(dd); /* will vanish when change_free(); */
	change_free(cp);
	cp = 0;

	os_chdir(dd);

	/*
	 * Adjust the file actions to reflect the current state of
	 * the project.
	 */
	need_to_test = 0;
	could_have_a_trojan = 0;
	for (j = 0; j < change_set->src->length; ++j)
	{
		cstate_src	src_data;
		fstate_src	p_src_data;

		src_data = change_set->src->list[j];
		assert(src_data->file_name);
		p_src_data = project_file_find(pp, src_data->file_name);
		if (!p_src_data || p_src_data->action == file_action_remove)
		{
			if (src_data->action == file_action_remove)
			{
				/*
				 * Removing a removed file would be an
				 * error, so butcher the action field
				 * and none of the selection loops will
				 * use it.
				 */
				src_data->action = -1;
			}
			else
				src_data->action = file_action_create;
		}
		else
		{
			if (src_data->action != file_action_remove)
				src_data->action = file_action_modify;
		}
		if (project_file_trojan_suspect(pp, src_data->file_name))
			could_have_a_trojan = 1;
	}
	project_free(pp);
	pp = 0;

	/*
	 * add the modified files to the change
	 */
	string_list_constructor(&files_source);
	string_list_constructor(&files_build);
	string_list_constructor(&files_test_auto);
	string_list_constructor(&files_test_manual);
	for (j = 0; j < change_set->src->length; ++j)
	{
		cstate_src	src_data;

		/*
		 * For now, we are only copying files.
		 */
		src_data = change_set->src->list[j];
		assert(src_data->file_name);
		if (src_data->action != file_action_modify)
			continue;
		if (src_data->usage == file_usage_build)
			continue;

		/*
		 * add it to the list
		 */
		string_list_append(&files_source, src_data->file_name);
		if
		(
			src_data->usage == file_usage_test
		||
			src_data->usage == file_usage_manual_test
		)
			need_to_test = 1;
	}
	uncopy = 0;
	if (files_source.nstrings)
	{
		string_ty	*delopt;

		delopt = 0;
		if (delta)
		{
			delopt = str_from_c(delta);
			s = str_quote_shell(delopt);
			str_free(delopt);
			delopt = str_format(" --delta=%S", s);
			str_free(s);
		}
		uncopy = 1;
		string_list_quote_shell(&wl, &files_source);
		s2 = wl2str(&wl, 0, wl.nstrings, (char *)0);
		string_list_destructor(&wl);
		s =
			str_format
			(
		   "aegis --copy-file %S --project=%S --change=%ld --verbose%s",
				s2,
				project_name,
				change_number,
				(delopt ? delopt->str_text : "")
			);
		if (delopt)
			str_free(delopt);
		str_free(s2);
		os_execute(s, OS_EXEC_FLAG_INPUT, dd);
		str_free(s);
	}
	string_list_destructor(&files_source);

	/*
	 * add the removed files to the change
	 */
	for (j = 0; j < change_set->src->length; ++j)
	{
		cstate_src	src_data;

		/*
		 * For now, we are only removing files.
		 */
		src_data = change_set->src->list[j];
		assert(src_data->file_name);
		if (src_data->action != file_action_remove)
			continue;

		/*
		 * add it to the list
		 */
		string_list_append(&files_source, src_data->file_name);
	}
	if (files_source.nstrings)
	{
		string_list_quote_shell(&wl, &files_source);
		s2 = wl2str(&wl, 0, wl.nstrings, (char *)0);
		string_list_destructor(&wl);
		s =
			str_format
			(
		   "aegis --remove-file %S --project=%S --change=%ld --verbose",
				s2,
				project_name,
				change_number
			);
		str_free(s2);
		os_execute(s, OS_EXEC_FLAG_INPUT, dd);
		str_free(s);
	}
	string_list_destructor(&files_source);

	/*
	 * add the new files to the change
	 */
	need_to_test = 0;
	for (j = 0; j < change_set->src->length; ++j)
	{
		cstate_src	src_data;

		/*
		 * for now, we are only dealing with create
		 */
		src_data = change_set->src->list[j];
		assert(src_data->file_name);
		if (src_data->action != file_action_create)
			continue;

		/*
		 * add it to the list
		 */
		switch (src_data->usage)
		{
		case file_usage_source:
			string_list_append(&files_source, src_data->file_name);
			break;

		case file_usage_build:
			string_list_append(&files_build, src_data->file_name);
			break;

		case file_usage_test:
			string_list_append(&files_test_auto, src_data->file_name);
			need_to_test = 1;
			break;

		case file_usage_manual_test:
			string_list_append(&files_test_manual, src_data->file_name);
			need_to_test = 1;
			break;
		}
	}

	os_become_orig();
	if (files_build.nstrings)
	{
		string_list_quote_shell(&wl, &files_build);
		s2 = wl2str(&wl, 0, wl.nstrings, (char *)0);
		string_list_destructor(&wl);
		s =
			str_format
			(
	      "aegis --new-file %S --build --project=%S --change=%ld --verbose",
				s2,
				project_name,
				change_number
			);
		str_free(s2);
		os_execute(s, OS_EXEC_FLAG_INPUT, dd);
		str_free(s);
	}
	if (files_test_auto.nstrings)
	{
		string_list_quote_shell(&wl, &files_test_auto);
		s2 = wl2str(&wl, 0, wl.nstrings, (char *)0);
		string_list_destructor(&wl);
		s =
			str_format
			(
	  "aegis --new-test %S --automatic --project=%S --change=%ld --verbose",
				s2,
				project_name,
				change_number
			);
		str_free(s2);
		os_execute(s, OS_EXEC_FLAG_INPUT, dd);
		str_free(s);
	}
	if (files_test_manual.nstrings)
	{
		string_list_quote_shell(&wl, &files_test_manual);
		s2 = wl2str(&wl, 0, wl.nstrings, (char *)0);
		string_list_destructor(&wl);
		s =
			str_format
			(
	     "aegis --new-test %S --manual --project=%S --change=%ld --verbose",
				s2,
				project_name,
				change_number
			);
		str_free(s2);
		os_execute(s, OS_EXEC_FLAG_INPUT, dd);
		str_free(s);
	}
	/*
	 * NOTE: do this one last, in case it includes the first instance
	 * of the project config file.
	 */
	if (files_source.nstrings)
	{
		string_list_quote_shell(&wl, &files_source);
		s2 = wl2str(&wl, 0, wl.nstrings, (char *)0);
		string_list_destructor(&wl);
		s =
			str_format
			(
		      "aegis --new-file %S --project=%S --change=%ld --verbose",
				s2,
				project_name,
				change_number
			);
		str_free(s2);
		os_execute(s, OS_EXEC_FLAG_INPUT, dd);
		str_free(s);
	}
	string_list_destructor(&files_source);
	string_list_destructor(&files_build);
	string_list_destructor(&files_test_auto);
	string_list_destructor(&files_test_manual);

	/*
	 * now extract each file from the input
	 */
	config_seen = 0;
	the_config_file = str_from_c(THE_CONFIG_FILE);
	for (j = 0; j < change_set->src->length; ++j)
	{
		cstate_src	src_data;
		output_ty	*ofp;

		/* verbose progress message here? */
		src_data = change_set->src->list[j];
		if
		(
			src_data->action != file_action_create
		&&
			src_data->action != file_action_modify
		)
			continue;
		if (src_data->usage == file_usage_build)
			continue;
		assert(src_data->file_name);
		if (str_equal(src_data->file_name, the_config_file))
		{
			could_have_a_trojan = 1;
			config_seen = 1;
		}
		else if
		(
			src_data->usage == file_usage_test
		||
			src_data->usage == file_usage_manual_test
		)
			could_have_a_trojan = 1;
		archive_name = 0;
		ifp = input_cpio_child(cpio_p, &archive_name);
		if (!ifp)
			input_format_error(cpio_p);
		assert(archive_name);
		s = str_format("src/%S", src_data->file_name);
		if (!str_equal(archive_name, s))
			input_format_error(ifp);
		str_free(s);
		ofp = output_file_binary_open(src_data->file_name->str_text);
		input_to_output(ifp, ofp);
		output_delete(ofp);
		input_delete(ifp);
		str_free(archive_name);
	}
	str_free(the_config_file);

	/*
	 * should be at end of input
	 */
	archive_name = 0;
	ifp = input_cpio_child(cpio_p, &archive_name);
	if (ifp)
		input_format_error(cpio_p);
	input_delete(cpio_p);

	/*
	 * Un-copy any files which did not change.
	 *
	 * The idea is, if there are no files left, there is nothing
	 * for this change to do, so cancel it.
	 */
	if (uncopy)
	{
		s =
			str_format
			(
       "aegis --copy-file-undo --unchanged --change=%ld --project=%S --verbose",
				change_number,
				project_name
			);
		os_execute(s, OS_EXEC_FLAG_INPUT, dd);
		str_free(s);
	
		/*
		 * If there are no files left, we already have this change.
		 */
		os_become_undo();
		if (number_of_files(project_name, change_number) == 0)
		{
			/*
			 * get out of there
			 */
			os_become_orig();
			os_chdir(dot);

			/*
			 * stop developing the change
			 */
			s =
				str_format
				(
	       "aegis --develop-begin-undo --change=%ld --project=%S --verbose",
					change_number,
					project_name
				);
			os_execute(s, OS_EXEC_FLAG_INPUT, dot);
			str_free(s);
	
			/*
			 * cancel the change
			 */
			s =
				str_format
				(
		  "aegis --new-change-undo --change=%ld --project=%S --verbose",
					change_number,
					project_name
				);
			os_execute(s, OS_EXEC_FLAG_INPUT, dot);
			str_free(s);
	
			/*
			 * run away, run away!
			 */
			os_become_undo();
			error_intl(0, i18n("change already present"));
			return;
		}
		os_become_orig();
	}

	if (trojan > 0)
		could_have_a_trojan = 1;
	else if (trojan == 0)
	{
		error_intl
		(
			0,
			i18n("warning: potential trojan, proceeding anyway")
		);
		could_have_a_trojan = 0;
		config_seen = 0;
	}

	/*
	 * If the change could have a trojan horse in the project config
	 * file, stop here with a warning.  Don't even difference the
	 * change, because the trojan could be embedded in the diff
	 * command.  The user needs to look at it and check.
	 *
	 * FIX ME: what if the aecpu got rid of it?
	 */
	if (config_seen)
	{
		error_intl
		(
			0,
	 i18n("warning: potential trojan, review before completing development")
		);
		return;
	}

	/*
	 * now diff the change
	 */
	s =
		str_format
		(
			"aegis --diff --change=%ld --project=%S --verbose",
			change_number,
			project_name
		);
	os_execute(s, OS_EXEC_FLAG_INPUT, dd);
	str_free(s);

	/*
	 * If the change could have a trojan horse in it, stop here with
	 * a warning.  The user needs to look at it and check.
	 */
	if (could_have_a_trojan)
	{
		error_intl
		(
			0,
	 i18n("warning: potential trojan, review before completing development")
		);
		return;
	}

	/*
	 * now build the change
	 */
	s =
		str_format
		(
			"aegis --build --change=%ld --project=%S --verbose",
			change_number,
			project_name
		);
	os_execute(s, OS_EXEC_FLAG_INPUT, dd);
	str_free(s);

	/*
	 * now test the change
	 */
	if (need_to_test)
	{
		s =
			str_format
			(
			     "aegis --test --change=%ld --project=%S --verbose",
				change_number,
				project_name
			);
		os_execute(s, OS_EXEC_FLAG_INPUT, dd);
		str_free(s);

		s =
			str_format
			(
		  "aegis --test --baseline --change=%ld --project=%S --verbose",
				change_number,
				project_name
			);
		os_execute(s, OS_EXEC_FLAG_INPUT, dd);
		str_free(s);
	}

	/* always to a regession test? */

	/*
	 * end development (if we got this far!)
	 */
	s =
		str_format
		(
		      "aegis --develop-end --change=%ld --project=%S --verbose",
			change_number,
			project_name
		);
	os_execute(s, OS_EXEC_FLAG_INPUT, dd);
	str_free(s);

	/* verbose success message here? */

	/*
	 * clean up and go home
	 */
	os_become_undo();
}
