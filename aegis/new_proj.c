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
 * MANIFEST: functions to create and destroy projects
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arglex2.h>
#include <commit.h>
#include <error.h>
#include <gonzo.h>
#include <help.h>
#include <list.h>
#include <lock.h>
#include <new_proj.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


static void new_project_usage _((void));

static void
new_project_usage()
{
	char	*progname;

	progname = option_get_progname();
	fprintf(stderr, "usage: %s -New_Project <name> [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -New_Project -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -New_Project -Help\n", progname);
	quit(1);
}


static void new_project_help _((void));

static void
new_project_help()
{
	static char *text[] =
	{
"NAME",
"	%s -New_PRoject - create a new project",
"",
"SYNOPSIS",
"	%s -New_PRoject <project-name> [ <option>... ]",
"	%s -New_PRoject -List [ <option>... ]",
"	%s -New_PRoject -Help",
"",
"DESCRIPTION",
"	The %s -New_PRoject command is used to create a new",
"	project.",
"",
"	The project directory, under which the project baseline",
"	and history and state and change data are kept, will be",
"	created at this time.  If the -DIRectory option is not",
"	given, the project directory will be created in the",
"	directory specified by the default_project_directory field",
"	of aeuconf(5), or if not set in current user's home",
"	directory; in either case with the same name as the",
"	project.",
"",
"	The project is created with the current user and group as",
"	the owning user and group.  The current user is an",
"	administrator for the project.  The project has no",
"	developers, reviewers, integrators or other",
"	administrators.",
"",
"	The project pointer will be added to the first element of",
"	the search path, or /usr/local/lib/%s if no path is",
"	set.  If this is inappropriate, use the -LIBrary option",
"	to explicitly set the desired location.  See the -LIBrary",
"	option for more information.",
"",
"OPTIONS",
"	The following options are understood:",
"",
"	-DIRectory <path>",
"		This option may be used to specify which directory",
"		is to be used.  It is an error if the current user",
"		does not have appropriate permissions to create",
"		the directory path given.  This must be an",
"		absolute path.",
"",
"		Caution: If you are using an automounter do not",
"		use `pwd` to make an absolute path, it usually",
"		gives the wrong answer.",
"",
"	-Help",
"		This option may be used to obtain more information",
"		about how to use the %s program.",
"",
"	-LIBrary <abspath>",
"		This option may be used to specify a directory to",
"		be searched for global state files and user state",
"		files.  (See aegstate(5) and aeustate(5) for more",
"		information.)  Several library options may be",
"		present on the command line, and are search in the",
"		order given.  Appended to this explicit search",
"		path are the directories specified by the AEGIS",
"		enviroment variable (colon separated), and",
"		finally, /usr/local/lib/%s is always searched.",
"		All paths specified, either on the command line or",
"		in the AEGIS environment variable, must be",
"		absolute.",
"",
"	-List",
"		This option may be used to obtain a list of",
"		suitable subjects for this command.  The list may",
"		be more general than expected.",
"",
"	-TERse",
"		This option may be used to cause listings to",
"		produce the bare minimum of information.  It is",
"		usually useful for shell scripts.",
"",
"	-Verbose",
"		This option may be used to cause %s to produce",
"		more output.  By default %s only produces",
"		output on errors.  When used with the -List option",
"		this option causes column headings to be added.",
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
"	csh%%	alias aenpr '%s -npr \\!* -v'",
"	sh$	aenpr(){%s -npr $* -v}",
"",
"ERRORS",
"	It is an error if the project name already exists.",
"	It is an error if the project directory already exists.",
"	It is an error if the currrent user does not have suffient",
"	permissions to create the directory specified with the",
"	-DIRectory option.",
"",
"EXIT STATUS",
"	The %s command will exit with a status of 1 on any",
"	error.  The %s command will only exit with a status of",
"	0 if there are no errors.",
"",
"COPYRIGHT",
"	%C",
"",
"AUTHOR",
"	%A",
	};

	help(text, SIZEOF(text), new_project_usage);
}


static void new_project_list _((void));

