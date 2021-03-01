/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995 Peter Miller;
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
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: functions to implement copy file
 */

#include <stdio.h>
#include <ac/stdlib.h>
#include <ac/unistd.h>

#include <aecp.h>
#include <ael.h>
#include <arglex2.h>
#include <commit.h>
#include <change.h>
#include <error.h>
#include <file.h>
#include <help.h>
#include <lock.h>
#include <log.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <trace.h>
#include <undo.h>
#include <user.h>
#include <word.h>


static void copy_file_usage _((void));

static void
copy_file_usage()
{
	char		*progname;

	progname = option_progname_get();
	fprintf
	(
		stderr,
		"usage: %s -CoPy_file [ <option>... ] <filename>...\n",
		progname
	);
	fprintf
	(
		stderr,
		"       %s -CoPy_file -List [ <option>... ]\n",
		progname
	);
	fprintf(stderr, "       %s -CoPy_file -Help\n", progname);
	quit(1);
}


static void copy_file_help _((void));

static void
copy_file_help()
{
	static char *text[] =
	{
#include <../man1/aecp.h>
	};

	help(text, SIZEOF(text), copy_file_usage);
}


static void copy_file_list _((void));

static void
copy_file_list()
{
	string_ty	*project_name;
	long		change_number;

	trace(("copy_file_list()\n{\n"/*}*/));
	arglex();
	project_name = 0;
	change_number = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(copy_file_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				copy_file_usage();
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				fatal("duplicate -Change option");
			change_number = arglex_value.alv_number;
			if (change_number < 1)
				fatal("change %ld out of range", change_number);
			break;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				copy_file_usage();
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	list_project_files(project_name, change_number);
	if (project_name)
		str_free(project_name);
	trace((/*{*/"}\n"));
}


static void copy_file_main _((void));

static void
copy_file_main()
{
	string_ty	*id;
	string_ty	*dd;
	string_ty	*bl;
	wlist		wl;
	wlist		wl2;
	wlist		wl3;
	string_ty	*s1;
	string_ty	*s2;
	int		stomp;
	cstate		cstate_data;
	pstate		pstate_data;
	int		j, k;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	int		nolog;
	user_ty		*up;
	char		*output;
	long		delta_number;
	char		*delta_name;
	string_ty	*delta_number_implies_edit_number = 0;
	int		config_seen;
	string_ty	*config_name;
	int		number_of_errors;

	trace(("copy_file_main()\n{\n"/*}*/));
	wl_zero(&wl);
	stomp = 0;
	project_name = 0;
	change_number = 0;
	nolog = 0;
	output = 0;
	delta_number = 0;
	delta_name = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(copy_file_usage);
			continue;

		case arglex_token_overwriting:
			if (stomp)
			{
				duplicate:
				fatal
				(
					"duplicate %s option",
					arglex_value.alv_string
				);
			}
			stomp = 1;
			break;

		case arglex_token_file:
		case arglex_token_directory:
			if (arglex() != arglex_token_string)
				copy_file_usage();
			/* fall through... */

		case arglex_token_string:
			s1 = str_from_c(arglex_value.alv_string);
			os_become_orig();
			s2 = os_pathname(s1, 1);
			os_become_undo();
			str_free(s1);
			if (wl_member(&wl, s2))
			{
				fatal
				(
					"file \"%s\" named more than once",
					s2->str_text
				);
			}
			wl_append(&wl, s2);
			str_free(s2);
			break;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				copy_file_usage();
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				fatal("duplicate -Change option");
			change_number = arglex_value.alv_number;
			if (change_number < 1)
				fatal("change %ld out of range", change_number);
			break;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				copy_file_usage();
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_nolog:
			if (nolog)
				goto duplicate;
			nolog = 1;
			break;

		case arglex_token_delta:
			if (delta_number || delta_name)
				goto duplicate;
			switch (arglex())
			{
			default:
				error("the -DELta option must be followed by a name or a number");
				copy_file_usage();
				/*NOTREACHED*/

			case arglex_token_number:
				delta_number = arglex_value.alv_number;
				if (delta_number < 1)
				{
					fatal
					(
						"delta %ld is out of range",
						delta_number
					);
				}
				break;

			case arglex_token_string:
				delta_name = arglex_value.alv_string;
				break;
			}
			break;

		case arglex_token_output:
			if (output)
				goto duplicate;
			switch (arglex())
			{
			default:
				copy_file_usage();

			case arglex_token_stdio:
				output = "";
				break;

			case arglex_token_string:
				output = arglex_value.alv_string;
				break;
			}
			break;
		}
		arglex();
	}
	if (!wl.wl_nwords)
		fatal("no filenames specified");

	/*
	 * make sure output is unambiguous
	 */
	if (output)
	{
		if (wl.wl_nwords != 1)
		{
			fatal
			(
		"only one file may be specifed when the -Output option is used"
			);
		}
		stomp = 1;
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
	 * lock the change file
	 */
	if (!output)
	{
		change_cstate_lock_prepare(cp);
		lock_take();
	}
	cstate_data = change_cstate_get(cp);
	pstate_data = project_pstate_get(pp);

	/*
	 * When there is no explicit output file:
	 * It is an error if the change is not in the being_developed state.
	 * It is an error if the change is not assigned to the current user.
	 */
	if (output)
	{
		switch (cstate_data->state)
		{
		case cstate_state_being_developed:
		case cstate_state_being_reviewed:
		case cstate_state_awaiting_integration:
		case cstate_state_being_integrated:
			break;

		default:
			wrong_state:
			change_fatal
			(
				cp,
"this change is in the '%s' state, \
it must be in the 'being developed' state to modify files with it",
				cstate_state_ename(cstate_data->state)
			);
		}
	}
	else
	{
		if (cstate_data->state != cstate_state_being_developed)
			goto wrong_state;
		if (!str_equal(change_developer_name(cp), user_name(up)))
		{
			change_fatal
			(
				cp,
      "user \"%S\" is not the developer, only user \"%S\" may add copied files",
				user_name(up),
				change_developer_name(cp)
			);
		}
	}

	/*
	 * it is an error if the delta does not exist
	 */
	if (delta_name)
		delta_number = project_delta_name_to_number(pp, delta_name);
	if (delta_number)
	{
		switch (project_delta_exists(pp, delta_number))
		{
		case 0:
			project_fatal(pp, "delta %ld unknown", delta_number);

		case -1:
			/*
			 * the delta given was the current baseline,
			 * don't bother with history
			 *
			 * Except when integrating,
			 * in which case the integration copy may differ.
			 */
			if (cstate_data->state != cstate_state_being_integrated)
				delta_number = 0;
			break;
		}
	}

	/*
	 * resolve the path of each file
	 * 1. the absolute path of the file name is obtained
	 * 2. if the file is inside the development directory, ok
	 * 3. if the file is inside the baseline, ok
	 * 4. if neither, error
	 *
	 * To cope with automounters, directories are stored as given,
	 * or are derived from the home directory in the passwd file.
	 * Within aegis, pathnames have their symbolic links resolved,
	 * and any comparison of paths is done on this "system idea"
	 * of the pathname.
	 */
	config_seen = 0;
	config_name = str_from_c(THE_CONFIG_FILE);
	dd = change_development_directory_get(cp, 1);
	if (cstate_data->state == cstate_state_being_integrated)
		id = change_integration_directory_get(cp, 1);
	else
		id = 0;
	bl = project_baseline_path_get(pp, 1);
	wl_zero(&wl2);
	number_of_errors = 0;
	for (j = 0; j < wl.wl_nwords; ++j)
	{
		s1 = wl.wl_word[j];
		assert(s1->str_text[0] == '/');
		s2 = os_below_dir(dd, s1);
		if (!s2)
			s2 = os_below_dir(bl, s1);
		if (!s2 && id)
			s2 = os_below_dir(id, s1);
		if (!s2)
			change_fatal(cp, "path \"%S\" unrelated", s1);
		if (project_src_dir(pp, s2, &wl3))
		{
			/*
			 * if the user named a directory,
			 * add all of the source files in that directory,
			 * provided they are not already in the change.
			 */
			if (output)
			{
				fatal
				(
			   "may not name directories and use the -Output option"
				);
			}
			for (k = 0; k < wl3.wl_nwords; ++k)
			{
				string_ty	*s3;

				s3 = wl3.wl_word[k];
				if (stomp || !change_src_find(cp, s3))
				{
					wl_append_unique(&wl2, s3);
					if (str_equal(s3, config_name))
						config_seen++;
				}
			}
			wl_free(&wl3);
		}
		else
		{
			wl_append_unique(&wl2, s2);
			if (str_equal(s2, config_name))
				config_seen++;
		}
		str_free(s2);
	}
	wl_free(&wl);
	wl = wl2;
	str_free(config_name);

	/*
	 * ensure that each file
	 * 1. is not already part of the change
	 * 2. is in the baseline
	 */
	for (j = 0; j < wl.wl_nwords; ++j)
	{
		pstate_src	src_data;

		s1 = wl.wl_word[j];
		if (change_src_find(cp, s1) && !stomp && !output)
		{
			change_error
			(
				cp,
"file \"%S\" is already in the change, \
it will not be overwritten; \
use the -OverWriting option if you want to overwrite it",
				s1
			);
			++number_of_errors;
			continue;
		}
		if (output)
		{
			cstate_src	c_src_data;

			/*
			 * OK to use a file that "almost" exists
			 * in combination with the -Output option
			 */
			c_src_data = change_src_find(cp, s1);
			if
			(
				c_src_data
			&&
				c_src_data->action == file_action_create
			)
				continue;
		}
		src_data = project_src_find(pp, s1);
		if
		(
			!src_data
		||
			src_data->about_to_be_created_by
		||
			src_data->deleted_by
		)
		{
			src_data = project_src_find_fuzzy(pp, s1);
			if (src_data)
			{
				project_error
				(
					pp,
			     "file \"%S\" unknown, closest was the \"%S\" file",
					s1,
					src_data->file_name
				);
			}
			else
				project_error(pp, "file \"%S\" unknown", s1);
			++number_of_errors;
			continue;
		}
		if (src_data && src_data->usage == file_usage_build && !output)
		{
			change_error
			(
				cp,
				"file \"%S\" is built, may not copy",
				s1
			);
			++number_of_errors;
		}
	}
	if (number_of_errors)
	{
		change_fatal
		(
			cp,
			"found %d fatal error%s, no files copied",
			number_of_errors,
			(number_of_errors == 1 ? "" : "s")
		);
	}

	/*
	 * Copy each file into the development directory.
	 * Create any necessary directories along the way.
	 */
	for (j = 0; j < wl.wl_nwords; ++j)
	{
		string_ty	*from;
		string_ty	*to;

		s1 = wl.wl_word[j];
		if (delta_number)
		{
			/*
			 * find the edit number,
			 * given fie file name and delta number
			 *
			 * NULL returned means that the file
			 * does not exist at the given delta.
			 */
			delta_number_implies_edit_number =
				project_delta_to_edit(pp, delta_number, s1);
			if (delta_number_implies_edit_number)
			{
				/*
				 * make a temporary file
				 */
				from = os_edit_filename(0);
				user_become(up);
				undo_unlink_errok(from);
				user_become_undo();

				/* 
				 * get the file from history
				 */
				change_run_history_get_command
				(
					cp,
					s1,
					delta_number_implies_edit_number,
					from,
					up
				);
			}
			else
				from = str_from_c("/dev/null");

			/*
			 * figure where to send it
			 */
			if (output)
				to = str_from_c(output);
			else
				to = str_format("%S/%S", dd, s1);

			/*
			 * copy the file
			 */
			user_become(up);
			if (!output)
			{
				os_mkdir_between(dd, s1, 02755);
				if (os_exists(to))
					os_unlink(to);
			}
			copy_whole_file(from, to, 0);

			/*
			 * clean up afterwards
			 */
			if (delta_number_implies_edit_number)
				os_unlink_errok(from);
			user_become_undo();
			str_free(from);
			str_free(to);
		}
		else
		{
			if (id)
				from = str_format("%S/%S", id, s1);
			else
				from = str_format("%S/%S", bl, s1);
			if (output)
				to = str_from_c(output);
			else
				to = str_format("%S/%S", dd, s1);

			/*
			 * copy the file
			 */
			user_become(up);
			if (!output)
			{
				os_mkdir_between(dd, s1, 02755);
				if (os_exists(to))
					os_unlink(to);
			}
			copy_whole_file(from, to, 0);
			user_become_undo();

			/* 
			 * clean up afterwards
			 */
			str_free(from);
			str_free(to);
		}
	}

	/*
	 * Add each file to the change file,
	 * or update the edit number.
	 */
	if (!output)
	{
		for (j = 0; j < wl.wl_nwords; ++j)
		{
			cstate_src	c_src_data;
			pstate_src	p_src_data;
	
			s1 = wl.wl_word[j];
			p_src_data = project_src_find(pp, s1);
			assert(p_src_data);
			c_src_data = change_src_find(cp, s1);
			if (!c_src_data)
			{
				c_src_data = change_src_new(cp);
				c_src_data->file_name = str_copy(s1);
				c_src_data->action = file_action_modify;
				c_src_data->usage = p_src_data->usage;
	
				/*
				 * The change now has at least one test,
				 * so cancel any testing exemption.
				 * (But test_baseline_exempt is still viable.)
				 */
				if
				(
					c_src_data->usage == file_usage_test 
				||
					(
						c_src_data->usage
					==
						file_usage_manual_test
					)
				)
					cstate_data->test_exempt = 0;
			}
			else
				str_free(c_src_data->edit_number);
			assert(p_src_data->edit_number);
			if (delta_number_implies_edit_number)
				c_src_data->edit_number =
					delta_number_implies_edit_number;
			else
				c_src_data->edit_number =
					str_copy(p_src_data->edit_number);
			if (config_seen)
				c_src_data->diff_time = 0;
		}

		/*
		 * the number of files changed,
		 * so stomp on the validation fields.
		 */
		change_build_times_clear(cp);
	}

	/*
	 * release the locks
	 */
	if (!output)
	{
		change_cstate_write(cp);
		commit();
		lock_release();
	}

	/*
	 * verbose success message
	 */
	for (j = 0; j < wl.wl_nwords; ++j)
		change_verbose(cp, "file \"%S\" copied", wl.wl_word[j]);

	/*
	 * run the change file command
	 */
	if (!output)
	{
		if (!nolog)
			log_open(change_logfile_get(cp), up, log_style_append);
		change_run_change_file_command(cp, &wl, up);
	}
	wl_free(&wl);
	project_free(pp);
	change_free(cp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
copy_file()
{
	trace(("copy_file()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		copy_file_main();
		break;

	case arglex_token_help:
		copy_file_help();
		break;

	case arglex_token_list:
		copy_file_list();
		break;
	}
	trace((/*{*/"}\n"));
}
