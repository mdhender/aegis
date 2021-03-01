/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993 Peter Miller.
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
 * MANIFEST: functions for state transitions to and from being_integrated state
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <arglex2.h>
#include <commit.h>
#include <change.h>
#include <dir.h>
#include <error.h>
#include <file.h>
#include <help.h>
#include <integra2.h>
#include <list.h>
#include <lock.h>
#include <log.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


static void integrate_begin_usage _((void));

static void
integrate_begin_usage()
{
	char		*progname;

	progname = option_get_progname();
	fprintf(stderr, "usage: %s -Integrate_Begin [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Integrate_Begin -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Integrate_Begin -Help\n", progname);
	quit(1);
}


static void integrate_begin_help _((void));

static void
integrate_begin_help()
{
	static char *text[] =
	{
"NAME",
"	%s -Integrate_Begin - begin integrating a change",
"",
"SYNOPSIS",
"	%s -Integrate_Begin <change-number> [ <option>... ]",
"	%s -Integrate_Begin -List [ <option>... ]",
"	%s -Integrate_Begin -Help",
"",
"DESCRIPTION",
"	The %s -Integrate_Begin command is used to begin the",
"	integration of a change into the baseline of a project.",
"",
"	The change will advance from the 'awaiting_integration'",
"	state to the 'being_integrated' state.",
"",
"	A copy of the baseline is created in an integration",
"	directory and the the files of the change are added to",
"	the integration directory; time stamps on the files are",
"	preserved.  The '%s -Change_Directory' command may be",
"	used to locate this directory.	The change will be",
"	assigned to the current user.",
"",
"	The change will be assigned a delta number.  Delta",
"	numbers are incrimented once for each %s",
"	-Intgerate_Begin command for the project.  If an",
"	integration is subsequently aborted with either the %s",
"	-Integrate_Begin_Undo or %s -Integrate_FAIL command,",
"	the delta number will not be re-used.",
"",
"	It is not possible to choose the integration directory,",
"	as there are many constraints upon it, including the fact",
"	that it must be on the same device as the baseline",
"	directory, and that many UNIX implementations don't allow",
"	renaming directories up and down the trees.  The",
"	integration directory will be in the project directory,",
"	and named for the delta number.",
"",
"OPTIONS",
"	The following options are understood:",
"",
"	-Change <number>",
"		This option may be used to specify a particular",
"		change within a project.  When no -Change option is",
"		specified, the AEGIS_CHANGE environment variable is",
"		consulted.  If that does not exist, the user's",
"		$HOME/.aegisrc file is examined for a default change",
"		field (see aeuconf(5) for more information).  If",
"		that does not exist, when the user is only working",
"		on one change within a project, that is the default",
"		change number.  Otherwise, it is an error.",
"",
"	-Help",
"		This option may be used to obtain more",
"		information about how to use the %s program.",
"",
"	-List",
"		This option may be used to obtain a list of",
"		suitable subjects for this command.  The list may",
"		be more general than expected.",
"",
"	-MINimum",
"		This option may be used to cause only the source",
"		files to be copied into the integration",
"		directory.  The default is to copy all files,",
"		unless the change requires the deletion of a",
"		file.",
"",
"	-Project <name>",
"		This option may be used to select the project of",
"		interest.  When no -Project option is specified, the",
"		AEGIS_PROJECT environment variable is consulted.  If",
"		that does not exist, the user's $HOME/.aegisrc file",
"		is examined for a default project field (see",
"		aeuconf(5) for more information).  If that does not",
"		exist, when the user is only working on changes",
"		within a single project, the project name defaults",
"		to that project.  Otherwise, it is an error.",
"",
"	-TERse",
"		This option may be used to cause listings to",
"		produce the bare minimum of information.  It is",
"		usually useful for shell scripts.",
"",
"	-Verbose",
"		This option may be used to cause %s to produce",
"		more output.  By default %s only produces",
"		output on errors.  When used with the -List",
"		option this option causes column headings to be",
"		added.",
"",
"	All options may be abbreviated; the abbreviation is",
"	documented as the upper case letters, all lower case",
"	letters and underscores (_) are optional.  You must use",
"	consecutive sequences of optional letters.",
"",
"	All options are case insensitive, you may type them in",
"	upper case or lower case or a combination of both, case",
"	is not important.",
"",
"	For example: the arguments \"-project, \"-PROJ\" and \"-p\"",
"	are all interpreted to mean the -Project option.  The",
"	argument \"-prj\" will not be understood, because",
"	consecutive optional characters were not supplied.",
"",
"	Options and other command line arguments may be mixed",
"	arbitrarily on the command line, after the function",
"	selectors.",
"",
"	The GNU long option names are understood.  Since all",
"	option names for aegis are long, this means ignoring the",
"	extra leading '-'.  The \"--option=value\" convention is",
"	also understood.",
"",
"RECOMMENDED ALIAS",
"	The recommended alias for this command is",
"	csh%%	alias aeib '%s -ib \\!* -v'",
"	sh$	aeib(){%s -ib $* -v}",
"",
"ERRORS",
"	It is an error if the change is not in the",
"	'awaiting_integration' state.",
"	It is an error if the current user is not an integrator",
"	of the project.",
"	It is an error if there is an integration in progress for",
"	the project.",
"	It is an error if the current user developed the change",
"	and the project is configured to disallow developers to",
"	integrate their own changes (default).",
"	It is an error if the current user reviewed the change",
"	and the project is configured to disallow reviewers to",
"	integrate their such changes (default).",
"",
"EXIT STATUS",
"	The %s command will exit with a status of 1 on any",
"	error.	The %s command will only exit with a status of",
"	0 if there are no errors.",
"",
"COPYRIGHT",
"	%C",
"",
"AUTHOR",
"	%A",
	};

	help(text, SIZEOF(text), integrate_begin_usage);
}


static void integrate_begin_list _((void));

static void
integrate_begin_list()
{
	string_ty	*project_name;

	trace(("integrate_begin_list()\n{\n"/*}*/));
	arglex();
	project_name = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(integrate_begin_usage);
			continue;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				integrate_begin_usage();
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	list_changes_in_state_mask
	(
		project_name,
		1 << cstate_state_awaiting_integration
	);
	if (project_name)
		str_free(project_name);
	trace((/*{*/"}\n"));
}


static void link_tree_callback_minimum _((void *, dir_walk_message_ty,
	string_ty *, struct stat *));

static void
link_tree_callback_minimum(arg, message, path, st)
	void		*arg;
	dir_walk_message_ty message;
	string_ty	*path;
	struct stat	*st;
{
	string_ty	*s1;
	string_ty	*s2;
	change_ty	*cp;
	pstate_src	src;

