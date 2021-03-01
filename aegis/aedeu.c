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
 * MANIFEST: functions to implement develop end undo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <aedeu.h>
#include <ael.h>
#include <arglex2.h>
#include <change.h>
#include <col.h>
#include <commit.h>
#include <common.h>
#include <dir.h>
#include <error.h>
#include <help.h>
#include <lock.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


static void develop_end_undo_usage _((void));

static void
develop_end_undo_usage()
{
	char		*progname;

	progname = option_progname_get();
	fprintf(stderr, "usage: %s -Undo_Develop_End [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Undo_Develop_End -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Undo_Develop_End -Help\n", progname);
	quit(1);
}


static void develop_end_undo_help _((void));

static void
develop_end_undo_help()
{
	static char *text[] =
	{
"NAME",
"	%s -Develop_End_Undo - recall a change for further",
"	development",
"",
"SYNOPSIS",
"	%s -Develop_End_Undo <change-number> [ <option>... ]",
"	%s -Develop_End_Undo -List [ <option>... ]",
"	%s -Develop_End_Undo -Help",
"",
"DESCRIPTION",
"	The %s -Develop_End_Undo command is used to recall a",
"	change for further development.",
"",
"	Successful execution of this command returns the change",
"	to the 'being_developed' state.",
"",
"	The files are changed back to being owned by the current",
"	user, and cease to be read-only.",
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
"	All options are case insensitive.  Options may be",
"	abbreviated; the abbreviation is the upper case letters.",
"	Options and other command line arguments may be mixed",
"	arbitrarily on the command line.",
"",
"RECOMMENDED ALIAS",
"	The recommended alias for this command is",
"	csh%%	alias aedeu '%s -deu \\!* -v'",
"	sh$	aedeu(){%s -deu $* -v}",
"",
"ERRORS",
"	It is an error if the change is not in one of the",
"	'being_reviewed' or 'awaiting_integration' states.",
"	It is an error if the change was not developed by the",
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

	help(text, SIZEOF(text), develop_end_undo_usage);
}


static void develop_end_undo_list _((void));

static void
develop_end_undo_list()
{
	string_ty	*project_name;

	trace(("develop_end_undo_list()\n{\n"/*}*/));
	project_name = 0;
	arglex();
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(develop_end_undo_usage);
			continue;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				develop_end_undo_usage();
			/* fall through... */
		
		case arglex_token_string:
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
		(
			(1 << cstate_state_being_reviewed)
		|
			(1 << cstate_state_awaiting_integration)
		)
	);
	trace((/*{*/"}\n"));
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


static void deu_func _((void *, dir_walk_message_ty, string_ty *, struct stat *));

static void
deu_func(arg, message, path, st)
	void		*arg;
	dir_walk_message_ty message;
	string_ty	*path;
	struct stat	*st;
{
	change_ty	*cp;
	int		uid;

	trace(("deu_func(message = %d, path = \"%s\", st = %08lX)\n{\n"/*}*/,
		message, path->str_text, st));
	cp = (change_ty *)arg;
	switch (message)
	{
	case dir_walk_dir_before:
	case dir_walk_file:
		os_become_query(&uid, (int *)0, (int *)0);
		if (st->st_uid == uid)
		{
			os_chmod(path, st->st_mode | 0200);
			undo_chmod(path, st->st_mode);
		}
		repair_diff_time(cp, path);
		break;

	case dir_walk_special:
	case dir_walk_symlink:
	case dir_walk_dir_after:
		break;
	}
	trace((/*{*/"}\n"));
}


static void develop_end_undo_main _((void));

static void
develop_end_undo_main()
{
	cstate		cstate_data;
	pstate		pstate_data;
	cstate_history	history_data;
	int		j;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	user_ty		*up;
	string_ty	*dd;

	trace(("develop_end_undo_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(develop_end_undo_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				develop_end_undo_usage();
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
				develop_end_undo_usage();
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
		change_number = user_default_change(up);;
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
	 * It is an error if the change is not in one of the 'being_reviewed'
	 * or 'awaiting_integration' states.
	 * It is an error if the current user did not develop the change.
	 */
	if
	(
		cstate_data->state != cstate_state_being_reviewed
	&&
		cstate_data->state != cstate_state_awaiting_integration
	)
		change_fatal(cp, "not in 'being_reviewed' state");
	if (!str_equal(change_developer_name(cp), user_name(up)))
	{
		change_fatal
		(
			cp,
			"user \"%S\" was not the developer",
			user_name(up)
		);
	}

	/*
	 * Change the state.
	 * Add to the change's history.
	 */
	cstate_data->state = cstate_state_being_developed;
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_develop_end_undo;
	cstate_data->build_time = 0;

	/*
	 * add it back into the user's change list
	 */
	user_own_add(up, project_name_get(pp), change_number);

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
			/* this is really a corrupted file */
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
	 * change the ownership back to the user
	 */
	dd = change_development_directory_get(cp, 1);
	user_become(up);
	dir_walk(dd, deu_func, cp);
	user_become_undo();

	/*
	 * write out the data and release the locks
	 */
	change_cstate_write(cp);
	project_pstate_write(pp);
	user_ustate_write(up);
	commit();
	lock_release();

	/*
	 * run the notify command
	 */
	change_run_develop_end_undo_notify_command(cp);

	/*
	 * verbose success message
	 */
	change_verbose(cp, "development resumed");
	change_free(cp);
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
develop_end_undo()
{
	trace(("develop_end_undo()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		develop_end_undo_main();
		break;

	case arglex_token_help:
		develop_end_undo_help();
		break;

	case arglex_token_list:
		develop_end_undo_list();
		break;
	}
	trace((/*{*/"}\n"));
}
