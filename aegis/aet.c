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
 * MANIFEST: functions to implement test
 */

#include <stdio.h>
#include <stdlib.h>

#include <ael.h>
#include <aet.h>
#include <arglex2.h>
#include <change.h>
#include <commit.h>
#include <error.h>
#include <help.h>
#include <lock.h>
#include <log.h>
#include <option.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <trace.h>
#include <user.h>
#include <word.h>


static void test_usage _((void));

static void
test_usage()
{
	char		*progname;

	progname = option_progname_get();
	fprintf(stderr, "usage: %s -Test [ <filename>... ][ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Test -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Test -Help\n", progname);
	quit(1);
}


static void test_help _((void));

static void
test_help()
{
	static char *text[] =
	{
"NAME",
"	%s -Test - run tests",
"",
"SYNOPSIS",
"	%s -Test [ <file-name>... ][ <option>... ]",
"	%s -Test -INDependent [ <file-name>... ][ <option>... ]",
"	%s -Test -List [ <option>... ]",
"	%s -Test -Help",
"",
"DESCRIPTION",
"	The %s -Test command is used to run tests.  If no",
"	files are named, all relevant tests are run.  By default",
"	both automatic and manual tests are run.",
"",
"	The %s program will attempt to intuit the file names",
"	intended.  All file names are stored within %s as",
"	relative to the root of the baseline directory tree.  The",
"	development directory and the integration directory are",
"	shadows of the baseline directory, and so these relative",
"	names aply there, too.	Files named on the command line",
"	are first converted to absolute paths if necessary.  They",
"	are then compared with the baseline path, and the",
"	development directory path, and the integration directory",
"	path, to determine a root-relative name.  It is an error",
"	if the file named is outside one of these directory",
"	trees.",
"",
"	Tests are executed by the Bourne shell.  The current directory",
"	will be the top of the appropriate directory tree.  If",
"	tests require temporary files, they should create them in",
"	/tmp, as a test cannot expect to have write permission in",
"	the current directory.",
"",
"OPTIONS",
"	The following options are understood:",
"",
"	-AUTOmatic",
"		This option may be uset to specify manual tests.",
"		Automatic tests require no human assitance.",
"",
"	-BaseLine",
"		This option may be used to specify that the",
"		project baseline is the subject of the command.",
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
"	-Development_Directory",
"		This option is ised to specify that the",
"		development directory is the subject of the",
"		command.  This is only useful for a change which",
"		is in the 'being_integrated' state, when the",
"		default is the integration directory.",
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
"	-MANual",
"		This option may be used to specify manual tests.",
"		Manual tests require some human intervention,",
"		e.g.:  confirmation of some screen behaviour",
"		(X11, for instance), or some user action, \"unplug",
"		ethernet cable now\".",
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
"	csh%%	alias aet '%s -t \\!* -v'",
"	sh$	aet(){%s -t $* -v}",
"",
"ERRORS",
"	It is an error if the change is not in one of the",
"	'being_developed' or 'being_integrated' states.",
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

	help(text, SIZEOF(text), test_usage);
}


static void test_list _((void));

static void
test_list()
{
	string_ty	*project_name;
	long		change_number;

	trace(("test_list()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	arglex();
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(test_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				test_usage();
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
				test_usage();
			if (project_name)
				fatal("duplicate -Project option");
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


static void test_main _((void));

static void
test_main()
{
	string_ty	*bl;
	string_ty	*top;
	int		baseline_flag;
	int		devdir_flag;
	int		regression_flag;
	int		manual_flag;
	int		automatic_flag;
	wlist		wl;
	wlist		cfile;
	wlist		pfile;
	string_ty	*s1;
	string_ty	*s2;
	pstate		pstate_data;
	pstate_src	p_src_data;
	cstate		cstate_data;
	cstate_src	c_src_data;
	string_ty	*dir;
	int		j;
	int		npassed;
	int		nfailed;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	int		nolog;
	user_ty		*up;

	trace(("test_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	baseline_flag = 0;
	devdir_flag = 0;
	regression_flag = 0;
	manual_flag = 0;
	automatic_flag = 0;
	nolog = 0;
	wl_zero(&wl);
	wl_zero(&cfile);
	wl_zero(&pfile);
	dir = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(test_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				test_usage();
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				fatal("duplicate -Change option");
			change_number = arglex_value.alv_number;
			if (change_number < 1)
				fatal("change %ld out of range", change_number);
			break;

		case arglex_token_regression:
			if (regression_flag)
			{
				duplicate:
				fatal("duplicate %s option", arglex_value.alv_string);
			}
			regression_flag = 1;
			break;

		case arglex_token_manual:
			if (manual_flag)
				goto duplicate;
			manual_flag = 1;
			break;

		case arglex_token_automatic:
			if (automatic_flag)
				goto duplicate;
			automatic_flag = 1;
			break;

		case arglex_token_string:
			s1 = str_from_c(arglex_value.alv_string);
			os_become_orig();
			s2 = os_pathname(s1, 1);
			os_become_undo();
			if (wl_member(&wl, s1))
			{
				fatal
				(
					"file \"%s\" named more than once",
					arglex_value.alv_string
				);
			}
			wl_append(&wl, s2);
			str_free(s1);
			str_free(s2);
			break;

		case arglex_token_development_directory:
			if (devdir_flag)
				goto duplicate;
			devdir_flag = 1;
			break;

		case arglex_token_baseline:
			if (baseline_flag)
				goto duplicate;
			baseline_flag = 1;
			break;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				test_usage();
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
	if (wl.wl_nwords)
	{
		if (automatic_flag)
			fatal("may not name files and use the -AUTOmatic option");
		if (manual_flag)
			fatal("may not name files and use the -MANual option");
		if (regression_flag)
			fatal("may not name files and use the -REGression option");
	}
	else
	{
		if (!manual_flag && !automatic_flag)
		{
			manual_flag = 1;
			automatic_flag = 1;
		}
	}
	trace_int(manual_flag);
	trace_int(automatic_flag);

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
	 * take a lock on the change
	 */
	change_cstate_lock_prepare(cp);
	lock_take();
	pstate_data = project_pstate_get(pp);
	cstate_data = change_cstate_get(cp);

	/*
	 * see if it is an appropriate thing to be doing
	 */
	bl = project_baseline_path_get(pp, 1);
	switch (cstate_data->state)
	{
	case cstate_state_being_developed:
		if (!str_equal(change_developer_name(cp), user_name(up)))
		{
			change_fatal
			(
				cp,
				"user \"%S\" is not the developer",
				user_name(up)
			);
		}
		if (baseline_flag)
			dir = bl;
		else
		{
			dir = change_development_directory_get(cp, 1);
			trace_string(dir->str_text);
		}
		break;

	case cstate_state_being_integrated:
		if (!str_equal(change_integrator_name(cp), user_name(up)))
		{
			change_fatal
			(
				cp,
				"user \"%S\" is not the integrator",
				user_name(up)
			);
		}
		if (baseline_flag)
			dir = bl;
		else
		{
			if (devdir_flag)
				dir = change_development_directory_get(cp, 1);
			else
				dir = change_integration_directory_get(cp, 1);
		}
		break;

	default:
		change_fatal(cp, "not in 'being_developed' state");
	}
	assert(dir);

	/*
	 * see if this is a complete change test.
	 * If it is, we can update the relevant test time field.
	 */
	if (automatic_flag && manual_flag && !regression_flag && !wl.wl_nwords)
	{
		if (baseline_flag)
			cstate_data->test_baseline_time = time((time_t *)0);
		else
			cstate_data->test_time = time((time_t *)0);
	}
	if (regression_flag)
		cstate_data->regression_test_time = time((time_t *)0);

	/*
	 * check that the named files make sense
	 * split the named files into two separate lists
	 */
	for (j = 0; j < wl.wl_nwords; ++j)
	{
		s1 = wl.wl_word[j];
		assert(s1->str_text[0] == '/');
		s2 = os_below_dir(bl, s1);
		if (!s2 && cstate_data->state == cstate_state_being_integrated)
			s2 = os_below_dir(change_integration_directory_get(cp, 1), s1);
		if (!s2)
			s2 = os_below_dir(change_development_directory_get(cp, 1), s1);
		if (!s2)
			change_fatal(cp, "path \"%S\" unrelated", s1);
		c_src_data = change_src_find(cp, s2);
		if (c_src_data)
		{
			if (c_src_data->action == file_action_remove)
				change_fatal(cp, "file \"%S\" is being removed", s2);
			if
			(
				c_src_data->usage != file_usage_test
			&&
				c_src_data->usage != file_usage_manual_test
			)
				change_fatal(cp, "file \"%S\" is not a test", s2);
			wl_append(&cfile, s2);
			if (c_src_data->usage == file_usage_manual_test)
				nolog = 1;
		}
		else
		{
			p_src_data = project_src_find(pp, s2);
			if
			(
				!p_src_data
			||
				p_src_data->deleted_by
			||
				p_src_data->about_to_be_created_by
			)
				change_fatal(cp, "file \"%S\" unknown", s2);
			if
			(
				p_src_data->usage != file_usage_test
			&&
				p_src_data->usage != file_usage_manual_test
			)
				project_fatal(pp, "file \"%S\" is not a test", s2);
			wl_append(&pfile, s2);
			if (p_src_data->usage == file_usage_manual_test)
				nolog = 1;
		}
		str_free(s2);
	}

	if (automatic_flag || manual_flag)
	{
		assert(!cfile.wl_nwords);
		assert(!pfile.wl_nwords);
		if (regression_flag)
		{
			/*
			 * run through the project test files
			 */
			for (j = 0; j < pstate_data->src->length; ++j)
			{
				p_src_data = pstate_data->src->list[j];

				/*
				 * don't run the test if it does not exist
				 */
				if
				(
					p_src_data->deleted_by
				||
					p_src_data->about_to_be_created_by
				)
					continue;

				/*
				 * don't run the test if it is being
				 * modified by the change
				 */
				if (change_src_find(cp, p_src_data->file_name))
					continue;

				/*
				 * run the test if it satisfies the request
				 */
				if
				(
					(
					    p_src_data->usage == file_usage_test
					&&
						automatic_flag
					)
				||
					(
				     p_src_data->usage == file_usage_manual_test
					&&
						manual_flag
					)
				)
					wl_append(&pfile, p_src_data->file_name);
			}
		}
		else
		{
			for (j = 0; j < cstate_data->src->length; ++j)
			{
				c_src_data = cstate_data->src->list[j];
				if (c_src_data->action == file_action_remove)
					continue;
				if (baseline_flag && c_src_data->action != file_action_create)
					continue;
				if
				(
					(
					    c_src_data->usage == file_usage_test
					&&
						automatic_flag
					)
				||
					(
				     c_src_data->usage == file_usage_manual_test
					&&
						manual_flag
					)
				)
				{
					wl_append(&cfile, c_src_data->file_name);
					if (c_src_data->usage == file_usage_manual_test)
						nolog = 1;
				}
			}
		}
	}

	/*
	 * make sure we are actually doing something
	 */
	if (!cfile.wl_nwords && !pfile.wl_nwords)
		change_fatal(cp, "has no tests");

	/*
	 * Do each of the change's tests.
	 * Log if no manual tests.
	 */
	if (!nolog)
	{
		/*
		 * open the log file as the appropriate user
		 */
		if (cstate_data->state == cstate_state_being_integrated)
		{
			user_ty	*pup;

			pup = project_user(pp);
			log_open(change_logfile_get(cp), pup);
			user_free(pup);
		}
		else
			log_open(change_logfile_get(cp), up);
	}
	npassed = 0;
	nfailed = 0;
	trace_string(dir->str_text);
	if (cstate_data->state == cstate_state_being_integrated)
		top = change_integration_directory_get(cp, 1);
	else
		top = change_development_directory_get(cp, 1);
	for (j = 0; j < cfile.wl_nwords; ++j)
	{
		int		result;
		int		flags;

		s1 = cfile.wl_word[j];
		c_src_data = change_src_find(cp, s1);
		assert(c_src_data);
		s2 = str_format("%s %S/%S", os_shell(), top, s1);
		if (c_src_data->usage == file_usage_manual_test)
			flags = OS_EXEC_FLAG_INPUT;
		else
			flags = OS_EXEC_FLAG_NO_INPUT;
		user_become(up);
		result = os_execute_retcode(s2, flags, dir);
		os_become_undo();
		str_free(s2);
		if (baseline_flag)
		{
			if (result)
			{
				change_verbose
				(
					cp,
				"test \"%S\" on baseline failed (as it should)",
					s1
				);
				++npassed;
			}
			else
			{
				change_verbose
				(
					cp,
				"test \"%S\" on baseline passed (it must fail)",
					s1
				);
				++nfailed;
				cstate_data->test_baseline_time = 0;
			}
		}
		else
		{
			if (result)
			{
				change_verbose(cp, "test \"%S\" failed", s1);
				++nfailed;
				cstate_data->test_time = 0;
			}
			else
			{
				change_verbose(cp, "test \"%S\" passed", s1);
				++npassed;
			}
		}
	}

	/*
	 * Do each of the project's tests.
	 * Log if no manual tests.
	 */
	for (j = 0; j < pfile.wl_nwords; ++j)
	{
		int		result;
		int		flags;

		s1 = pfile.wl_word[j];
		p_src_data = project_src_find(pp, s1);
		assert(p_src_data);
		s2 = str_format("%s %S/%S", os_shell(), bl, s1);
		if (p_src_data->usage == file_usage_manual_test)
			flags = OS_EXEC_FLAG_INPUT;
		else
			flags = OS_EXEC_FLAG_NO_INPUT;
		user_become(up);
		result = os_execute_retcode(s2, flags, dir);
		os_become_undo();
		str_free(s2);
		if (result)
		{
			change_verbose(cp, "test \"%S\" failed", s1);
			++nfailed;
			cstate_data->test_time = 0;
			cstate_data->regression_test_time = 0;
		}
		else
		{
			change_verbose(cp, "test \"%S\" passed", s1);
			++npassed;
		}
	}
	if (npassed)
	{
		change_verbose
		(
			cp,
			"passed %d test%s",
			npassed,
			((npassed == 1) ? "" : "s")
		);
	}
	if (nfailed)
	{
		change_fatal
		(
			cp,
			"failed %d test%s",
			nfailed,
			((nfailed == 1) ? "" : "s")
		);
	}

	/*
	 * write out the data
	 */
	change_cstate_write(cp);
	commit();
	lock_release();
	change_free(cp);
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


static void test_independent _((void));

static void
test_independent()
{
	string_ty	*bl;
	int		automatic_flag;
	int		manual_flag;
	wlist		wl;
	string_ty	*s1;
	string_ty	*s2;
	pstate		pstate_data;
	pstate_src	src_data;
	int		j;
	int		npassed;
	int		nfailed;
	string_ty	*project_name;
	project_ty	*pp;
	user_ty		*pup;

	trace(("test_independent()\n{\n"/*}*/));
	project_name = 0;
	automatic_flag = 0;
	manual_flag = 0;
	wl_zero(&wl);
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(test_usage);
			continue;

		case arglex_token_manual:
			if (manual_flag)
			{
				duplicate:
				fatal("duplicate %s option", arglex_value.alv_string);
			}
			manual_flag = 1;
			break;

		case arglex_token_automatic:
			if (automatic_flag)
				goto duplicate;
			automatic_flag = 1;
			break;

		case arglex_token_string:
			s1 = str_from_c(arglex_value.alv_string);
			os_become_orig();
			s2 = os_pathname(s1, 1);
			os_become_undo();
			if (wl_member(&wl, s1))
			{
				fatal
				(
					"path \"%s\" named more than once",
					arglex_value.alv_string
				);
			}
			wl_append(&wl, s2);
			str_free(s1);
			str_free(s2);
			break;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				test_usage();
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	if (wl.wl_nwords)
	{
		if (automatic_flag)
			fatal("may not name files and use the -AUTOmatic option");
		if (manual_flag)
			fatal("may not name files and use the -MANual option");
	}
	else
	{
		if (!automatic_flag && !manual_flag)
		{
			automatic_flag = 1;
			manual_flag = 1;
		}
	}

	/*
	 * locate project data
	 */
	if (!project_name)
		project_name = user_default_project();
	pp = project_alloc(project_name);
	str_free(project_name);
	project_bind_existing(pp);
	pup = user_symbolic(pp, project_owner(pp));

	/*
	 * grab some info
	 */
	pstate_data = project_pstate_get(pp);

	/*
	 * make sure the paths make sense
	 */
	bl = project_baseline_path_get(pp, 1);
	for (j = 0; j < wl.wl_nwords; ++j)
	{
		s1 = wl.wl_word[j];
		assert(s1->str_text[0] == '/');
		s2 = os_below_dir(bl, s1);
		if (!s2)
			project_fatal(pp, "path \"%S\" unrelated", s1);
		src_data = project_src_find(pp, s2);
		if
		(
			!src_data
		||
			src_data->deleted_by
		||
			src_data->about_to_be_created_by
		)
			project_fatal(pp, "file \"%S\" unknown", s2);
		if
		(
			src_data->usage != file_usage_test
		&&
			src_data->usage != file_usage_manual_test
		)
			project_fatal(pp, "file \"%S\" not a test", s2);
		wl.wl_word[j] = s2;
		str_free(s1);
	}

	/*
	 * snarf the test names from the project
	 */
	if (automatic_flag || manual_flag)
	{
		for (j = 0; j < pstate_data->src->length; ++j)
		{
			pstate_src	src_data;

			src_data = pstate_data->src->list[j];
			if
			(
				src_data->deleted_by
			||
				src_data->about_to_be_created_by
			)
				continue;
			if
			(
				(
					src_data->usage == file_usage_test
				&&
					automatic_flag
				)
			||
				(
				       src_data->usage == file_usage_manual_test
				&&
					manual_flag
				)
			)
				wl_append(&wl, src_data->file_name);
		}
		if (!wl.wl_nwords)
			project_fatal(pp, "has no tests");
	}

	/*
	 * do each of the tests
	 * (Logging is disabled, because there is no [logical] place
	 * to put the log file; the user should redirect stdout and stderr.)
	 */
	npassed = 0;
	nfailed = 0;
	for (j = 0; j < wl.wl_nwords; ++j)
	{
		int		result;
		pstate_src	src_data;
		int		flags;

		src_data = project_src_find(pp, wl.wl_word[j]);
		assert(src_data);
		if (src_data->usage == file_usage_manual_test)
			flags = OS_EXEC_FLAG_INPUT;
		else
			flags = OS_EXEC_FLAG_NO_INPUT;
		project_become(pp);
		result =
			os_execute_retcode
			(
				src_data->file_name,
				flags,
				bl
			);
		os_become_undo();
		if (result)
		{
			project_verbose
			(
				pp,
				"test \"%S\" failed",
				src_data->file_name
			);
			++nfailed;
		}
		else
		{
			project_verbose
			(
				pp,
				"test \"%S\" passed",
				src_data->file_name
			);
			++npassed;
		}
	}
	if (npassed)
	{
		project_verbose
		(
			pp,
			"passed %d test%s",
			npassed,
			((npassed == 1) ? "" : "s")
		);
	}
	if (nfailed)
	{
		project_fatal
		(
			pp,
			"failed %d test%s",
			nfailed,
			((nfailed == 1) ? "" : "s")
		);
	}
	project_free(pp);
	user_free(pup);
	trace((/*{*/"}\n"));
}


void
test()
{
	trace(("test()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		test_main();
		break;

	case arglex_token_help:
		test_help();
		break;

	case arglex_token_list:
		test_list();
		break;

	case arglex_token_independent:
		test_independent();
		break;
	}
	trace((/*{*/"}\n"));
}