	trace(("link_tree_callback_minimum(message = %d, path = %08lX, \
st = %08lX)\n{\n"/*}*/, message, path, st));
	cp = (change_ty *)arg;
	assert(cp);
	trace_string(path->str_text);
	s1 = os_below_dir(project_baseline_path_get(cp->pp, 1), path);
	assert(s1);
	trace_string(s1->str_text);
	if (!s1->str_length)
		s2 = str_copy(change_integration_directory_get(cp, 1));
	else
		s2 = str_format("%S/%S", change_integration_directory_get(cp, 1), s1);
	trace_string(s2->str_text);
	switch (message)
	{
	case dir_walk_dir_before:
		if (!s1->str_length)
		{
			assert(!os_exists(s2));
			os_mkdir(s2, 02755);
			undo_rmdir_errok(s2);
		}
		break;

	case dir_walk_file:
		if (st->st_mode & 07000)
		{
			/*
			 * Don't link files with horrible modes.
			 * They shouldn't be source, anyway.
			 */
			if (!project_src_find(cp->pp, s1))
				break;
		}

		/*
		 * don't link it if it's not a source file
		 */
		src = project_src_find(cp->pp, s1);
		if
		(
			!src
		||
			src->deleted_by
		||
			src->about_to_be_created_by
		)
			break;

		/*
		 * make sure the directory is there
		 */
		os_mkdir_between
		(
			change_integration_directory_get(cp, 1),
			s1,
			02755
		);

		/*
		 * don't link file if in change
		 */
		if (change_src_find(cp, s1))
			break;

		/*
		 * link the file and make sure it is a suitable mode
		 */
		os_link(path, s2);
		undo_unlink_errok(s2);
		os_chmod(s2, (st->st_mode | 0444) & ~0222 & ~change_umask(cp));
		break;

	case dir_walk_dir_after:
		break;

	case dir_walk_special:
	case dir_walk_symlink:
		/*
		 * ignore special files
		 *
		 * They could never be source files,
		 * so they must be created by the build.
		 * These ones must always be created at build time, that's all.
		 */
		break;
	}
	str_free(s2);
	str_free(s1);
	trace((/*{*/"}\n"));
}


static void link_tree_callback _((void *, dir_walk_message_ty, string_ty *,
	struct stat *));

static void
link_tree_callback(arg, message, path, st)
	void		*arg;
	dir_walk_message_ty message;
	string_ty	*path;
	struct stat	*st;
{
	string_ty	*s1;
	string_ty	*s2;
	change_ty	*cp;
	pstate_src	src;

	trace(("link_tree_callback(message = %d, path = %08lX, \
st = %08lX)\n{\n"/*}*/, message, path, st));
	cp = (change_ty *)arg;
	assert(cp);
	trace_string(path->str_text);
	s1 = os_below_dir(project_baseline_path_get(cp->pp, 1), path);
	assert(s1);
	trace_string(s1->str_text);
	if (!s1->str_length)
		s2 = str_copy(change_integration_directory_get(cp, 1));
	else
		s2 = str_format("%S/%S", change_integration_directory_get(cp, 1), s1);
	trace_string(s2->str_text);
	switch (message)
	{
	case dir_walk_dir_before:
		assert(!os_exists(s2));
		os_mkdir(s2, 02755);
		undo_rmdir_errok(s2);
		break;

	case dir_walk_file:
		src = project_src_find(cp->pp, s1);
		if (st->st_mode & 07000)
		{
			/*
			 * Don't link files with horrible modes.
			 * They shouldn't be source, anyway.
			 */
			if (!src)
				break;
		}

		/*
		 * don't link change files
		 */
		if (change_src_find(cp, s1))
			break;

		/*
		 * link the file and make sure it is a suitable mode
		 */
		os_link(path, s2);
		undo_unlink_errok(s2);
		if (!src)
			os_chmod(s2, (st->st_mode | 0644) & ~0022 & ~change_umask(cp));
		else
			os_chmod(s2, (st->st_mode | 0444) & ~0222 & ~change_umask(cp));
		break;

	case dir_walk_dir_after:
		break;

	case dir_walk_special:
	case dir_walk_symlink:
		/*
		 * ignore special files
		 *
		 * They could never be source files,
		 * so they must be created by the build.
		 * These ones must always be created at build time, that's all.
		 */
		break;
	}
	str_free(s2);
	str_free(s1);
	trace((/*{*/"}\n"));
}


static void copy_tree_callback_minimum _((void *, dir_walk_message_ty,
	string_ty *, struct stat *));

static void
copy_tree_callback_minimum(arg, message, path, st)
	void		*arg;
	dir_walk_message_ty message;
	string_ty	*path;
	struct stat	*st;
{
	string_ty	*s1;
	string_ty	*s2;
	change_ty	*cp;
	pstate_src	src;
	int		uid;

	trace(("copy_tree_callback(message = %d, path = %08lX, \
st = %08lX)\n{\n"/*}*/, message, path, st));
	cp = (change_ty *)arg;
	assert(cp);
	trace_string(path->str_text);
	s1 = os_below_dir(project_baseline_path_get(cp->pp, 1), path);
	assert(s1);
	trace_string(s1->str_text);
	if (!s1->str_length)
		s2 = str_copy(change_integration_directory_get(cp, 1));
	else
	{
		s2 =
			str_format
			(
				"%S/%S",
				change_integration_directory_get(cp, 1),
				s1
			);
	}
	trace_string(s2->str_text);
	switch (message)
	{
	case dir_walk_dir_before:
		if (!s1->str_length)
		{
			assert(!os_exists(s2));
			os_mkdir(s2, 02755);
			undo_rmdir_errok(s2);
		}
		break;

	case dir_walk_file:
		if (st->st_mode & 07000)
		{
			/*
			 * Don't copy files with horrible modes.
			 * They shouldn't be source, anyway.
			 */
			if (!project_src_find(cp->pp, s1))
				break;
		}

		/*
		 * Don't copy files which don't belong to us.
		 * They shouldn't be source, anyway.
		 */
		os_become_query(&uid, (int *)0, (int *)0);
		src = project_src_find(cp->pp, s1);
		if (st->st_uid != uid && !src)
			break;

		/*
		 * don't copy it if it's not a source file
		 */
		if
		(
			!src
		||
			src->deleted_by
		||
			src->about_to_be_created_by
		)
			break;

		/*
		 * make sure the directory is there
		 */
		os_mkdir_between
		(
			change_integration_directory_get(cp, 1),
			s1,
			02755
		);

		/*
		 * don't copy change files
		 */
		if (change_src_find(cp, s1))
			break;

		/*
		 * copy the file
		 */
		copy_whole_file(path, s2, 0);
		undo_unlink_errok(s2);
		os_chmod(s2, (st->st_mode | 0444) & ~0222 & ~change_umask(cp));
		break;

	case dir_walk_dir_after:
		break;

	case dir_walk_special:
	case dir_walk_symlink:
		/*
		 * ignore special files
		 *
		 * They could never be source files,
		 * so they must be created by the build.
		 * These ones must always be created at build time, that's all.
		 */
		break;
	}
	str_free(s2);
	str_free(s1);
	trace((/*{*/"}\n"));
}


static void copy_tree_callback _((void *, dir_walk_message_ty, string_ty *,
	struct stat *));

