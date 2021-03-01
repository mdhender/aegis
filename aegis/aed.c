/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999 Peter Miller;
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
 * MANIFEST: difference a change
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/time.h>
#include <ac/unistd.h>

#include <aed.h>
#include <ael/change/files.h>
#include <arglex2.h>
#include <change_bran.h>
#include <change/file.h>
#include <col.h>
#include <commit.h>
#include <error.h>
#include <help.h>
#include <lock.h>
#include <log.h>
#include <os.h>
#include <progname.h>
#include <project.h>
#include <project/file.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>
#include <str_list.h>

#define NOT_SET (-1)


static void difference_usage _((void));

static void
difference_usage()
{
	char		*progname;

	progname = progname_get();
	fprintf
	(
		stderr,
		"usage: %s -DIFFerence [ <option>... ][ <filename>... ]\n",
		progname
	);
	fprintf
	(
		stderr,
		"       %s -DIFFerence -List [ <option>... ]\n",
		progname
	);
	fprintf(stderr, "       %s -DIFFerence -Help\n", progname);
	quit(1);
}


static void difference_help _((void));

static void
difference_help()
{
	help("aed", difference_usage);
}


static void difference_list _((void));

static void
difference_list()
{
	string_ty	*project_name;
	long		change_number;

	trace(("difference_list()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	arglex();
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(difference_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				option_needs_number(arglex_token_change, difference_usage);
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				duplicate_option_by_name(arglex_token_change, difference_usage);
			change_number = arglex_value.alv_number;
			if (change_number == 0)
				change_number = MAGIC_ZERO;
			else if (change_number < 1)
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set(scp, "Number", "%ld", change_number);
				fatal_intl(scp, i18n("change $number out of range"));
				/* NOTREACHED */
				sub_context_delete(scp);
			}
			break;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_project, difference_usage);
			if (project_name)
				duplicate_option_by_name(arglex_token_project, difference_usage);
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}

	list_change_files(project_name, change_number);
	if (project_name)
		str_free(project_name);
	trace((/*{*/"}\n"));
}


static void anticipate _((string_ty *, long, char *, long, log_style_ty, string_list_ty *));

