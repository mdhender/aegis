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
 * MANIFEST: functions to implement test
 */

#include <stdio.h>
#include <ac/stdlib.h>

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
	fprintf
	(
		stderr,
		"usage: %s -Test [ <filename>... ][ <option>... ]\n",
		progname
	);
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
#include <../man1/aet.h>
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
	string_ty	*bl;	/* resolved */
	string_ty	*top;	/* unresolved */
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
	string_ty	*dir;	/* unresolved */
	int		j;
	int		npassed;
	int		nfailed;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	int		nolog;
	int		(*run_test_command)_((change_ty *, user_ty *,
				string_ty *, string_ty *, int));
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
				fatal
				(
					"duplicate %s option",
					arglex_value.alv_string
				);
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
		{
			fatal
			(
			      "may not name files and use the -AUTOmatic option"
			);
		}
		if (manual_flag)
		{
			fatal("may not name files and use the -MANual option");
		}
		if (regression_flag)
		{
			fatal
			(
			     "may not name files and use the -REGression option"
			);
		}
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
	run_test_command = change_run_test_command;
	switch (cstate_data->state)
	{
	case cstate_state_being_developed:
		if (!str_equal(change_developer_name(cp), user_name(up)))
		{
			change_fatal
			(
				cp,
	     "user \"%S\" is not the developer, only user \"%S\" may run tests",
				user_name(up),
				change_developer_name(cp)
			);
		}
		if (baseline_flag)
			dir = project_baseline_path_get(pp, 0);
		else
		{
			dir = change_development_directory_get(cp, 0);
			trace_string(dir->str_text);
			run_test_command = change_run_development_test_command;
		}
		break;

	case cstate_state_being_integrated:
		if (!str_equal(change_integrator_name(cp), user_name(up)))
		{
			change_fatal
			(
				cp,
	    "user \"%S\" is not the integrator, only user \"%S\" may run tests",
				user_name(up),
				change_integrator_name(cp)
			);
		}
		if (baseline_flag)
			dir = project_baseline_path_get(pp, 0);
		else
		{
			if (devdir_flag)
				dir = change_development_directory_get(cp, 0);
			else
				dir = change_integration_directory_get(cp, 0);
		}
		break;

	default:
		change_fatal
		(
			cp,
"this change is in the '%s' state, \
it must be in the 'being developed' or 'being integrated' state to run tests",
			cstate_state_ename(cstate_data->state)
		);
	}
	assert(dir);

	/*
	 * see if this is a complete change test.
	 * If it is, we can update the relevant test time field.
	 */
	os_throttle();
	if (automatic_flag && manual_flag && !regression_flag && !wl.wl_nwords)
	{
		if (baseline_flag)
			change_test_baseline_time_set(cp);
		else
			change_test_time_set(cp);
	}
	if (regression_flag)
		change_regression_test_time_set(cp);

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
		{
			s2 =
				os_below_dir
				(
					change_integration_directory_get(cp, 1),
					s1
				);
		}
		if (!s2)
		{
			s2 =
				os_below_dir
				(
					change_development_directory_get(cp, 1),
					s1
				);
		}
		if (!s2)
			change_fatal(cp, "path \"%S\" unrelated", s1);
		c_src_data = change_src_find(cp, s2);
		if (c_src_data)
		{
			if
			(
				c_src_data->usage != file_usage_test
			&&
				c_src_data->usage != file_usage_manual_test
			)
			{
				change_fatal
				(
					cp,
					"file \"%S\" is not a test",
					s2
				);
			}
			if (c_src_data->action == file_action_remove)
			{
				change_fatal
				(
					cp,
					"file \"%S\" is being removed",
					s2
				);
			}
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
			{
				p_src_data = project_src_find_fuzzy(pp, s2);
				if (p_src_data)
				{
					project_fatal
					(
						pp,
			     "file \"%S\" unknown, closest was the \"%S\" file",
						s2,
						p_src_data->file_name
					);
				}
				else
				{
					change_fatal
					(
						cp,
						"file \"%S\" unknown",
						s2
					);
				}
			}
			if
			(
				p_src_data->usage != file_usage_test
			&&
				p_src_data->usage != file_usage_manual_test
			)
			{
				project_fatal
				(
					pp,
					"file \"%S\" is not a test",
					s2
				);
			}
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
						(
							p_src_data->usage
						==
							file_usage_manual_test
						)
					&&
						manual_flag
					)
				)
				{
					wl_append
					(
						&pfile,
						p_src_data->file_name
					);
				}
			}
		}
		else
		{
			for (j = 0; j < cstate_data->src->length; ++j)
			{
				c_src_data = cstate_data->src->list[j];
				if (c_src_data->action == file_action_remove)
					continue;
				if
				(
					baseline_flag
				&&
					c_src_data->action != file_action_create
				)
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
						(
							c_src_data->usage
						==
							file_usage_manual_test
						)
					&&
						manual_flag
					)
				)
				{
					wl_append
					(
						&cfile,
						c_src_data->file_name
					);
					if
					(
						c_src_data->usage
					==
						file_usage_manual_test
					)
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
			log_open
			(
				change_logfile_get(cp),
				pup,
				log_style_snuggle
			);
			user_free(pup);
		}
		else
			log_open(change_logfile_get(cp), up, log_style_snuggle);
	}
	npassed = 0;
	nfailed = 0;

	/*
	 * During long tests the automounter can unmount the
	 * directories referenced by the ``dir'' and ``top''
	 * variables.  To minimize this, it is essential that they are
	 * unresolved, and thus always trigger the automounter.
	 */
	trace_string(dir->str_text);
	if (cstate_data->state == cstate_state_being_integrated)
		top = change_integration_directory_get(cp, 0);
	else
		top = change_development_directory_get(cp, 0);
	for (j = 0; j < cfile.wl_nwords; ++j)
	{
		int		inp;
		int		result;

		s1 = cfile.wl_word[j];
		c_src_data = change_src_find(cp, s1);
		assert(c_src_data);
		s2 = str_format("%S/%S", top, s1);
		inp = (c_src_data->usage == file_usage_manual_test);
		result = run_test_command(cp, up, s2, dir, inp);
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
	top = project_baseline_path_get(pp, 0);
	for (j = 0; j < pfile.wl_nwords; ++j)
	{
		int		result;
		int		inp;

		s1 = pfile.wl_word[j];
		p_src_data = project_src_find(pp, s1);
		assert(p_src_data);
		s2 = str_format("%S/%S", top, s1);
		inp = (p_src_data->usage == file_usage_manual_test);
		result = run_test_command(cp, up, s2, dir, inp);
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

	/*
	 * verbose result message
	 */
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
	change_ty	*cp;
	user_ty		*up;

	trace(("test_independent()\n{\n"/*}*/));
	project_name = 0;
	automatic_flag = 0;
	manual_flag = 0;
	wl_zero(&wl);
	arglex();
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
				fatal
				(
					"duplicate %s option",
					arglex_value.alv_string
				);
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
		{
			fatal
			(
			      "may not name files and use the -AUTOmatic option"
			);
		}
		if (manual_flag)
		{
			fatal("may not name files and use the -MANual option");
		}
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
	 * locate user data
	 */
	up = user_executing(pp);

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
	 * create a fake change,
	 * so can set environment variables 
	 * for the test
	 */
	cp = change_alloc(pp, pstate_data->next_change_number);
	change_bind_new(cp);
	change_architecture_from_pconf(cp);
	cp->bogus = 1;

	/*
	 * do each of the tests
	 * (Logging is disabled, because there is no [logical] place
	 * to put the log file; the user should redirect stdout and stderr.)
	 */
	npassed = 0;
	nfailed = 0;
	for (j = 0; j < wl.wl_nwords; ++j)
	{
		string_ty	*fn;
		string_ty	*path;
		int		inp;
		int		result;

		fn = wl.wl_word[j];
		path = str_format("%S/%S", bl, fn);
		src_data = project_src_find(pp, fn);
		assert(src_data);
		inp = (src_data->usage == file_usage_manual_test);
		result = change_run_test_command(cp, up, path, bl, inp);
		str_free(path);

		if (result)
		{
			project_verbose(pp, "test \"%S\" failed", fn);
			++nfailed;
		}
		else
		{
			project_verbose(pp, "test \"%S\" passed", fn);
			++npassed;
		}
	}

	/*
	 * verbose result message
	 */
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

	/*
	 * clean up and go home
	 */
	change_free(cp);
	user_free(up);
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