static void
copy_tree_callback(arg, message, path, st)
	void		*arg;
	dir_walk_message_ty message;
	string_ty	*path;
	struct stat	*st;
{
	string_ty	*s1;
	string_ty	*s2;
	change_ty	*cp;
	pstate_src	src;
	int		uid;

	trace(("copy_tree_callback(message = %d, path = %08lX, st = %08lX)\n{\n"/*}*/, message, path, st));
	cp = (change_ty *)arg;
	assert(cp);
	trace_string(path->str_text);
	s1 = os_below_dir(project_baseline_path_get(cp->pp, 1), path);
	assert(s1);
	trace_string(s1->str_text);
	if (!s1->str_length)
		s2 = str_copy(change_integration_directory_get(cp, 1));
	else
	{
		s2 =
			str_format
			(
				"%S/%S",
				change_integration_directory_get(cp, 1),
				s1
			);
	}
	trace_string(s2->str_text);
	switch (message)
	{
	case dir_walk_dir_before:
		assert(!os_exists(s2));
		os_mkdir(s2, 02755);
		undo_rmdir_errok(s2);
		break;

	case dir_walk_file:
		/*
		 * Don't copy files with horrible modes.
		 * They shouldn't be source, anyway.
		 */
		src = project_src_find(cp->pp, s1);
		if ((st->st_mode & 07000) && !src)
			break;

		/*
		 * Don't copy files which don't belong to us.
		 * They shouldn't be source, anyway.
		 */
		os_become_query(&uid, (int *)0, (int *)0);
		if (st->st_uid != uid && !src)
			break;

		/*
		 * don't copy change files
		 */
		if (change_src_find(cp, s1))
			break;

		/*
		 * copy the file
		 */
		copy_whole_file(path, s2, 0);
		undo_unlink_errok(s2);
		if (!src)
			os_chmod(s2, (st->st_mode | 0644) & ~0022 & ~change_umask(cp));
		else
			os_chmod(s2, (st->st_mode | 0444) & ~0222 & ~change_umask(cp));
		break;

	case dir_walk_dir_after:
		break;

	case dir_walk_special:
	case dir_walk_symlink:
		/*
		 * ignore special files
		 *
		 * They could never be source files,
		 * so they must be created by the build.
		 * These ones must always be created at build time, that's all.
		 */
		break;
	}
	str_free(s2);
	str_free(s1);
	trace((/*{*/"}\n"));
}


static void integrate_begin_main _((void));