static void
anticipate(project_name, change_number, branch, cn2, log_style, wl)
	string_ty	*project_name;
	long		change_number;
	char		*branch;
	long		cn2;
	log_style_ty	log_style;
	string_list_ty	*wl;
{
	string_ty	*dd1;
	string_ty	*dd2;
	cstate		cstate_data;
	cstate		cstate2_data;
	size_t		j;
	project_ty	*pp;
	change_ty	*cp;
	user_ty		*up;
	change_ty	*acp;
	project_ty	*pp2;

	trace(("anticipate()\n{\n"/*}*/));

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
		pp2 = project_find_branch(pp, branch);
	else
		pp2 = pp;

	/*
	 * locate user data
	 */
	up = user_executing(pp);

	/*
	 * locate change data
	 */
	if (!change_number)
		change_number = user_default_change(up);
	cp = change_alloc(pp, change_number);
	change_bind_existing(cp);
	acp = change_alloc(pp2, cn2);
	change_bind_existing(acp);

	/*
	 * It is an error if the change is not in the being_developed state.
	 * It is an error if the change is not assigned to the current user.
	 * It is an error if the anticipated change is not awaiting
	 * integration or being integrated.
	 */
	cstate_data = change_cstate_get(cp);
	cstate2_data = change_cstate_get(acp);
	if (change_is_a_branch(cp) && cstate_data->state != cstate_state_being_integrated)
		change_fatal(cp, 0, i18n("bad branch diff"));
	else if (cstate_data->state != cstate_state_being_developed)
		change_fatal(cp, 0, i18n("bad diff state"));
	if
	(
		!change_is_a_branch(acp)
	&&
		cstate2_data->state != cstate_state_being_reviewed
	&&
		cstate2_data->state != cstate_state_awaiting_integration
	&&
		cstate2_data->state != cstate_state_being_integrated
	)
	{
		change_fatal(acp, 0, i18n("bad anticipate diff"));
	}
	if (!str_equal(change_developer_name(cp), user_name(up)))
		change_fatal(cp, 0, i18n("not developer"));

	/*
	 * If no files were named on the command line,
	 * then diff every file the two changes have in common.
	 * Ignore all but the most obvious of combinations.
	 */
	dd1 = change_development_directory_get(cp, 1);
	dd2 = change_development_directory_get(acp, 1);
	if (!wl->nstrings)
	{
		for (j = 0; ; ++j)
		{
			fstate_src	src1_data;
			fstate_src	src2_data;

			src1_data = change_file_nth(cp, j);
			if (!src1_data)
				break;
			if (src1_data->action != file_action_modify)
				continue;
			src2_data = change_file_find(acp, src1_data->file_name);
			if (!src2_data)
				continue;
			if (src2_data->action != file_action_modify)
				continue;
			string_list_append(wl, src1_data->file_name);
		}
		if (!wl->nstrings)
			change_fatal(cp, 0, i18n("no suitable files in common"));
	}
	else
	{
		string_list_ty		search_path;
		string_list_ty		search_path2;
		size_t		k;
		int		number_of_errors;
		string_list_ty		wl2;

		/*
		 * resolve the path of each file
		 * 1.	the absolute path of the file name is obtained
		 * 2.	if the file is inside the development directory, ok
		 * 3.	if the file is inside the baseline, ok
		 * 4.	if neither, error
		 */
		number_of_errors = 0;
		change_search_path_get(cp, &search_path, 1);
		change_search_path_get(acp, &search_path2, 1);
		for (k = 0; k < search_path2.nstrings; ++k)
			string_list_append_unique(&search_path, search_path2.string[k]);
		string_list_constructor(&wl2);
		for (j = 0; j < wl->nstrings; ++j)
		{
			string_ty	*s1;
			string_ty	*s2;

			s1 = wl->string[j];
			assert(s1->str_text[0] == '/');
			s2 = 0;
			for (k = 0; k < search_path.nstrings; ++k)
			{
				s2 = os_below_dir(search_path.string[k], s1);
				if (s2)
					break;
			}
			if (!s2)
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set(scp, "File_Name", "%S", s1);
				change_error(cp, scp, i18n("$filename unrelated"));
				sub_context_delete(scp);
				++number_of_errors;
			}
			else
				string_list_append_unique(&wl2, s1);
		}
		string_list_destructor(wl);
		*wl = wl2;
		string_list_destructor(&search_path);
		string_list_destructor(&search_path2);

		/*
		 * confirm that each file is in both changes
		 * and is sensable to compare
		 */
		for (j = 0; j < wl->nstrings; ++j)
		{
			fstate_src	src1_data;
			fstate_src	src2_data;
			string_ty	*s1;

			s1 = wl->string[j];
			src1_data = change_file_find(cp, s1);
			if (!src1_data)
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set(scp, "File_Name", "%S", s1);
				change_error(cp, scp, i18n("no $filename"));
				sub_context_delete(scp);
				++number_of_errors;
				continue;
			}
			if (src1_data->action != file_action_modify)
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set(scp, "File_Name", "%S", s1);
				change_error(cp, scp, i18n("bad cp undo $filename"));
				sub_context_delete(scp);
				++number_of_errors;
				continue;
			}
			src2_data = change_file_find(acp, s1);
			if (!src2_data)
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set(scp, "File_Name", "%S", s1);
				change_error(acp, scp, i18n("no $filename"));
				sub_context_delete(scp);
				++number_of_errors;
				continue;
			}
			if (src2_data->action != file_action_modify)
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set(scp, "File_Name", "%S", s1);
				change_error(acp, scp, i18n("bad cp undo $filename"));
				sub_context_delete(scp);
				++number_of_errors;
				continue;
			}
		}

		/*
		 * abort now if there were errors
		 */
		if (number_of_errors > 0)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "Number", "%d", number_of_errors);
			sub_var_optional(scp, "Number");
			change_fatal(cp, scp, i18n("diff fail"));
			/* NOTREACHED */
			sub_context_delete(scp);
		}
	}

	/*
	 * diff each file
	 */
	log_open(change_logfile_get(cp), up, log_style);
	os_throttle();
	for (j = 0; j < wl->nstrings; ++j)
	{
		fstate_src	src1_data;
		fstate_src	src2_data;
		string_ty	*original;
		string_ty	*most_recent;
		string_ty	*input;
		string_ty	*output;
		int		original_unlink;
		string_ty	*s1;

		s1 = wl->string[j];
		src1_data = change_file_find(cp, s1);
		assert(src1_data);
		src2_data = change_file_find(acp, s1);
		assert(src2_data);

		most_recent = change_file_path(acp, s1);
		assert(most_recent);
		if (change_has_merge_command(cp))
		{
			output = change_file_path(cp, s1);
			assert(output);
			input = str_format("%S,B", output);
			user_become(up);
			os_rename(output, input);
			user_become_undo();
		}
		else
		{
			input = change_file_path(cp, s1);
			assert(input);
			output = str_format("%S,D", input);
		}
		if (change_file_up_to_date(pp2, src1_data))
		{
			/*
			 * if they are both based on the same version,
			 * common ancestor is the baseline
			 */
			original = project_file_path(pp, s1);
			assert(original);
			original_unlink = 0;
		}
		else
		{
			/*
			 * If they are based on different versions,
			 * need to get the original out of history
			 */
			original = os_edit_filename(0);
			original_unlink = 1;
			user_become(up);
			undo_unlink_errok(original);
			user_become_undo();

			if
			(
				src1_data->edit_number
			&&
				!src1_data->edit_number_origin
			)
			{
				/* Historical 2.3 -> 3.0 transition. */
				src1_data->edit_number_origin =
					str_copy(src1_data->edit_number);
			}

			assert(src1_data->edit_number_origin);
			change_run_history_get_command
			(
				cp,
				s1,
				src1_data->edit_number_origin,
				original,
				up
			);
		}

		/*
		 * merge as appropriate
		 */
		if (change_has_merge_command(cp))
		{
			change_run_merge_command
			(
				cp,
				up,
				original,
				most_recent,
				input,
				output
			);
		}
		else
		{
			change_run_diff3_command
			(
				cp,
				up,
				original,
				most_recent,
				input,
				output
			);
		}

		/*
		 * remove the temporary file
		 */
		if (original_unlink)
		{
			user_become(up);
			os_unlink(original);
			user_become_undo();
		}

		/*
		 * make sure the diff is out-of-date,
		 * to force a non-anticipate diff later
		 */
		if (src1_data->diff_file_fp)
		{
			fingerprint_type.free(src1_data->diff_file_fp);
			src1_data->diff_file_fp = 0;
		}

		str_free(original);
		str_free(most_recent);
		str_free(input);
		str_free(output);
	}

	/*
	 * verbose success message
	 */
	change_verbose(cp, 0, i18n("anticipated merge complete"));
	change_free(cp);
	change_free(acp);
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


