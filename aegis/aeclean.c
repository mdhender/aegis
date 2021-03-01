/*
 *	aegis - project change supervisor
 *	Copyright (C) 1998, 1999 Peter Miller;
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
 * MANIFEST: clean a change development directory
 */

#include <ac/errno.h>
#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/time.h>
#include <ac/unistd.h>

#include <aeclean.h>
#include <ael/change/files.h>
#include <arglex2.h>
#include <change.h>
#include <change/file.h>
#include <commit.h>
#include <dir.h>
#include <error.h>
#include <glue.h>
#include <help.h>
#include <lock.h>
#include <log.h>
#include <os.h>
#include <progname.h>
#include <project/file.h>
#include <str_list.h>
#include <sub.h>
#include <trace.h>
#include <user.h>


static void clean_usage _((void));

static void
clean_usage()
{
	char		*progname;

	progname = progname_get();
	fprintf(stderr, "usage: %s -CLEan [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -CLEan -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -CLEan -Help\n", progname);
	quit(1);
}


static void clean_help _((void));

static void
clean_help()
{
	help("aeclean", clean_usage);
}


static void clean_list _((void));

static void
clean_list()
{
	string_ty	*project_name;
	long		change_number;

	trace(("clean_list()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	arglex();
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(clean_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				option_needs_number(arglex_token_change, clean_usage);
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				duplicate_option_by_name(arglex_token_change, clean_usage);
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
				option_needs_name(arglex_token_project, clean_usage);
			if (project_name)
				duplicate_option_by_name(arglex_token_project, clean_usage);
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


typedef struct clean_info_ty clean_info_ty;
struct clean_info_ty
{
	string_ty	*dd;
	change_ty	*cp;
	int		minimum;
	user_ty		*up;
};


static void clean_out_the_garbage _((void *, dir_walk_message_ty, string_ty *,
	struct stat *));

static void
clean_out_the_garbage(p, msg, path, st)
	void		*p;
	dir_walk_message_ty msg;
	string_ty	*path;
	struct stat	*st;
{
	clean_info_ty	*sip;
	string_ty	*s1;
	int		delete_me;

	sip = p;
	switch (msg)
	{
	case dir_walk_dir_before:
		break;

	case dir_walk_dir_after:
		/*
		 * Try to remove each directory.  This makes directories
		 * where we deleted everything else go away, which is
		 * usually what is desired.  It isn't an error if the
		 * rmdir fails because it isn't empty, so quietly ignore
		 * those errors.
		 *
		 * Exception: Don't remove the development directory!
		 */
		if (str_equal(path, sip->dd))
			break;
		if (glue_rmdir(path->str_text) && errno != ENOTEMPTY)
		{
			sub_context_ty  *scp;

			scp = sub_context_new();
			sub_errno_set(scp);
			sub_var_set(scp, "File_Name", "%S", path);
			error_intl(scp, i18n("warning: rmdir $filename: $errno"));
			sub_context_delete(scp);
		}
		break;

	case dir_walk_special:
	case dir_walk_symlink:
		/*
		 * This can't be a change source file, so always
		 * delete it.
		 */
		os_unlink_errok(path);
		break;

	case dir_walk_file:
		/*
		 * leave difference files alone
		 */
		if
		(
			path->str_length > 2
		&&
			path->str_text[path->str_length - 2] == ','
		&&
			path->str_text[path->str_length - 1] == 'D'
		)
			break;

		/*
		 * Find the pathname relative to the development directory.
		 */
		s1 = os_below_dir(sip->dd, path);
		assert(s1);

		/*
		 * don't delete change files
		 */
		delete_me = 1;
		user_become_undo();
		if (change_file_find(sip->cp, s1))
			delete_me = 0;

		/*
		 * The minimum option says to leave regular files
		 * which obscure project files (though aecp -ro
		 * would have been more appropriate).
		 */
		if (sip->minimum && project_file_find(sip->cp->pp, s1))
			delete_me = 0;
		user_become(sip->up);

		/*
		 * delete the file
		 */
		if (delete_me)
			os_unlink_errok(path);
		str_free(s1);
		break;
	}
}


static void clean_main _((void));

static void
clean_main()
{
	sub_context_ty	*scp;
	string_ty	*dd;
	cstate		cstate_data;
	size_t		j;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	log_style_ty	log_style;
	user_ty		*up;
	int		mergeable_files;
	int		diffable_files;
	string_list_ty	wl;
	fstate_src	p_src_data;
	fstate_src	c_src_data;
	pconf		pconf_data;
	int		minimum;
	clean_info_ty	info;

	trace(("clean_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	log_style = log_style_snuggle_default;
	minimum = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(clean_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				option_needs_number(arglex_token_change, clean_usage);
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				duplicate_option_by_name(arglex_token_change, clean_usage);
			change_number = arglex_value.alv_number;
			if (change_number == 0)
				change_number = MAGIC_ZERO;
			else if (change_number < 1)
			{
				scp = sub_context_new();
				sub_var_set(scp, "Number", "%ld", change_number);
				fatal_intl(scp, i18n("change $number out of range"));
				/* NOTREACHED */
				sub_context_delete(scp);
			}
			break;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_project, clean_usage);
			if (project_name)
				duplicate_option_by_name(arglex_token_project, clean_usage);
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_nolog:
			if (log_style == log_style_none)
				duplicate_option(clean_usage);
			log_style = log_style_none;
			break;

		case arglex_token_wait:
		case arglex_token_wait_not:
			user_lock_wait_argument(clean_usage);
			break;

		case arglex_token_minimum:
			if (minimum)
				duplicate_option(clean_usage);
			minimum = 1;
			break;
		}
		arglex();
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
	 * lock on the appropriate row of the change table.
	 */
	change_cstate_lock_prepare(cp);
	lock_take();

	/*
	 * It is an error if the change is not in the being_developed state.
	 * It is an error if the change is not assigned to the current user.
	 */
	cstate_data = change_cstate_get(cp);
	if (cstate_data->state != cstate_state_being_developed)
		change_fatal(cp, 0, i18n("bad clean state"));
	if (!str_equal(change_developer_name(cp), user_name(up)))
		change_fatal(cp, 0, i18n("not developer"));

	dd = change_development_directory_get(cp, 0);
	os_throttle();

	/*
	 * open the log file
	 */
	log_open(change_logfile_get(cp), up, log_style);

	/*
	 * look for files which need to be merged
	 */
	mergeable_files = 0;
	diffable_files = 0;
	for (j = 0; ; ++j)
	{
		string_ty	*path;
		string_ty	*path_d;
		int		exists;
		int		ignore;

		/*
		 * find the relevant change src data
		 */
		c_src_data = change_file_nth(cp, j);
		if (!c_src_data)
			break;

		/*
		 * generated files are not merged
		 * created or deleted files are not merged
		 */
		switch (c_src_data->usage)
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
		switch (c_src_data->action)
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
		p_src_data = project_file_find(pp, c_src_data->file_name);
		if (!p_src_data)
			continue;

		/*
		 * check for files which need to be merged
		 */
		if (!change_file_up_to_date(pp, c_src_data))
		{
			/*
			 * this one needs merging
			 */
			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", c_src_data->file_name);
			change_verbose(cp, scp, i18n("warning: file \"$filename\" needs merge"));
			sub_context_delete(scp);
			++mergeable_files;
			continue;
		}

		/*
		 * the removed half of a move is not differenced
		 */
		if
		(
			c_src_data->action == file_action_remove
		&&
			c_src_data->move
		&&
			change_file_find(cp, c_src_data->move)
		)
			continue;

		/*
		 * build various paths
		 */
		path = change_file_path(cp, c_src_data->file_name);
		assert(path);
		trace_string(path->str_text);

		/*
		 * make sure the change sourec file even exists
		 */
		user_become(up);
		exists = os_exists(path);
		user_become_undo();
		if (!exists)
		{
			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", c_src_data->file_name);
			change_error(cp, scp, i18n("file \"$filename\" does not exist"));
			sub_context_delete(scp);
			str_free(path);
			++diffable_files;
			continue;
		}
		path_d = str_format("%S,D", path);
		trace_string(path_d->str_text);
	
		/*
		 * Check the file's fingerprint.  This will zap
		 * the other timestamps if the fingerprint has
		 * changed.
		 */
		if (c_src_data->action != file_action_remove)
			change_file_fingerprint_check(cp, c_src_data);
	
		/*
		 * See if we need to diff the file
		 */
		user_become(up);
		ignore = change_fingerprint_same(c_src_data->diff_file_fp, path_d, 0);
		user_become_undo();
		if (!ignore)
		{
			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", c_src_data->file_name);
			change_verbose(cp, scp, i18n("warning: file \"$filename\" needs diff"));
			sub_context_delete(scp);
			++diffable_files;
		}
		str_free(path);
		str_free(path_d);
	}
	if (mergeable_files)
	{
		scp = sub_context_new();
		sub_var_set(scp, "Number", "%ld", mergeable_files);
		sub_var_optional(scp, "Number");
		change_verbose(cp, scp, i18n("warning: mergable files"));
		sub_context_delete(scp);
	}
	if (diffable_files)
	{
		scp = sub_context_new();
		sub_var_set(scp, "Number", "%ld", diffable_files);
		sub_var_optional(scp, "Number");
		change_verbose(cp, scp, i18n("warning: diffable files"));
		sub_context_delete(scp);
	}

	/*
	 * now walk the change directory tree,
	 * looking for files to throw away
	 */
	info.cp = cp;
	info.minimum = minimum;
	info.up = up;
	info.dd = change_development_directory_get(cp, 1);
	user_become(up);
	dir_walk(info.dd, clean_out_the_garbage, &info);
	user_become_undo();

	/*
	 * create the symbolic links again, if required
	 */
	pconf_data = change_pconf_get(cp, 0);
	if
	(
		pconf_data->create_symlinks_before_build
	&&
		!pconf_data->remove_symlinks_after_build
	)
		change_create_symlinks_to_baseline(cp, pp, up, minimum);

	/*
	 * Re-run the change file command and the project file command,
	 * if defined, as these usually manipulate information used by
	 * the build tool.
	 */
	string_list_constructor(&wl);
	for (j = 0; ; ++j)
	{
		c_src_data = change_file_nth(cp, j);
		if (!c_src_data)
			break;
		string_list_append(&wl, c_src_data->file_name);
	}
	change_run_change_file_command(cp, &wl, up);
	string_list_destructor(&wl);
	cstate_data->project_file_command_sync = 0;
	change_run_project_file_command(cp, up);

	/*
	 * If the change row (or change file table) changed,
	 * write it out.
	 * Release advisory lock.
	 */
	change_cstate_write(cp);
	commit();
	lock_release();

	/*
	 * Run the develop begin command, because we have probably
	 * removed its side effects.  This is specifically run outside
	 * the locks.
	 */
	change_run_develop_begin_command(cp, up);

	/*
	 * verbose success message
	 */
	change_verbose(cp, 0, i18n("clean complete"));

	change_free(cp);
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
clean()
{
	trace(("clean()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		clean_main();
		break;

	case arglex_token_help:
		clean_help();
		break;

	case arglex_token_list:
		clean_list();
		break;
	}
	trace((/*{*/"}\n"));
}