static void
integrate_begin_main()
{
	string_ty	*bl;
	string_ty	*dd;
	string_ty	*id;
	pconf		pconf_data;
	pstate		pstate_data;
	cstate		cstate_data;
	int		j;
	cstate_history	history_data;
	int		minimum;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	int		nolog;
	user_ty		*up;
	int		errs;
	string_ty	*s;

	trace(("integrate_begin_main()\n{\n"/*}*/));
	minimum = 0;
	project_name = 0;
	change_number = 0;
	nolog = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(integrate_begin_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				integrate_begin_usage();
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				fatal("duplicate -Change option");
			change_number = arglex_value.alv_number;
			if (change_number < 1)
				fatal("change %ld out of range", change_number);
			break;

		case arglex_token_minimum:
			if (minimum)
			{
				duplicate:
				fatal("duplicate %s option", arglex_value.alv_string);
			}
			minimum = 1;
			break;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				integrate_begin_usage();
			/* fall through... */

		case arglex_token_string:
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_nolog:
			if (nolog)
				goto duplicate;
			nolog = 1;
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
	 * lock the project, the change and the user
	 */
	project_pstate_lock_prepare(pp);
	change_cstate_lock_prepare(cp);
	user_ustate_lock_prepare(up);
	lock_take();
	pstate_data = project_pstate_get(pp);
	cstate_data = change_cstate_get(cp);
	pconf_data = change_pconf_get(cp);

	/*
	 * make sure they are allowed to
	 */
	if (!project_integrator_query(pp, user_name(up)))
	{
		project_fatal
		(
			pp,
			"user \"%S\" is not an integrator",
			user_name(up)
		);
	}
	if (cstate_data->state != cstate_state_awaiting_integration)
		change_fatal(cp, "not in 'awaiting_integration' state");

	/*
	 * make sure only one integration at a time
	 * for each project
	 */
	if (pstate_data->currently_integrating_change)
	{
		project_fatal
		(
			pp,
			"change %ld: currently active integration",
			pstate_data->currently_integrating_change
		);
	}
	pstate_data->currently_integrating_change = change_number;

	/*
	 * grab a delta number
	 * and advance the project's delta counter
	 */
	cstate_data->delta_number = pstate_data->next_delta_number;
	pstate_data->next_delta_number++;

	/*
	 * Create the integration directory.
	 */
	s =
		str_format
		(
			"%S/delta.%3.3ld",
			project_home_path_get(pp),
			cstate_data->delta_number
		);
	change_integration_directory_set(cp, s);
	str_free(s);

	/*
	 * There will be many files in the baseline in addition
	 * to the sources files.
	 *
	 * If any files are being deleted, only copy the source files
	 * from the baseline to the integration directory.
	 * This way the additional files relating to the removed sources file
	 * are also removed (eg remove a .c file and you need to get rid of the
	 * .o file).  It also shows when dependencies have become out-of-date.
	 *
	 * It is possible to ask for this from the command line, too.
	 */
	for (j = 0; j < cstate_data->src->length; ++j)
	{
		if (cstate_data->src->list[j]->action == file_action_remove)
		{
			minimum = 1;
			break;
		}
	}

	/*
	 * before creating the integration directory,
	 * make sure none of the change files have been tampered with.
	 *
	 * (a) the project owner must have access
	 * (b) the changed file must exist and not have been modified
	 * (c) the difference file must exist and not have been modified
	 */
	dd = change_development_directory_get(cp, 1);
	id = change_integration_directory_get(cp, 1);
	bl = project_baseline_path_get(pp, 1);
	project_become(pp);
	errs = 0;
	for (j = 0; j < cstate_data->src->length; ++j)
	{
		cstate_src	src_data;
		string_ty	*s1;
		string_ty	*s2;

		src_data = cstate_data->src->list[j];
		s1 = str_format("%S/%S", dd, src_data->file_name);
		s2 = str_format("%S,D", s1);
		switch (src_data->action)
		{
		case file_action_remove:
			break;

		case file_action_modify:
		case file_action_create:
			if
			(
				!os_exists(s1)
			||
				os_mtime(s1) != src_data->diff_time
			)
			{
				change_error
				(
					cp,
					"file \"%S\" has been altered",
					s1
				);
				errs++;
			}
			break;
		}
		if (!os_exists(s2) || os_mtime(s2) != src_data->diff_file_time)
		{
			change_error(cp, "file \"%S\" has been altered", s2);
			errs++;
		}
	}
	if (errs)
		quit(1);

	/*
	 * create the integration directory
	 * copy everything from baseline to integration directory
	 * except things from the change
	 * and change owner to the project's owner.
	 */
	if (pconf_data->link_integration_directory)
	{
		change_verbose(cp, "link baseline to integration directory");
		dir_walk
		(
			bl,
			(
				minimum
			?
				link_tree_callback_minimum
			:
				link_tree_callback
			),
			cp
		);
	}
	else
	{
		change_verbose(cp, "copy baseline to integration directory");
		dir_walk
		(
			bl,
			(
				minimum
			?
				copy_tree_callback_minimum
			:
				copy_tree_callback
			),
			cp
		);
	}

	/*
	 * apply the changes to the integration directory
	 */
	change_verbose(cp, "apply change to integration directory");
	for (j = 0; j < cstate_data->src->length; ++j)
	{
		cstate_src	src_data;
		string_ty	*s1;
		string_ty	*s2;

		src_data = cstate_data->src->list[j];
		s1 = str_format("%S/%S", dd, src_data->file_name);
		s2 = str_format("%S/%S", id, src_data->file_name);
		if (os_exists(s2))
		{
			/*
			 * this is defensive,
			 * and should never need to be executed
			 */
			os_unlink(s2);
		}
		switch (src_data->action)
		{
		case file_action_remove:
			break;

		case file_action_modify:
		case file_action_create:
			/*
			 * New files do not exist in the baseline,
			 * and old files may not be copied under -MINimum,
			 * so we may need to create directories.
			 */
			os_mkdir_between(id, src_data->file_name, 02755);
			copy_whole_file(s1, s2, 0);
			os_chmod(s2, 0444 & ~change_umask(cp));
			break;
		}
	}
	project_become_undo();

	/*
	 * add the change to the user's list
	 */
	user_own_add(up, project_name_get(pp), change_number);
	cstate_data->state = cstate_state_being_integrated;
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_integrate_begin;

	/*
	 * write the data out
	 * and release the locks
	 */
	project_pstate_write(pp);
	change_cstate_write(cp);
	user_ustate_write(up);
	commit();
	lock_release();

	/*
	 * run the integrate begin command
	 */
	if (pconf_data->integrate_begin_command)
	{
		string_ty	*the_command;

		if (!nolog)
			log_open(change_logfile_get(cp), up);
		sub_var_set("1", "${project}");
		sub_var_set("2", "${change}");
		sub_var_set("3", "${version}");
		the_command = pconf_data->integrate_begin_command;
		the_command = substitute(cp, the_command);
		os_execute
		(
			the_command,
			OS_EXEC_FLAG_NO_INPUT + OS_EXEC_FLAG_ERROK,
			change_integration_directory_get(cp, 1)
		);
		str_free(the_command);
	}

	/*
	 * verbose success message
	 */
	change_verbose(cp, "integration has begun");
	change_free(cp);
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
integrate_begin()
{
	trace(("integrate_begin()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		integrate_begin_main();
		break;

	case arglex_token_help:
		integrate_begin_help();
		break;

	case arglex_token_list:
		integrate_begin_list();
		break;
	}
	trace((/*{*/"}\n"));
}


static void integrate_begin_undo_usage _((void));

static void
integrate_begin_undo_usage()
{
	char		*progname;

	progname = option_get_progname();
	fprintf(stderr, "usage: %s -Integrate_Begin_Undo [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Integrate_Begin_Undo -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Integrate_Begin_Undo -Help\n", progname);
	quit(1);
}


static void integrate_begin_undo_help _((void));

static void
integrate_begin_undo_help()
{
	static char *text[] =
	{
"NAME",
"	%s -Integrate_Begin_Undo - reverse the aeib command",
"",
"SYNOPSIS",
"	%s -Integrate_Begin_Undo [ <option>... ]",
"	%s -Integrate_Begin_Undo -List [ <option>... ]",
"	%s -Integrate_Begin_Undo -Help",
"",
"DESCRIPTION",
"	The %s -Integrate_Begin_Undo command is used to",
"	reverse the actions of the '%s -Integrate_Begin'",
"	command.",
"",
"	Successful execution of this command will move the change",
"	from the 'being_integrated' state to the",
"	'awaiting_integration' state.  The integration directory",
"	will be deleted.  The change will cease to be assigned to",
"	the current user.",
"",
"OPTIONS",
"	The following options are understood:",
"",
"	-Change <number>",
"		This option may be used to specify a particular",
"		change within a project.  When no -Change option is",
"		specified, the AEGIS_CHANGE environment variable is",
"		consulted.  If that does not exist, the user's",
"		$HOME/.aegisrc file is examined for a default change",
"		field (see aeuconf(5) for more information).  If",
"		that does not exist, when the user is only working",
"		on one change within a project, that is the default",
"		change number.  Otherwise, it is an error.",
"",
"	-Help",
"		This option may be used to obtain more",
"		information about how to use the %s program.",
"",
"	-Keep",
"		This option may be used to retain files and/or",
"		directories usually deleted by the command.",
"",
"	-List",
"		This option may be used to obtain a list of",
"		suitable subjects for this command.  The list may",
"		be more general than expected.",
"",
"	-Project <name>",
"		This option may be used to select the project of",
"		interest.  When no -Project option is specified, the",
"		AEGIS_PROJECT environment variable is consulted.  If",
"		that does not exist, the user's $HOME/.aegisrc file",
"		is examined for a default project field (see",
"		aeuconf(5) for more information).  If that does not",
"		exist, when the user is only working on changes",
"		within a single project, the project name defaults",
"		to that project.  Otherwise, it is an error.",
"",
"	-TERse",
"		This option may be used to cause listings to",
"		produce the bare minimum of information.  It is",
"		usually useful for shell scripts.",
"",
"	-Verbose",
"		This option may be used to cause %s to produce",
"		more output.  By default %s only produces",
"		output on errors.  When used with the -List",
"		option this option causes column headings to be",
"		added.",
"",
"	All options may be abbreviated; the abbreviation is",
"	documented as the upper case letters, all lower case",
"	letters and underscores (_) are optional.  You must use",
"	consecutive sequences of optional letters.",
"",
"	All options are case insensitive, you may type them in",
"	upper case or lower case or a combination of both, case",
"	is not important.",
"",
"	For example: the arguments \"-project, \"-PROJ\" and \"-p\"",
"	are all interpreted to mean the -Project option.  The",
"	argument \"-prj\" will not be understood, because",
"	consecutive optional characters were not supplied.",
"",
"	Options and other command line arguments may be mixed",
"	arbitrarily on the command line, after the function",
"	selectors.",
"",
"	The GNU long option names are understood.  Since all",
"	option names for aegis are long, this means ignoring the",
"	extra leading '-'.  The \"--option=value\" convention is",
"	also understood.",
"",
"RECOMMENDED ALIAS",
"	The recommended alias for this command is",
"	csh%%	alias aeibu '%s -ibu \\!* -v'",
"	sh$	aeibu(){%s -ibu $* -v}",
"",
"ERRORS",
"	It is an error if the change is not in the",
"	'being_integrated' state.",
"	It is an error if the change is not assigned to the",
"	current user.",
"",
"EXIT STATUS",
"	The %s command will exit with a status of 1 on any",
"	error.	The %s command will only exit with a status of",
"	0 if there are no errors.",
"",
"COPYRIGHT",
"	%C",
"",
"AUTHOR",
"	%A",
	};

	help(text, SIZEOF(text), integrate_begin_undo_usage);
}


static void integrating_list _((void (*)(void)));

static void
integrating_list(usage)
	void		(*usage)_((void));
{
	string_ty	*project_name;

	trace(("integrating_list()\n{\n"/*}*/));
	arglex();
	project_name = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(usage);
			continue;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				usage();
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	list_changes_in_state_mask
	(
		project_name,
		1 << cstate_state_being_integrated
	);
	if (project_name)
		str_free(project_name);
	trace((/*{*/"}\n"));
}


static void integrate_begin_undo_main _((void));

static void
integrate_begin_undo_main()
{
	cstate		cstate_data;
	pstate		pstate_data;
	cstate_history	history_data;
	string_ty	*dir;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	user_ty		*up;

	trace(("integrate_begin_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(integrate_begin_undo_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				integrate_begin_undo_usage();
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
				integrate_begin_undo_usage();
			/* fall through... */

		case arglex_token_string:
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
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
	 * lock the change for writing
	 */
	project_pstate_lock_prepare(pp);
	change_cstate_lock_prepare(cp);
	user_ustate_lock_prepare(up);
	lock_take();
	cstate_data = change_cstate_get(cp);
	pstate_data = project_pstate_get(pp);

	/*
	 * it is an error if the change is not in the 'being_integrated' state.
	 */
	if (cstate_data->state != cstate_state_being_integrated)
		change_fatal(cp, "not in 'being_integrated' state");
	if (!str_equal(change_integrator_name(cp), user_name(up)))
	{
		change_fatal
		(
			cp,
			"user \"%S\" is not the integrator",
			user_name(up)
		);
	}

	/*
	 * Change the state.
	 * Add to the change's history.
	 */
	cstate_data->state = cstate_state_awaiting_integration;
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_integrate_begin_undo;

	/*
	 * remove it from the user's change list
	 */
	user_own_remove(up, project_name_get(pp), change_number);

	/*
	 * Note that the project has no current integration
	 */
	pstate_data->currently_integrating_change = 0;
	dir = str_copy(change_integration_directory_get(cp, 1));
	change_integration_directory_clear(cp);
	cstate_data->build_time = 0;
	cstate_data->delta_number = 0;

	/*
	 * Complain if they are in the integration directory,
	 * because the rmdir at the end can't then run to completion.
	 */
	os_become_orig();
	if (os_below_dir(dir, os_curdir()))
		change_fatal(cp, "please leave the integration directory");
	os_become_undo();

	/*
	 * write out the data and release the locks
	 */
	change_cstate_write(cp);
	user_ustate_write(up);
	project_pstate_write(pp);
	user_become(up);
	commit_rmdir_tree_errok(dir);
	user_become_undo();
	str_free(dir);
	commit();
	lock_release();

	/*
	 * verbose success message
	 */
	change_verbose(cp, "integrate begin undo");
	change_free(cp);
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
integrate_begin_undo()
{
	trace(("integrate_begin_undo()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		integrate_begin_undo_main();
		break;

	case arglex_token_help:
		integrate_begin_undo_help();
		break;

	case arglex_token_list:
		integrating_list(integrate_begin_undo_usage);
		break;
	}
	trace((/*{*/"}\n"));
}


static void integrate_fail_usage _((void));

static void
integrate_fail_usage()
{
	char		*progname;

	progname = option_get_progname();
	fprintf(stderr, "usage: %s -Integrate_FAIL <commentfile> [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Integrate_FAIL -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Integrate_FAIL -Help\n", progname);
	quit(1);
}


static void integrate_fail_help _((void));

static void
integrate_fail_help()
{
	static char *text[] =
	{
"NAME",
"	%s -Integrate_FAIL - fail a change integration",
"",
"SYNOPSIS",
"	%s -Integrate_FAIL [ <option>... ]",
"	%s -Integrate_FAIL -List [ <option>... ]",
"	%s -Integrate_FAIL -Help",
"",
"DESCRIPTION",
"	The %s -Integrate_FAIL command is used to inform %s",
"	that a change has failed integration.",
"",
"	The change will be returned from the 'being_integrated'",
"	state to the 'being_developed' state.  The change will",
"	cease to be assigned to the current user, and will be",
"	reassigned to the originating developer.  The integration",
"	directory will be deleted.",
"",
"	The reviewer and the developer will be notified by mail.",
"	See the integrate_fail_notify_command in aepconf(5) for",
"	more information.",
"",
"OPTIONS",
"	The following options are understood:",
"",
"	-Change <number>",
"		This option may be used to specify a particular",
"		change within a project.  When no -Change option is",
"		specified, the AEGIS_CHANGE environment variable is",
"		consulted.  If that does not exist, the user's",
"		$HOME/.aegisrc file is examined for a default change",
"		field (see aeuconf(5) for more information).  If",
"		that does not exist, when the user is only working",
"		on one change within a project, that is the default",
"		change number.  Otherwise, it is an error.",
"",
"	-Help",
"		This option may be used to obtain more",
"		information about how to use the %s program.",
"",
"	-Keep",
"		This option may be used to retain files and/or",
"		directories usually deleted by the command.",
"",
"	-List",
"		This option may be used to obtain a list of",
"		suitable subjects for this command.  The list may",
"		be more general than expected.",
"",
"	-Project <name>",
"		This option may be used to select the project of",
"		interest.  When no -Project option is specified, the",
"		AEGIS_PROJECT environment variable is consulted.  If",
"		that does not exist, the user's $HOME/.aegisrc file",
"		is examined for a default project field (see",
"		aeuconf(5) for more information).  If that does not",
"		exist, when the user is only working on changes",
"		within a single project, the project name defaults",
"		to that project.  Otherwise, it is an error.",
"",
"	-TERse",
"		This option may be used to cause listings to",
"		produce the bare minimum of information.  It is",
"		usually useful for shell scripts.",
"",
"	-Verbose",
"		This option may be used to cause %s to produce",
"		more output.  By default %s only produces",
"		output on errors.  When used with the -List",
"		option this option causes column headings to be",
"		added.",
"",
"	All options may be abbreviated; the abbreviation is",
"	documented as the upper case letters, all lower case",
"	letters and underscores (_) are optional.  You must use",
"	consecutive sequences of optional letters.",
"",
"	All options are case insensitive, you may type them in",
"	upper case or lower case or a combination of both, case",
"	is not important.",
"",
"	For example: the arguments \"-project, \"-PROJ\" and \"-p\"",
"	are all interpreted to mean the -Project option.  The",
"	argument \"-prj\" will not be understood, because",
"	consecutive optional characters were not supplied.",
"",
"	Options and other command line arguments may be mixed",
"	arbitrarily on the command line, after the function",
"	selectors.",
"",
"	The GNU long option names are understood.  Since all",
"	option names for aegis are long, this means ignoring the",
"	extra leading '-'.  The \"--option=value\" convention is",
"	also understood.",
"",
"RECOMMENDED ALIAS",
"	The recommended alias for this command is",
"	csh%%	alias aeif '%s -if \\!* -v'",
"	sh$	aeif(){%s -if $* -v}",
"",
"ERRORS",
"	It is an error if the change is not in the",
"	'being_integrated' state.",
"	It is an error if the change is not assigned to the",
"	current user.",
"",
"EXIT STATUS",
"	The %s command will exit with a status of 1 on any",
"	error.	The %s command will only exit with a status of",
"	0 if there are no errors.",
"",
"COPYRIGHT",
"	%C",
"",
"AUTHOR",
"	%A",
	};

	help(text, SIZEOF(text), integrate_fail_usage);
}


static void repair_diff_time _((change_ty *, string_ty *));

static void
repair_diff_time(cp, path)
	change_ty	*cp;
	string_ty	*path;
{
	string_ty	*s;
	cstate_src	src_data;
	string_ty	*s2;

	s = os_below_dir(change_development_directory_get(cp, 1), path);
	src_data = change_src_find(cp, s);
	if (src_data)
		src_data->diff_time = os_mtime(path);
	else
	{
		if
		(
			s->str_length > 2
		&&
			!strcmp(s->str_text + s->str_length - 2, ",D")
		)
		{
			s2 = str_n_from_c(s->str_text, s->str_length - 2);
			src_data = change_src_find(cp, s2);
			if (src_data)
				src_data->diff_file_time = os_mtime(path);
			str_free(s2);
		}
	}
	str_free(s);
}


static void if_func _((void *, dir_walk_message_ty, string_ty *, struct stat *));

static void
if_func(arg, message, path, st)
	void		*arg;
	dir_walk_message_ty message;
	string_ty	*path;
	struct stat	*st;
{
	change_ty	*cp;
	int		uid;

	trace(("if_func(message = %d, path = \"%s\", st = %08lX)\n{\n"/*}*/,
		message, path->str_text, st));
	cp = (change_ty *)arg;
	switch (message)
	{
	case dir_walk_dir_before:
	case dir_walk_file:
		os_become_query(&uid, (int *)0, (int *)0);
		if (st->st_uid == uid)
		{
			os_chmod
			(
				path,
				(st->st_mode | 0644) & ~change_umask(cp)
			);
			undo_chmod(path, st->st_mode);
			repair_diff_time(cp, path);
		}
		break;

	case dir_walk_special:
	case dir_walk_symlink:
	case dir_walk_dir_after:
		break;
	}
	trace((/*{*/"}\n"));
}


static void integrate_fail_main _((void));

static void
integrate_fail_main()
{
	string_ty	*dd;
	cstate		cstate_data;
	pstate		pstate_data;
	cstate_history	history_data;
	string_ty	*comment = 0;
	string_ty	*rev_name;
	string_ty	*int_name;
	string_ty	*dir;
	int		j;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	user_ty		*up;
	user_ty		*devup;

	trace(("integrate_fail_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(integrate_fail_usage);
			continue;

		case arglex_token_string:
			if (comment)
				fatal("too many files named");
			os_become_orig();
			comment = read_whole_file(arglex_value.alv_string);
			os_become_undo();
			break;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				integrate_fail_usage();
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
				integrate_fail_usage();
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	if (!comment)
		fatal("no comment file supplied");

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
	 * lock the change for writing
	 */
	project_pstate_lock_prepare(pp);
	change_cstate_lock_prepare(cp);
	lock_prepare_ustate_all(); /* we don't know which users until later */
	lock_take();
	cstate_data = change_cstate_get(cp);
	pstate_data = project_pstate_get(pp);

	/*
	 * it is an error if the change is not in the 'being_integrated' state.
	 */
	if (cstate_data->state != cstate_state_being_integrated)
		change_fatal(cp, "not in 'being_integrated' state");
	if (!str_equal(change_integrator_name(cp), user_name(up)))
	{
		change_fatal
		(
			cp,
			"user \"%S\" not the integrator",
			user_name(up)
		);
	}

	/*
	 * Change the state.
	 * Add to the change's history.
	 */
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_integrate_fail;
	history_data->why = comment;
	cstate_data->build_time = 0;
	cstate_data->test_time = 0;
	cstate_data->test_baseline_time = 0;
	rev_name = change_reviewer_name(cp);
	int_name = change_integrator_name(cp);
	cstate_data->delta_number = 0;
	dir = str_copy(change_integration_directory_get(cp, 1));
	change_integration_directory_clear(cp);
	cstate_data->state = cstate_state_being_developed;

	/*
	 * Complain if they are in the integration directory,
	 * because the rmdir at the end can't then run to completion.
	 */
	os_become_orig();
	if (os_below_dir(dir, os_curdir()))
		change_fatal(cp, "please leave the integration directory");
	os_become_undo();

	/*
	 * note that the project has no current integration
	 */
	pstate_data->currently_integrating_change = 0;

	/*
	 * Remove it from the integrator's change list, and
	 * add it back into the developer's change list.
	 */
	user_own_remove(up, project_name_get(pp), change_number);
	devup = user_symbolic(pp, change_developer_name(cp));
	user_own_add(devup, project_name_get(pp), change_number);

	/*
	 * make the change files writable again
	 */
	change_verbose(cp, "make the development directory writable again");
	dd = change_development_directory_get(cp, 1);
	user_become(devup);
	dir_walk(dd, if_func, cp);
	user_become_undo();

	/*
	 * go through the files in the change and unlock them
	 * in the baseline
	 */
	for (j = 0; j < cstate_data->src->length; ++j)
	{
		cstate_src	c_src_data;
		pstate_src	p_src_data;

		c_src_data = cstate_data->src->list[j];
		p_src_data = project_src_find(pp, c_src_data->file_name);
		if (!p_src_data)
			continue;
		p_src_data->locked_by = 0;

		/*
		 * Remove the file if it is about_to_be_created
		 * by the change we are rescinding.
		 */
		if (p_src_data->about_to_be_created_by)
		{
			assert(p_src_data->about_to_be_created_by == change_number);
			project_src_remove(pp, c_src_data->file_name);
		}
	}

	/*
	 * remove the integration directory
	 */
	change_verbose(cp, "remove the integration directory");
	project_become(pp);
	commit_rmdir_tree_errok(dir);
	project_become_undo();

	/*
	 * write out the data and release the locks
	 */
	change_cstate_write(cp);
	user_ustate_write(up);
	user_ustate_write(devup);
	project_pstate_write(pp);
	str_free(dir);
	commit();
	lock_release();

	/*
	 * run the notify command
	 */
	change_run_integrate_fail_notify_command(cp);

	/*
	 * verbose success message
	 */
	change_verbose(cp, "failed integration, returned to developer");
	change_free(cp);
	project_free(pp);
	user_free(up);
	user_free(devup);
	trace((/*{*/"}\n"));
}


void
integrate_fail()
{
	trace(("integrate_fail()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		integrate_fail_main();
		break;

	case arglex_token_help:
		integrate_fail_help();
		break;

	case arglex_token_list:
		integrating_list(integrate_fail_usage);
		break;
	}
	trace((/*{*/"}\n"));
}


static void integrate_pass_usage _((void));

static void
integrate_pass_usage()
{
	char		*progname;

	progname = option_get_progname();
	fprintf(stderr, "usage: %s -Integrate_PASS [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Integrate_PASS -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Integrate_PASS -Help\n", progname);
	quit(1);
}


static void integrate_pass_help _((void));

static void
integrate_pass_help()
{
	static char *text[] =
	{
"NAME",
"	%s -Integrate_PASS - pass a change integration",
"",
"SYNOPSIS",
"	%s -Integrate_PASS [ <option>... ]",
"	%s -Integrate_PASS -List [ <option>... ]",
"	%s -Integrate_PASS -Help",
"",
"DESCRIPTION",
"	The %s -Integrate_PASS command is used to notify %s",
"	that a change has passed integration.",
"",
"	The change is advanced from the 'being_integrated' state",
"	to the 'completed' state.  The integration directory is",
"	renamed as the baseline directory, and the baseline",
"	directory is deleted.  The change is no longer assigned",
"	to the current user.",
"",
"	While there is a build in progress for any change in a",
"	project, an integrate pass for the project will wait until",
"	all the builds are completed before starting.  This is to",
"	ensure that the baseline is consistent for the entire build.",
"	Similarly, while an integrate pass is in progress for a",
"	project, any builds will wait until it is completed before",
"	starting.",
"",
"OPTIONS",
"	The following options are understood:",
"",
"	-Change <number>",
"		This option may be used to specify a particular",
"		change within a project.  When no -Change option is",
"		specified, the AEGIS_CHANGE environment variable is",
"		consulted.  If that does not exist, the user's",
"		$HOME/.aegisrc file is examined for a default change",
"		field (see aeuconf(5) for more information).  If",
"		that does not exist, when the user is only working",
"		on one change within a project, that is the default",
"		change number.  Otherwise, it is an error.",
"",
"	-Help",
"		This option may be used to obtain more",
"		information about how to use the %s program.",
"",
"	-List",
"		This option may be used to obtain a list of",
"		suitable subjects for this command.  The list may",
"		be more general than expected.",
"",
"	-Project <name>",
"		This option may be used to select the project of",
"		interest.  When no -Project option is specified, the",
"		AEGIS_PROJECT environment variable is consulted.  If",
"		that does not exist, the user's $HOME/.aegisrc file",
"		is examined for a default project field (see",
"		aeuconf(5) for more information).  If that does not",
"		exist, when the user is only working on changes",
"		within a single project, the project name defaults",
"		to that project.  Otherwise, it is an error.",
"",
"	-TERse",
"		This option may be used to cause listings to",
"		produce the bare minimum of information.  It is",
"		usually useful for shell scripts.",
"",
"	-Verbose",
"		This option may be used to cause %s to produce",
"		more output.  By default %s only produces",
"		output on errors.  When used with the -List",
"		option this option causes column headings to be",
"		added.",
"",
"	All options may be abbreviated; the abbreviation is",
"	documented as the upper case letters, all lower case",
"	letters and underscores (_) are optional.  You must use",
"	consecutive sequences of optional letters.",
"",
"	All options are case insensitive, you may type them in",
"	upper case or lower case or a combination of both, case",
"	is not important.",
"",
"	For example: the arguments \"-project, \"-PROJ\" and \"-p\"",
"	are all interpreted to mean the -Project option.  The",
"	argument \"-prj\" will not be understood, because",
"	consecutive optional characters were not supplied.",
"",
"	Options and other command line arguments may be mixed",
"	arbitrarily on the command line, after the function",
"	selectors.",
"",
"	The GNU long option names are understood.  Since all",
"	option names for aegis are long, this means ignoring the",
"	extra leading '-'.  The \"--option=value\" convention is",
"	also understood.",
"",
"RECOMMENDED ALIAS",
"	The recommended alias for this command is",
"	csh%%	alias aeip '%s -ip \\!* -v'",
"	sh$	aeip(){%s -ip $* -v}",
"",
"ERRORS",
"	It is an error if the change is not assigned to the",
"	current user.",
"	It is an error if The change is not in the",
"	'being_integrated' state.",
"	It is an error if there has been no successful '%s",
"	-Build' command for the integration.",
"	It is an error if there has been no successful '%s",
"	-Test' command for the integration.",
"	It is an error if there has been no successful '%s",
"	-Test -BaseLine' command for the integration.",
"",
"EXIT STATUS",
"	The %s command will exit with a status of 1 on any",
"	error.	The %s command will only exit with a status of",
"	0 if there are no errors.",
"",
"COPYRIGHT",
"	%C",
"",
"AUTHOR",
"	%A",
	};

	help(text, SIZEOF(text), integrate_pass_usage);
}


static void integrate_pass_main _((void));

static void
integrate_pass_main()
{
	string_ty	*hp;
	string_ty	*id;
	string_ty	*cwd;
	pstate		pstate_data;
	cstate		cstate_data;
	string_ty	*old_baseline;
	string_ty	*new_baseline;
	string_ty	*dev_dir;
	string_ty	*int_name;
	string_ty	*rev_name;
	string_ty	*dev_name;
	cstate_history	history_data;
	pstate_history	phistory_data;
	int		j;
	int		ncmds;
	pconf		pconf_data;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	int		nolog;
	user_ty		*up;
	user_ty		*devup;

	trace(("integrate_pass_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	nolog = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(integrate_pass_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				integrate_pass_usage();
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
				integrate_pass_usage();
			/* fall through... */
	
		case arglex_token_string:
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_nolog:
			if (nolog)
				fatal("duplicate %s option", arglex_value.alv_string);
			nolog = 1;
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
	 * Take an advisory write lock on the appropriate row of the change
	 * table.  Take an advisory write lock on the appropriate row of the
	 * user table.  Block until can get both simultaneously.
	 */
	project_pstate_lock_prepare(pp);
	change_cstate_lock_prepare(cp);
	user_ustate_lock_prepare(up);
	project_build_write_lock_prepare(pp);
	lock_take();
	cstate_data = change_cstate_get(cp);
	pstate_data = project_pstate_get(pp);
	pconf_data = change_pconf_get(cp);

	/*
	 * make sure they aren't in a nuisance place
	 */
	os_become_orig();
	cwd = os_curdir();
	os_become_undo();
	if (os_below_dir(change_development_directory_get(cp, 1), cwd))
		change_fatal(cp, "please leave development directory");
	id = change_integration_directory_get(cp, 1);
	if (os_below_dir(id, cwd))
		change_fatal(cp, "please leave integration directory");
	if (os_below_dir(project_baseline_path_get(pp, 1), cwd))
		project_fatal(pp, "please leave baseline directory");

	/*
	 * It is an error if the change is not in the being_integrated state.
	 * It is an error if the change is not assigned to the current user.
	 * It is an error if the change has no current build.
	 * It is an error if the change has no current test pass.
	 * It is an error if the change has no current baseline test pass.
	 */
	if (cstate_data->state != cstate_state_being_integrated)
		change_fatal(cp, "not in 'being_integrated' state");
	if (!str_equal(change_integrator_name(cp), user_name(up)))
	{
		change_fatal
		(
			cp,
			"user \"%S\" is not the integrator",
			user_name(up)
		);
	}
	if (!cstate_data->build_time)
	{
		change_fatal
		(
			cp,
			"no current '%s -Build' registration",
			option_get_progname()
		);
	}
	if (!cstate_data->test_exempt && !cstate_data->test_time)
	{
		change_fatal
		(
			cp,
			"no current '%s -Test' registration",
			option_get_progname()
		);
	}
	if
	(
		!cstate_data->test_baseline_exempt
	&&
		!cstate_data->test_baseline_time
	)
	{
		change_fatal
		(
			cp,
			"no current '%s -Test -BaseLine' registration",
			option_get_progname()
		);
	}

	/*
	 * add to history for state change
	 */
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_integrate_pass;
	phistory_data = project_history_new(pp);
	phistory_data->delta_number = cstate_data->delta_number;
	phistory_data->change_number = change_number;

	/*
	 * note that the project has no current integration
	 */
	pstate_data->currently_integrating_change = 0;

	/*
	 * Clear the default-change field of the user row.
	 * Remove the change from the list of assigned changes in the user
	 * change table (in the user row).
	 */
	user_own_remove(up, project_name_get(pp), change_number);

	/*
	 * Update the edit history of each changed file.
	 * Update the edit number of each file.
	 * Unlock each file.
	 */
	if (!nolog)
	{
		user_ty	*pup;

		pup = project_user(pp);
		log_open(change_logfile_get(cp), pup);
		user_free(pup);
	}
	ncmds = 0;
	hp = project_history_path_get(pp);
	trace(("mark\n"));
	for (j = 0; j < cstate_data->src->length; ++j)
	{
		cstate_src	c_src_data;
		pstate_src	p_src_data;
		string_ty	*src_file;
		string_ty	*dst_file;

		c_src_data = cstate_data->src->list[j];
		c_src_data->diff_time = 0;
		c_src_data->diff_file_time = 0;

		p_src_data = project_src_find(pp, c_src_data->file_name);
		if (!p_src_data)
			project_fatal(pp, "corrupted src field");
		p_src_data->locked_by = 0;
		switch (c_src_data->action)
		{
		case file_action_create:
			/*
			 * because history is never thrown away,
			 * we could be reusing an existing history file
			 */
			if (p_src_data->edit_number)
			{
				assert(p_src_data->deleted_by);
				p_src_data->deleted_by = 0;
				goto reusing_an_old_file;
			}

			/*
			 * it exists, now
			 */
			trace(("create\n"));
			p_src_data->about_to_be_created_by = 0;

			/*
			 * create the history
			 */
			change_run_history_create_command
			(
				cp,
				c_src_data->file_name
			);
			p_src_data->edit_number =
				change_run_history_query_command
				(
					cp,
					c_src_data->file_name
				);
			c_src_data->edit_number =
				str_copy(p_src_data->edit_number);
			break;

		case file_action_modify:
			reusing_an_old_file:
			trace(("modify\n"));
			assert(str_equal(p_src_data->edit_number,
				c_src_data->edit_number));

			/*
			 * prepare for the history commands
			 */
			src_file =
				str_format("%S/%S", id, c_src_data->file_name);
			dst_file =
				str_format("%S/%S", hp, c_src_data->file_name);

			/*
			 * update the history
			 */
			change_run_history_put_command
			(
				cp,
				c_src_data->file_name
			);
			p_src_data->edit_number =
				change_run_history_query_command
				(
					cp,
					c_src_data->file_name
				);
			c_src_data->edit_number =
				str_copy(p_src_data->edit_number);
			break;

		case file_action_remove:
			/*
			 * don't need to do much for deletions.
			 * Note that we never throw the history file away.
			 */
			trace(("remove\n"));
			assert(str_equal(p_src_data->edit_number,
				c_src_data->edit_number));
			p_src_data->deleted_by = change_number;
			break;
		}
	}

	/*
	 * Advance the change to the 'completed' state.
	 * Clear the build-time field.
	 * Clear the test-time field.
	 * Clear the test-baseline-time field.
	 */
	cstate_data->state = cstate_state_completed;
	cstate_data->build_time = 0;
	cstate_data->test_time = 0;
	cstate_data->test_baseline_time = 0;
	dev_dir = str_copy(change_development_directory_get(cp, 1));
	change_development_directory_clear(cp);
	new_baseline = str_copy(change_integration_directory_get(cp, 1));
	change_integration_directory_clear(cp);
	int_name = change_integrator_name(cp);
	rev_name = change_reviewer_name(cp);
	dev_name = change_developer_name(cp);

	/*
	 * we are going to keep the delta,
	 * and throw away the old baseline
	 */
	old_baseline =
		str_format
		(
			"%S.D%3.3ld",
			project_baseline_path_get(pp, 1),
			cstate_data->delta_number - 1
		);
	new_baseline = change_integration_directory_get(cp, 1);
	project_become(pp);
	commit_rename(project_baseline_path_get(pp, 1), old_baseline);
	commit_rename(new_baseline, project_baseline_path_get(pp, 1));
	commit_rmdir_tree_errok(old_baseline);
	project_become_undo();
	str_free(old_baseline);

	/*
	 * throw away the development directory
	 */
	change_verbose(cp, "discard old directories");
	devup = user_symbolic(pp, dev_name);
	user_become(devup);
	commit_rmdir_tree_errok(dev_dir);
	user_become_undo();
	str_free(dev_dir);
	user_free(devup);

	/*
	 * Write the change table row.
	 * Write the user table row.
	 * Release advisory locks.
	 */
	change_cstate_write(cp);
	user_ustate_write(up);
	project_pstate_write(pp);
	commit();
	lock_release();

	/*
	 * run the notify command
	 */
	change_run_integrate_pass_notify_command(cp);

	/*
	 * verbose success message
	 */
	change_verbose(cp, "integrate pass");
	change_free(cp);
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
integrate_pass()
{
	trace(("integrate_pass()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		integrate_pass_main();
		break;

	case arglex_token_help:
		integrate_pass_help();
		break;

	case arglex_token_list:
		integrating_list(integrate_pass_usage);
		break;
	}
	trace((/*{*/"}\n"));
}