static void
new_project_list()
{
	arglex();
	while (arglex_token != arglex_token_eoln)
		generic_argument(new_project_usage);
	list_projects(0, 0);
}


static void new_project_main _((void));

static void
new_project_main()
{
	pstate		pstate_data;
	string_ty	*home;
	string_ty	*s1;
	string_ty	*project_name;
	project_ty	*pp;
	user_ty		*up;
	string_ty	*bl;
	string_ty	*hp;
	string_ty	*ip;

	trace(("new_project_main()\n{\n"/*}*/));
	project_name = 0;
	home = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(new_project_usage);
			continue;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				new_project_usage();
			/* fall through... */

		case arglex_token_string:
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_directory:
			if (arglex() != arglex_token_string)
		  		fatal
				(
			  "the -DIRectory option must be followed by a pathname"
				);
			if (home)
				fatal("duplicate -DIRectory option");
			/*
			 * To cope with automounters, directories are stored as
			 * given, or are derived from the home directory in the
			 * passwd file.  Within aegis, pathnames have their
			 * symbolic links resolved, and any comparison of paths
			 * is done on this "system idea" of the pathname.
			 */
			home = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	if (!project_name)
		fatal("no project name given");
	
	/*
	 * read in the table
	 */
	gonzo_gstate_lock_prepare_new();
	lock_take();

	/*
	 * locate user data
	 */
	up = user_executing((project_ty *)0);

	/*
	 * make sure not too privileged
	 */
	if (!user_uid_check(up->name))
	{
		fatal
		(
			"user \"%s\" is too privileged",
			up->name->str_text
		);
	}
	if (!user_gid_check(up->group))
	{
		fatal
		(
			"group \"%s\" is too privileged",
			up->group->str_text
		);
	}

	/*
	 * it is an error if the name is already in use
	 */
	pp = project_alloc(project_name);
	str_free(project_name);
	project_bind_new(pp);

	/*
	 * create a new project state file
	 */
	pstate_data = project_pstate_get(pp);
	pstate_data->description =
		str_format("The \"%S\" program.", project_name_get(pp));
	pstate_data->next_change_number = 1;
	pstate_data->next_delta_number = 1;
	pstate_data->version_major = 1;
	pstate_data->version_minor = 0;
	pstate_data->owner_name = str_copy(user_name(up));
	pstate_data->group_name = str_copy(user_group(up));
	assert(pstate_data->administrator);
	project_administrator_add(pp, user_name(up));

	/*
	 * if no project directory was specified
	 * create the directory in their home directory.
	 */
	if (!home)
	{
		s1 = user_default_project_directory(up);
		assert(s1);
		home = str_format("%S/%S", s1, project_name_get(pp));
		str_free(s1);
	}
	project_verbose(pp, "project directory \"%S\"", home);
	project_home_path_set(pp, home);
	str_free(home);

	/*
	 * create the diectory and subdirectories.
	 * It is an error if the directories can't be created.
	 */
	s1 = project_home_path_get(pp);
	bl = project_baseline_path_get(pp, 0);
	hp = project_history_path_get(pp);
	ip = project_info_path_get(pp);
	project_become(pp);
	os_mkdir(s1, 02755);
	undo_rmdir_errok(s1);
	os_mkdir(bl, 02755);
	undo_rmdir_errok(bl);
	os_mkdir(hp, 02755);
	undo_rmdir_errok(hp);
	os_mkdir(ip, 02755);
	undo_rmdir_errok(ip);
	project_become_undo();

	/*
	 * add a row to the table
	 */
	gonzo_project_add(pp);

	/*
	 * write the project pointer back out
	 * release locks
	 */
	project_pstate_write(pp);
	gonzo_gstate_write();
	commit();
	lock_release();

	/*
	 * verbose success message
	 */
	project_verbose(pp, "created");
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
new_project()
{
	trace(("new_project()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		new_project_main();
		break;

	case arglex_token_help:
		new_project_help();
		break;

	case arglex_token_list:
		new_project_list();
		break;
	}
	trace((/*{*/"}\n"));
}
