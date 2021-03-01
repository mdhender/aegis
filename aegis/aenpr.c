/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994 Peter Miller.
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
 * MANIFEST: functions to implement new project
 */

#include <stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>

#include <ael.h>
#include <aenpr.h>
#include <arglex2.h>
#include <commit.h>
#include <error.h>
#include <gonzo.h>
#include <help.h>
#include <lock.h>
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

	progname = option_progname_get();
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
#include <../man1/aenpr.h>
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
	long		major;
	long		minor;
	int		um;

	trace(("new_project_main()\n{\n"/*}*/));
	project_name = 0;
	home = 0;
	major = 0;
	minor = 0;
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
			if (home)
			{
				duplicate:
				fatal
				(
					"duplicate %s option",
					arglex_value.alv_string
				);
			}
			if (arglex() != arglex_token_string)
			{
		  		fatal
				(
			  "the -DIRectory option must be followed by a pathname"
				);
			}

			/*
			 * To cope with automounters, directories are stored as
			 * given, or are derived from the home directory in the
			 * passwd file.  Within aegis, pathnames have their
			 * symbolic links resolved, and any comparison of paths
			 * is done on this "system idea" of the pathname.
			 */
			home = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_major:
			if (major)
				goto duplicate;
			if (arglex() != arglex_token_number)
			{
				major_bad:
				error
				(
		       "the -MAJor option must be followed by a positive number"
				);
				new_project_usage();
			}
			major = arglex_value.alv_number;
			if (major <= 0)
				goto major_bad;
			break;

		case arglex_token_minor:
			if (minor)
				goto duplicate;
			if (arglex() != arglex_token_number)
			{
				minor_bad:
				error
				(
		       "the -MINOr option must be followed by a positive number"
				);
				new_project_usage();
			}
			minor = arglex_value.alv_number;
			if (minor <= 0)
				goto minor_bad;
			break;
		}
		arglex();
	}
	if (!project_name)
		fatal("no project name given");
	if (!major)
		major = 1;
	if (!minor)
		minor = 0;
	
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
	pstate_data->version_major = major;
	pstate_data->version_minor = minor;
	pstate_data->owner_name = str_copy(user_name(up));
	pstate_data->group_name = str_copy(user_group(up));
	assert(pstate_data->administrator);
	project_administrator_add(pp, user_name(up));

	/*
	 * default the umask from the creating user
	 */
	os_become_orig_query((int *)0, (int *)0, &um);
	um = (um & 5) | 022;
	if (um == 023)
		um = 022;
	pstate_data->umask = um;

	/*
	 * if no project directory was specified
	 * create the directory in their home directory.
	 */
	if (!home)
	{
		int	max;

		s1 = user_default_project_directory(up);
		assert(s1);
		os_become_orig();
		max = os_pathconf_name_max(s1);
		os_become_undo();
		if (project_name_get(pp)->str_length > max)
		{
			fatal
			(
				"project name \"%s\" too long (by %ld)",
				project_name_get(pp)->str_text,
				project_name_get(pp)->str_length - max
			);
		}
		home = str_format("%S/%S", s1, project_name_get(pp));
		str_free(s1);
		project_verbose(pp, "project directory \"%S\"", home);
	}
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