static void difference_main _((void));

static void
difference_main()
{
	string_ty	*dd;
	string_list_ty	wl, wl2;
	string_ty	*s1 = 0;
	string_ty	*s2;
	string_list_ty	need_new_build;
	cstate		cstate_data;
	size_t		j, k;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	log_style_ty	log_style;
	user_ty		*up;
	long		cn2;
	int		merge_select;
	size_t		mergable_files;
	int		integrating;
	user_ty		*diff_user_p;
	char		*branch;
	project_ty	*pp2;
	project_ty	*pp2bl;
	int		trunk;
	int		grandparent;

	trace(("difference_main()\n{\n"/*}*/));
	string_list_constructor(&wl);
	string_list_constructor(&need_new_build);
	project_name = 0;
	change_number = 0;
	log_style = log_style_snuggle_default;
	cn2 = 0;
	merge_select = NOT_SET;
	branch = 0;
	trunk = 0;
	grandparent = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(difference_usage);
			continue;

		case arglex_token_directory:
			if (arglex() != arglex_token_string)
				option_needs_dir(arglex_token_directory, difference_usage);
			goto get_file_names;


		case arglex_token_file:
			if (arglex() != arglex_token_string)
				option_needs_files(arglex_token_file, difference_usage);
			/* fall through... */

		case arglex_token_string:
			get_file_names:
			s1 = str_from_c(arglex_value.alv_string);
			os_become_orig();
			s2 = os_pathname(s1, 1);
			os_become_undo();
			if (string_list_member(&wl, s2))
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set(scp, "File_Name", "%S", s1);
				fatal_intl(scp, i18n("too many $filename"));
				/* NOTREACHED */
				sub_context_delete(scp);
			}
			string_list_append(&wl, s2);
			str_free(s1);
			str_free(s2);
			break;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				option_needs_number(arglex_token_change, difference_usage);
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				duplicate_option_by_name(arglex_token_change, difference_usage);
			change_number = arglex_value.alv_number;
			if (change_number == 0)
				change_number = MAGIC_ZERO;
			else if (change_number < 1)
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set(scp, "Number", "%ld", change_number);
				fatal_intl(scp, i18n("change $number out of range"));
				/* NOTREACHED */
				sub_context_delete(scp);
			}
			break;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_project, difference_usage);
			if (project_name)
				duplicate_option_by_name(arglex_token_project, difference_usage);
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_nolog:
			if (log_style == log_style_none)
				duplicate_option(difference_usage);
			log_style = log_style_none;
			break;

		case arglex_token_anticipate:
			if (cn2)
				duplicate_option(difference_usage);
			if (arglex() != arglex_token_number)
				option_needs_number(arglex_token_anticipate, difference_usage);
			cn2 = arglex_value.alv_number;
			if (cn2 == 0)
				cn2 = MAGIC_ZERO;
			else if (cn2 < 1)
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set(scp, "Number", "%ld", cn2);
				fatal_intl(scp, i18n("change $number out of range"));
				/* NOTREACHED */
				sub_context_delete(scp);
			}
			break;

		case arglex_token_merge_not:
			if (merge_select == uconf_diff_preference_no_merge)
				duplicate_option(difference_usage);
			if (merge_select != NOT_SET)
			{
				too_many_merges:
				mutually_exclusive_options3
				(
					arglex_token_merge_not,
					arglex_token_merge_only,
					arglex_token_merge_automatic,
					difference_usage
				);
			}
			merge_select = uconf_diff_preference_no_merge;
			break;

		case arglex_token_merge_only:
			if (merge_select == uconf_diff_preference_only_merge)
				duplicate_option(difference_usage);
			if (merge_select != NOT_SET)
				goto too_many_merges;
			merge_select = uconf_diff_preference_only_merge;
			break;

		case arglex_token_merge_automatic:
			if (merge_select == uconf_diff_preference_automatic_merge)
				duplicate_option(difference_usage);
			if (merge_select != NOT_SET)
				goto too_many_merges;
			merge_select = uconf_diff_preference_automatic_merge;
			break;

		case arglex_token_branch:
			if (branch)
				duplicate_option(difference_usage);
			switch (arglex())
			{
			default:
				option_needs_number(arglex_token_branch, difference_usage);

			case arglex_token_number:
			case arglex_token_string:
				branch = arglex_value.alv_string;
				break;

			case arglex_token_stdio:
				branch = "";
				break;
			}
			break;

		case arglex_token_trunk:
			if (trunk)
				duplicate_option(difference_usage);
			++trunk;
			break;

		case arglex_token_grandparent:
			if (grandparent)
				duplicate_option(difference_usage);
			++grandparent;
			break;

		case arglex_token_wait:
		case arglex_token_wait_not:
			user_lock_wait_argument(difference_usage);
			break;
		}
		arglex();
	}
	if (grandparent)
	{
		if (branch)
		{
			mutually_exclusive_options
			(
				arglex_token_branch,
				arglex_token_grandparent,
				difference_usage
			);
		}
		if (trunk)
		{
			mutually_exclusive_options
			(
				arglex_token_trunk,
				arglex_token_grandparent,
				difference_usage
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
				difference_usage
			);
		}
		branch = "";
	}

	if (cn2)
	{
		if (merge_select != NOT_SET)
			fatal_intl(0, i18n("no merge anticipate"));
		anticipate(project_name, change_number, branch, cn2, log_style, &wl);
		trace((/*{*/"}\n"));
		return;
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
	 * locate which branch
	 */
	if (branch)
		pp2 = project_find_branch(pp, branch);
	else
		pp2 = pp;

	/*
	 * locate user data
	 */
	up = user_executing(pp);

	/*
	 * locate change data
	 */
	if (!change_number)
		change_number = user_default_change(up);
	cp = change_alloc(pp, change_number);
	change_bind_existing(cp);

	/*
	 * lock on the appropriate row of the change table.
	 *
	 * Also take a read lock on the baseline, to ensure that it does
	 * not change (aeip) for the duration of the diff/merge.
	 */
	change_cstate_lock_prepare(cp);
	project_baseline_read_lock_prepare(pp);
	lock_take();

	/*
	 * It is an error if the change is not in the being_developed or
	 * being_integrated state.
	 * It is an error if the change is not assigned to the current user.
	 */
	cstate_data = change_cstate_get(cp);
	if
	(
		change_is_a_branch(cp)
	&&
		cstate_data->state != cstate_state_being_integrated
	)
		change_fatal(cp, 0, i18n("bad branch diff"));
	if
	(
		cstate_data->state != cstate_state_being_developed
	&&
		cstate_data->state != cstate_state_being_integrated
	)
		change_fatal(cp, 0, i18n("bad idiff state"));
	integrating = (cstate_data->state == cstate_state_being_integrated);
	if (integrating)
	{
		if (!str_equal(change_integrator_name(cp), user_name(up)))
			change_fatal(cp, 0, i18n("not integrator"));
	}
	else
	{
		if (!str_equal(change_developer_name(cp), user_name(up)))
			change_fatal(cp, 0, i18n("not developer"));
	}

	/*
	 * the branch option implies a merge
	 */
	if (branch)
	{
		if (merge_select == uconf_diff_preference_no_merge)
		{
			mutually_exclusive_options
			(
				arglex_token_branch,
				arglex_token_merge_not,
				difference_usage
			);
		}
		if (merge_select == uconf_diff_preference_automatic_merge)
		{
			mutually_exclusive_options
			(
				arglex_token_branch,
				arglex_token_merge_automatic,
				difference_usage
			);
		}
		if (integrating)
			change_fatal(cp, 0, i18n("no cross branch int merge"));
		merge_select = uconf_diff_preference_only_merge;
	}

	/*
	 * Now that we know the change state, we can set the merge
	 * preference.  It is an error to request a merge in the 'being
	 * integrated' state.
	 */
	if (integrating)
	{
		if (merge_select == uconf_diff_preference_only_merge)
			change_fatal(cp, 0, i18n("no int merge"));
		merge_select = uconf_diff_preference_no_merge;
	}
	else
	{
		if (merge_select == NOT_SET)
			merge_select = user_diff_preference(up);
	}

	/*
	 * if no files were named on the command line,
	 * then diff every file in the change.
	 */
	if (!wl.nstrings)
	{
		if (!change_file_nth(cp, 0))
			change_fatal(cp, 0, i18n("no files"));
		for (j = 0; ; ++j)
		{
			fstate_src	src_data;

			src_data = change_file_nth(cp, j);
			if (!src_data)
				break;
			string_list_append(&wl, src_data->file_name);
		}
	}
	else
	{
		string_list_ty		search_path;
		int		number_of_errors;

		/*
		 * When the change is being integrated, you must
		 * difference everything.  You may not name files on the
		 * command line.
		 */
		number_of_errors = 0;
		if (integrating)
		{
			change_error(cp, 0, i18n("int must diff all"));
			++number_of_errors;
		}

		/*
		 * resolve the path of each file
		 * 1.	the absolute path of the file name is obtained
		 * 2.	if the file is inside the development directory, ok
		 * 3.	if the file is inside the baseline, ok
		 * 4.	if neither, error
		 */
		change_search_path_get(cp, &search_path, 1);
		string_list_constructor(&wl2);
		for (j = 0; j < wl.nstrings; ++j)
		{
			s1 = wl.string[j];
			assert(s1->str_text[0] == '/');
			s2 = 0;
			for (k = 0; k < search_path.nstrings; ++k)
			{
				s2 = os_below_dir(search_path.string[k], s1);
				if (s2)
					break;
			}
			if (!s2)
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set(scp, "File_Name", "%S", s1);
				change_error(cp, scp, i18n("$filename unrelated"));
				sub_context_delete(scp);
				++number_of_errors;
			}
			else
			{
				string_list_append_unique(&wl2, s2);
				str_free(s2);
			}
		}
		string_list_destructor(&search_path);
		string_list_destructor(&wl);
		wl = wl2;

		/*
		 * confirm that each file is in the change
		 * and resolve directories into files
		 */
		string_list_constructor(&wl2);
		for (j = 0; j < wl.nstrings; ++j)
		{
			string_list_ty		wl_in;

			s1 = wl.string[j];
			change_file_directory_query(cp, s1, &wl_in, 0);
			if (wl_in.nstrings)
				string_list_append_list_unique(&wl2, &wl_in);
			else if (change_file_find(cp, s1))
				string_list_append_unique(&wl2, s1);
			else
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set(scp, "File_Name", "%S", s1);
				change_error(cp, scp, i18n("no $filename"));
				sub_context_delete(scp);
				++number_of_errors;
			}
			string_list_destructor(&wl_in);
		}
		string_list_destructor(&wl);
		wl = wl2;

		/*
		 * abort now if there were errors
		 */
		if (number_of_errors > 0)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "Number", "%d", number_of_errors);
			sub_var_optional(scp, "Number");
			change_fatal(cp, scp, i18n("diff fail"));
			sub_context_delete(scp);
		}
	}

	/*
	 * look for files which need to be merged
	 */
	mergable_files = 0;
	for (j = 0; j < wl.nstrings; ++j)
	{
		fstate_src	src1_data;
		fstate_src	src2_data;

		/*
		 * find the relevant change src data
		 */
		s1 = wl.string[j];
		src1_data = change_file_find(cp, s1);
		assert(src1_data);

		/*
		 * generated files are not merged
		 * created or deleted files are not merged
		 */
		switch (src1_data->usage)
		{
		case file_usage_build:
			/* toss these ones */
			continue;

		case file_usage_source:
		case file_usage_test:
		case file_usage_manual_test:
			/* keep these ones */
			break;
		}
		switch (src1_data->action)
		{
		case file_action_create:
		case file_action_remove:
			/* toss these ones */
			continue;

		case file_action_modify:
		case file_action_insulate:
			/* keep these ones */
			break;
		}

		/*
		 * find the relevant baseline src data
		 * note that someone may have deleted it from under you
		 *
		 * If the edit numbers match (is up to date)
		 * then do not merge this one.
		 */
		src2_data = project_file_find(pp2, s1);
		if (!src2_data)
			continue;
		if (change_file_up_to_date(pp2, src1_data))
			continue;
		
		/*
		 * this one needs merging
		 */
		++mergable_files;
	}

	/*
	 * figure what to do if the user (indirectly)
	 * selected the automatic_merge option
	 *
	 * DO NOT merge files when the change is in the 'being
	 * integrated' state, no matter what the user preference
	 * indicates.  This was taken care of earlier.
	 */
	if (merge_select == uconf_diff_preference_automatic_merge)
	{
		if (mergable_files)
			merge_select = uconf_diff_preference_only_merge;
		else
			merge_select = uconf_diff_preference_no_merge;
	}

	/*
	 * Differences in the 'being integrated' state are done
	 * by the project owner, not the integrator.  (There is
	 * a known memory leak: the project user is not free()ed
	 * at the end of the loop.)
	 */
	pp2bl = pp2;
	if (integrating)
	{
		diff_user_p = project_user(pp);
		if (log_style == log_style_snuggle_default)
			log_style = log_style_append_default;

		/*
		 * when integrating, the baseline is one project deeper
		 */
		if (pp2->parent)
			pp2bl = pp2->parent;
	}
	else
		diff_user_p = up;

	log_open(change_logfile_get(cp), diff_user_p, log_style);
	if (integrating)
		dd = change_integration_directory_get(cp, 0);
	else
		dd = change_development_directory_get(cp, 0);
	os_throttle();

	if (merge_select == uconf_diff_preference_only_merge)
	{
		/*
		 * merge each file
		 */
		for (j = 0; j < wl.nstrings; ++j)
		{
			fstate_src	src1_data;
			fstate_src	src2_data;
			string_ty	*original;
			string_ty	*curfile;
			string_ty	*outname;
			string_ty	*most_recent;

			/*
			 * find the relevant change src data
			 */
			s1 = wl.string[j];
			src1_data = change_file_find(cp, s1);
			assert(src1_data);

			/*
			 * generated files are not merged
			 * created or deleted files are not merged
			 *
			 * Note: Insulation files are merged! This is
			 * important if they have been silly enough to
			 * change them.
			 */
			switch (src1_data->usage)
			{
			case file_usage_build:
				/* toss these ones */
				continue;

			case file_usage_source:
			case file_usage_test:
			case file_usage_manual_test:
				/* keep these ones */
				break;
			}
			switch (src1_data->action)
			{
			case file_action_create:
			case file_action_remove:
				/* toss these ones */
				continue;

			case file_action_modify:
			case file_action_insulate:
				/* keep these ones */
				break;
			}

			/*
			 * find the relevant baseline src data
			 * note that someone may have deleted it from under you
			 *
			 * If the edit numbers match (is up to date)
			 * then do not merge this one.
			 */
			src2_data = project_file_find(pp2, s1);
			if
			(
				src2_data
			&&
				(
					src2_data->deleted_by
				||
					src2_data->about_to_be_created_by
				)
			)
				src2_data = 0;
			if (!src2_data)
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set(scp, "File_Name", "%S", s1);
				change_fatal
				(
					cp,
					scp,
					i18n("no $filename in baseline")
				);
				sub_context_delete(scp);
			}
			if (change_file_up_to_date(pp2, src1_data))
				continue;

			/*
			 * build various paths
			 */
			if (change_has_merge_command(cp))
			{
				outname = change_file_path(cp, s1);
				trace_string(outname->str_text);
				curfile = str_format("%S,B", outname);
				trace_string(curfile->str_text);
				user_become(diff_user_p);
				os_rename(outname, curfile);
				user_become_undo();
			}
			else
			{
				curfile = change_file_path(cp, s1);
				trace_string(curfile->str_text);
				outname = str_format("%S,D", curfile);
				trace_string(outname->str_text);
			}

			/*
			 * name for temp file
			 */
			original = os_edit_filename(0);
			user_become(diff_user_p);
			undo_unlink_errok(original);
			user_become_undo();

			if
			(
				src1_data->edit_number
			&&
				!src1_data->edit_number_origin
			)
			{
				/* Historical 2.3 -> 3.0 transition. */
				src1_data->edit_number_origin =
					str_copy(src1_data->edit_number);
			}

			/*
			 * get the version out of history
			 */
			assert(src1_data->edit_number_origin);
			change_run_history_get_command
			(
				cp,
				s1,
				src1_data->edit_number_origin,
				original,
				diff_user_p
			);

			/*
			 * use the appropriate merge command
			 */
			most_recent = project_file_path(pp2, s1);
			assert(most_recent);
			if (change_has_merge_command(cp))
			{
				change_run_merge_command
				(
					cp,
					diff_user_p,
					original,
					most_recent,
					curfile,
					outname
				);
			}
			else
			{
				change_run_diff3_command
				(
					cp,
					diff_user_p,
					original,
					most_recent,
					curfile,
					outname
				);
			}
			str_free(most_recent);

			/*
			 * Remember to remove the temporary file when
			 * finished.
			 */
			user_become(diff_user_p);
			os_unlink(original);
			user_become_undo();
			str_free(original);

			/*
			 * p_src_data->edit_number
			 *	The head revision of the branch.
			 * p_src_data->edit_number_origin
			 *	The version originally copied.
			 *
			 * c_src_data->edit_number
			 *	Not meaningful until after integrate pass.
			 * c_src_data->edit_number_origin
			 *	The version originally copied.
			 * c_src_data->edit_number_origin_new
			 *	Updates branch edit_number_origin on
			 *	integrate pass.
			 */
			assert(src2_data->edit_number);
			if (cp->pp == pp2)
			{
				/* normal merge */
				if (src1_data->edit_number_origin)
					str_free(src1_data->edit_number_origin);
				src1_data->edit_number_origin =
					str_copy(src2_data->edit_number);
			}
			else
			{
				/* cross branch merge */
				if (src1_data->edit_number_origin_new)
					str_free
					(
					       src1_data->edit_number_origin_new
					);
				src1_data->edit_number_origin_new =
					str_copy(src2_data->edit_number);
			}

			/*
			 * Invalidate the file time stamps.
			 */
			if (src1_data->file_fp)
			{
				fingerprint_type.free(src1_data->file_fp);
				src1_data->file_fp = 0;
			}
			if (src1_data->diff_file_fp)
			{
				fingerprint_type.free(src1_data->diff_file_fp);
				src1_data->diff_file_fp = 0;
			}

			/*
			 * Remember this file name, so that we can give
			 * a verbose success message at the end.
			 */
			string_list_append(&need_new_build, src1_data->file_name);

			/*
			 * Nuke the build times.
			 */
			change_build_times_clear(cp);
			str_free(curfile);
			str_free(outname);
		}

		/*
		 * write the data back
		 *	(always do this, to ensure fstate is correct)
		 */
		change_cstate_write(cp);
		commit();
		lock_release();

		for (j = 0; j < need_new_build.nstrings; ++j)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", need_new_build.string[j]);
			if (change_has_merge_command(cp))
				change_error(cp, scp, i18n("$filename merged"));
			else
				change_error(cp, scp, i18n("$filename diff3ed"));
			sub_context_delete(scp);
		}

		if (need_new_build.nstrings)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "Number", "%ld", (long)need_new_build.nstrings);
			sub_var_optional(scp, "Number");
			change_error(cp, scp, i18n("merge complete"));
			sub_context_delete(scp);
		}
		string_list_destructor(&need_new_build);
	}
	else
	{
		if (mergable_files)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "Number", "%ld", mergable_files);
			sub_var_optional(scp, "Number");
			change_verbose(cp, scp, i18n("warning: mergable files"));
			sub_context_delete(scp);
		}

		/*
		 * diff each file
		 */
		for (j = 0; j < wl.nstrings; ++j)
		{
			fstate_src	src1_data;
			fstate_src	src2_data;
			string_ty	*original;
			string_ty	*input;
			string_ty	*path;
			string_ty	*path_d;
			int		ignore;

			/*
			 * find the relevant change src data
			 */
			s1 = wl.string[j];
			src1_data = change_file_find(cp, s1);
			if (!src1_data)
				this_is_a_bug();

			/*
			 * generated files are not differenced
			 */
			if (src1_data->usage == file_usage_build)
				continue;

			/*
			 * the removed half of a move is not differenced
			 */
			if
			(
				src1_data->action == file_action_remove
			&&
				src1_data->move
			&&
				change_file_find(cp, src1_data->move)
			)
				continue;

			/*
			 * build various paths
			 */
			path = change_file_path(cp, s1);
			assert(path);
			trace_string(path->str_text);
			path_d = str_format("%S,D", path);
			trace_string(path_d->str_text);

			/*
			 * Check the file's fingerprint.  This will zap
			 * the other timestamps if the fingerprint has
			 * changed.
			 */
			if (src1_data->action != file_action_remove && !integrating)
			{
				change_file_fingerprint_check(cp, src1_data);
				assert(src1_data->file_fp);
				assert(src1_data->file_fp->youngest > 0);
				assert(src1_data->file_fp->oldest > 0);
			}

			/*
			 * All change files have a difference file,
			 * eventually, even files being removed and
			 * created.  Do nothing if we can.
			 */
			user_become(diff_user_p);
			if (integrating)
				ignore = change_fingerprint_same(src1_data->idiff_file_fp, path_d, 0);
			else
				ignore = change_fingerprint_same(src1_data->diff_file_fp, path_d, 0);
			user_become_undo();
			if (ignore)
			{
				str_free(path);
				str_free(path_d);
				continue;
			}

			/*
			 * locate the equivalent project file
			 */
			src2_data = project_file_find(pp2bl, s1);
			if
			(
				src2_data
			&&
				(
					src2_data->deleted_by
				||
					src2_data->about_to_be_created_by
				)
			)
				src2_data = 0;

			switch (src1_data->action)
			{
			case file_action_create:
				/*
				 * check if someone created it ahead of you
				 */
				if (src2_data && !integrating)
				{
					sub_context_ty	*scp;

					scp = sub_context_new();
					sub_var_set(scp, "File_Name", "%S", s1);
					change_verbose
					(
						cp,
						scp,
				 i18n("warning: $filename in baseline, copying")
					);
					sub_context_delete(scp);
					src1_data->action = file_action_modify;
					if (src2_data->edit_number_origin)
					{
						src1_data->edit_number_origin =
							str_copy
							(
							  src2_data->edit_number
							);
					}
					goto modifying_file;
				}
				if (src2_data && integrating)
					goto modifying_file;

				/*
				 * difference the file
				 * from nothing
				 */
				creating_file:
				if
				(
					src1_data->move
				&&
					project_file_find(pp2bl, src1_data->move)
				)
				{
					original =
						project_file_path
						(
							pp2bl,
							src1_data->move
						);
					assert(original);
				}
				else
					original = str_from_c("/dev/null");
				change_run_diff_command
				(
					cp,
					diff_user_p,
					original,
					path,
					path_d
				);
				str_free(original);

				/*
				 * remember the new fingerprint
				 */
				set_fingerprint:
				user_become(diff_user_p);
				if (integrating)
				{
					if (!src1_data->idiff_file_fp)
						src1_data->idiff_file_fp =
						       fingerprint_type.alloc();
					src1_data->idiff_file_fp->youngest = 0;
					change_fingerprint_same(src1_data->idiff_file_fp, path_d, 0);
				}
				else
				{
					if (!src1_data->diff_file_fp)
						src1_data->diff_file_fp =
						       fingerprint_type.alloc();
					src1_data->diff_file_fp->youngest = 0;
					change_fingerprint_same(src1_data->diff_file_fp, path_d, 0);
				}
				user_become_undo();
				break;

			case file_action_remove:
				/*
				 * check if someone deleted it ahead of you
				 */
				if (!src2_data && !integrating)
				{
					sub_context_ty	*scp;

					scp = sub_context_new();
					sub_var_set(scp, "File_Name", "%S", s1);
					change_fatal
					(
						cp,
						scp,
						i18n("no $filename in baseline")
					);
					sub_context_delete(scp);
				}

				/*
				 * create directory for diff file
				 */
				user_become(diff_user_p);
				os_mkdir_between(dd, s1, 02755);
				user_become_undo();

				/*
				 * difference the file
				 * to nothing
				 */
				if (src2_data)
					original = project_file_path(pp2bl, s1);
				else
					original = str_from_c("/dev/null");
				assert(original);
				if
				(
					src1_data->move
				&&
					change_file_find(cp, src1_data->move)
				)
				{
					input =
						change_file_path
						(
							cp,
							src1_data->move
						);
					assert(input);
				}
				else
					input = str_from_c("/dev/null");
				change_run_diff_command
				(
					cp,
					diff_user_p,
					original,
					input,
					path_d
				);
				str_free(original);
				str_free(input);
				goto set_fingerprint;

			case file_action_modify:
				/*
				 * Check that nobody has
				 * deleted it from under you.
				 */
				if (!src2_data && !integrating)
				{
					sub_context_ty	*scp;

					scp = sub_context_new();
					sub_var_set(scp, "File_Name", "%S", s1);
					change_verbose
					(
						cp,
						scp,
			     i18n("warning: no $filename in baseline, creating")
					);
					sub_context_delete(scp);
					src1_data->action = file_action_create;
					goto creating_file;
				}
				if (!src2_data && integrating)
					goto creating_file;

				/*
				 * we did merges earlier,
				 * should not be necessary here
				 */
				assert(change_file_up_to_date(pp2, src1_data));

				/*
				 * use the diff-command
				 */
				modifying_file:
				original = project_file_path(pp2bl, s1);
				assert(original);
				change_run_diff_command
				(
					cp,
					diff_user_p,
					original,
					path,
					path_d
				);
				str_free(original);
				goto set_fingerprint;

			case file_action_insulate:
				/*
				 * At integration time, ignore read only
				 * entries.
				 */
				if (integrating)
					break;

				/*
				 * If the file has been deleted from the
				 * baseline since the read only copy was
				 * taken, pretend we are creating it.
				 */
				if (!src2_data)
					goto creating_file;

				/*
				 * Pretend we are modifying the file.
				 * This file is part of the change to
				 * insulate the change from the
				 * baseline.  Any modifications are a
				 * Bad Thing.
				 */
				goto modifying_file;
			}
			str_free(path);
			str_free(path_d);
		}

		/*
		 * If the change row (or change file table) changed,
		 * write it out.
		 * Release advisory lock.
		 */
		change_cstate_write(cp);
		commit();
		lock_release();

		/*
		 * verbose success message
		 */
		change_verbose(cp, 0, i18n("difference complete"));
	}
	change_free(cp);
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
difference()
{
	trace(("difference()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		difference_main();
		break;

	case arglex_token_help:
		difference_help();
		break;

	case arglex_token_list:
		difference_list();
		break;
	}
	trace((/*{*/"}\n"));
}
