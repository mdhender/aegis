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
 * MANIFEST: functions to implement new change
 */

#include <stdio.h>
#include <ac/stdlib.h>
#include <ac/time.h>

#include <aeca.h>
#include <aenc.h>
#include <ael.h>
#include <arglex2.h>
#include <cattr.h>
#include <change.h>
#include <col.h>
#include <commit.h>
#include <common.h>
#include <error.h>
#include <help.h>
#include <lock.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <trace.h>
#include <user.h>
#include <word.h>


static void new_change_usage _((void));

static void
new_change_usage()
{
	char	*progname;

	progname = option_progname_get();
	fprintf
	(
		stderr,
		"usage: %s -New_Change -File <attr-file> [ <option>... ]\n",
		progname
	);
	fprintf
	(
		stderr,
		"       %s -New_Change -Edit [ <option>... ]\n",
		progname
	);
	fprintf
	(
		
		stderr,
		"       %s -New_Change -List [ <option>... ]\n",
		progname
	);
	fprintf(stderr, "       %s -New_Change -Help\n", progname);
	quit(1);
}


static void new_change_help _((void));

static void
new_change_help()
{
	static char *text[] =
	{
#include <../man1/aenc.h>
	};

	help(text, SIZEOF(text), new_change_usage);
}


static void new_change_list _((void));

static void
new_change_list()
{
	string_ty	*project_name;

	trace(("new_chane_list()\n{\n"/*}*/));
	project_name = 0;
	arglex();
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(new_change_usage);
			continue;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				new_change_usage();
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	list_changes(project_name, 0);
	if (project_name)
		str_free(project_name);
	trace((/*{*/"}\n"));
}


static void cattr_defaults _((cattr, pstate, pconf));

static void
cattr_defaults(a, ps, pc)
	cattr		a;
	pstate		ps;
	pconf		pc;
{
	trace(("cattr_defaults(a = %08lX, ps = %08lX, pc = %08lX)\n{\n"/*}*/,
		(long)a, (long)ps, (long)pc));
	if
	(
		a->cause == change_cause_internal_improvement
	||
		a->cause == change_cause_internal_improvement
	)
	{
		if (!(a->mask & cattr_test_exempt_mask))
		{
			a->test_exempt = 1;
			a->mask |= cattr_test_exempt_mask;
		}
		if (!(a->mask & cattr_test_baseline_exempt_mask))
		{
			a->test_baseline_exempt = 1;
			a->mask |= cattr_test_baseline_exempt_mask;
		}
		if (!(a->mask & cattr_regression_test_exempt_mask))
		{
			a->regression_test_exempt = 0;
			a->mask |= cattr_regression_test_exempt_mask;
		}
	}
	else
	{
		if (!(a->mask & cattr_regression_test_exempt_mask))
		{
			a->regression_test_exempt = 1;
			a->mask |= cattr_regression_test_exempt_mask;
		}
	}
	if (!(a->mask & cattr_test_exempt_mask))
	{
		a->test_exempt = ps->default_test_exemption;
		a->mask |= cattr_test_exempt_mask;
	}
	if (!(a->mask & cattr_test_baseline_exempt_mask))
	{
		a->test_baseline_exempt = ps->default_test_exemption;
		a->mask |= cattr_test_baseline_exempt_mask;
	}

	if (!a->architecture)
		a->architecture =
			(cattr_architecture_list)
			cattr_architecture_list_type.alloc();
	assert(pc->architecture);
	assert(pc->architecture->length);
	if (!a->architecture->length)
	{
		long		j;

		for (j = 0; j < pc->architecture->length; ++j)
		{
			type_ty		*type_p;
			string_ty	**str_p;
			pconf_architecture pca;

			str_p =
				cattr_architecture_list_type.list_parse
				(
					a->architecture,
					&type_p
				);
			assert(type_p == &string_type);
			pca = pc->architecture->list[j];
			*str_p = str_copy(pca->name);
		}
	}
	trace((/*{*/"}\n"));
}


static void check_permissions _((project_ty *, user_ty *));

static void
check_permissions(pp, up)
	project_ty	*pp;
	user_ty		*up;
{
	pstate		pstate_data;

	pstate_data = project_pstate_get(pp);

	/*
	 * it is an error if
	 * the user is not an administrator for the project.
	 */
	if
	(
		!project_administrator_query(pp, user_name(up))
	&&
		(
			!pstate_data->developers_may_create_changes
		||
			!project_developer_query(pp, user_name(up))
		)
	)
	{
		project_fatal
		(
			pp,
			"user \"%S\" is not an administrator",
			user_name(up)
		);
	}
}


static void new_change_main _((void));

static void
new_change_main()
{
	pstate		pstate_data;
	cstate		cstate_data;
	cstate_history	history_data;
	cattr		cattr_data;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	user_ty		*up;
	int		edit;
	long		j;
	pconf		pconf_data;
	wlist		carch;
	wlist		darch;
	wlist		parch;

	trace(("new_change_main()\n{\n"/*}*/));
	cattr_data = 0;
	project_name = 0;
	edit = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(new_change_usage);
			continue;

		case arglex_token_string:
			error
			(
"warning: please use the -File option when specifying an attributes file, \
the unadorned form is now obsolescent"
			);
			if (cattr_data)
				fatal("too many files named");
			goto read_input_file;

		case arglex_token_file:
			if (cattr_data)
				goto duplicate;
			if (arglex() != arglex_token_string)
			{
				error
				(
				 "the -File option requires a filename argument"
				);
				new_change_usage();
			}
			read_input_file:
			os_become_orig();
			cattr_data = cattr_read_file(arglex_value.alv_string);
			os_become_undo();
			cattr_verify(arglex_value.alv_string, cattr_data);
			break;

		case arglex_token_project:
			if (project_name)
				goto duplicate;
			if (arglex() != arglex_token_string)
				new_change_usage();
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_edit:
			if (edit)
			{
				duplicate:
				fatal
				(
					"duplicate \"%s\" option",
					arglex_value.alv_string
				);
			}
			edit++;
			break;
		}
		arglex();
	}
	if (!edit && !cattr_data)
	{
		error("warning: no -File specified, assuming -Edit desired");
		++edit;
	}

	/*
	 * locate project data
	 */
	if (!project_name)
		fatal("project name must be stated explicitly");
	pp = project_alloc(project_name);
	str_free(project_name);
	project_bind_existing(pp);

	/*
	 * locate user data
	 */
	up = user_executing(pp);
	
	/*
	 * see if must invoke editor
	 */
	if (edit)
	{

		/*
		 * make sure they are allowed to,
		 * to avoid a wasted edit
		 */
		check_permissions(pp, up);

		/*
		 * build template cattr
		 */
		if (!cattr_data)
		{
			string_ty *none = str_from_c("none");
			cattr_data = (cattr)cattr_type.alloc();
			cattr_data->brief_description = str_copy(none);
			cattr_data->description = str_copy(none);
			cattr_data->cause = change_cause_internal_bug;
			str_free(none);
		}

		/*
		 * default a few things
		 *	(create a fake change to extract the pconf)
		 */
		trace(("mark\n"));
		pstate_data = project_pstate_get(pp);
		cp = change_alloc(pp, pstate_data->next_change_number);
		change_bind_new(cp);
		cstate_data = change_cstate_get(cp);
		cstate_data->state = cstate_state_awaiting_development;
		pconf_data = change_pconf_get(cp, 0);
		cattr_defaults(cattr_data, pstate_data, pconf_data);
		change_free(cp);

		/*
		 * edit the attributes
		 */
		cattr_edit(&cattr_data);
	}

	/*
	 * Lock the project state file.
	 * Block if necessary.
	 */
	trace(("mark\n"));
	project_pstate_lock_prepare(pp);
	lock_take();
	pstate_data = project_pstate_get(pp);

	/*
	 * make sure they are allowed to
	 * (even if edited, may have changed while editing)
	 */
	check_permissions(pp, up);

	/*
	 * Add another row to the change table.
	 */
	trace(("mark\n"));
	assert(pstate_data->next_change_number >= 1);
	change_number = pstate_data->next_change_number++;
	cp = change_alloc(pp, change_number);
	change_bind_new(cp);
	cstate_data = change_cstate_get(cp);
	cstate_data->state = cstate_state_awaiting_development;
	pconf_data = change_pconf_get(cp, 0);
	if (change_number == 1)
	{
		cattr_data->cause = change_cause_internal_enhancement;
		cattr_data->test_baseline_exempt = 1;
		cattr_data->mask |= cattr_test_baseline_exempt_mask;
		cattr_data->regression_test_exempt = 1;
		cattr_data->mask |= cattr_regression_test_exempt_mask;
	}
	cattr_defaults(cattr_data, pstate_data, pconf_data);

	/*
	 * when developers create changes,
	 * they may not give themselves a testing exemption,
	 * or a architecture exemption,
	 * only administrators may do that.
	 */
	trace(("mark\n"));
	if (!project_administrator_query(pp, user_name(up)))
	{
		/*
		 * If they are asking for default behaviour, don't complain.
		 * (e.g. admin may have given general testing exemption)
		 */
		cattr dflt = cattr_type.alloc();
		dflt->cause = cattr_data->cause;
		cattr_defaults(dflt, pstate_data, pconf_data);

		if
		(
			(
				(cattr_data->mask & cattr_test_exempt_mask)
			&&
				cattr_data->test_exempt
			&&
				(cattr_data->test_exempt != dflt->test_exempt)
			)
		||
			(
				(
					cattr_data->mask
				&
					cattr_test_baseline_exempt_mask
				)
			&&
				cattr_data->test_baseline_exempt
			&&
				(
					cattr_data->test_baseline_exempt
				!=
					dflt->test_baseline_exempt
				)
			)
		||
			(
				(
					cattr_data->mask
				&
					cattr_regression_test_exempt_mask
				)
			&&
				cattr_data->regression_test_exempt
			&&
				(
					cattr_data->regression_test_exempt
				!=
					dflt->regression_test_exempt
				)
			)
		)
			fatal("developers may not grant testing exemptions");
		assert(cattr_data->architecture);
		assert(cattr_data->architecture->length);
		assert(dflt->architecture);
		assert(dflt->architecture->length);

		wl_zero(&carch);
		for (j = 0; j < cstate_data->architecture->length; ++j)
			wl_append(&carch, cstate_data->architecture->list[j]);
		wl_zero(&darch);
		for (j = 0; j < dflt->architecture->length; ++j)
			wl_append(&darch, dflt->architecture->list[j]);
		if (!wl_equal(&carch, &darch))
		{
			fatal
			(
			      "developers may not grant architecture exemptions"
			);
		}
		wl_free(&carch);
		wl_free(&darch);
		cattr_type.free(dflt);
	}

	/*
	 * make sure the architecture list only covers
	 * variations in the project's architecture list
	 */
	assert(cattr_data->architecture);
	wl_zero(&carch);
	for (j = 0; j < cattr_data->architecture->length; ++j)
		wl_append(&carch, cattr_data->architecture->list[j]);
	assert(pconf_data->architecture);
	wl_zero(&parch);
	for (j = 0; j < pconf_data->architecture->length; ++j)
		wl_append(&parch, pconf_data->architecture->list[j]->name);
	if (!wl_subset(&carch, &parch))
		fatal("architecture includes variations not in project");
	wl_free(&carch);
	wl_free(&parch);

	/*
	 * set change state from the attributes
	 * Create the change history.
	 */
	trace(("mark\n"));
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_new_change;
	if (cattr_data->description)
		cstate_data->description = str_copy(cattr_data->description);
	assert(cattr_data->brief_description);
	cstate_data->brief_description =
		str_copy(cattr_data->brief_description);
	assert(cattr_data->mask & cattr_cause_mask);
	cstate_data->cause = cattr_data->cause;
	assert(cattr_data->mask & cattr_test_exempt_mask);
	cstate_data->test_exempt = cattr_data->test_exempt;
	assert(cattr_data->mask & cattr_test_baseline_exempt_mask);
	cstate_data->test_baseline_exempt =
		cattr_data->test_baseline_exempt;
	assert(cattr_data->mask & cattr_regression_test_exempt_mask);
	cstate_data->regression_test_exempt =
		cattr_data->regression_test_exempt;
	cstate_data->architecture =
		(cstate_architecture_list)cstate_architecture_list_type.alloc();
	for (j = 0; j < cattr_data->architecture->length; ++j)
	{
		type_ty		*type_p;
		string_ty	**str_p;

		str_p =
			cstate_architecture_list_type.list_parse
			(
				cstate_data->architecture,
				&type_p
			);
		assert(type_p == &string_type);
		*str_p = str_copy(cattr_data->architecture->list[j]);
	}
	cattr_type.free(cattr_data);

	/*
	 * Write out the change file.
	 * There is no need to lock this file
	 * as it does not exist yet;
	 * the project state file, with the number in it, is locked.
	 */
	trace(("mark\n"));
	change_cstate_write(cp);

	/*
	 * Add the change to the list of existing changes.
	 * Incriment the next_change_number.
	 * and write pstate back out.
	 */
	trace(("mark\n"));
	project_change_append(pp, change_number);

	/*
	 * Unlock the pstate file.
	 */
	trace(("mark\n"));
	project_pstate_write(pp);
	commit();
	lock_release();

	/*
	 * verbose success message
	 */
	trace(("mark\n"));
	change_verbose(cp, "created");
	project_free(pp);
	change_free(cp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
new_change()
{
	trace(("new_change()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		new_change_main();
		break;

	case arglex_token_help:
		new_change_help();
		break;

	case arglex_token_list:
		new_change_list();
		break;
	}
	trace((/*{*/"}\n"));
}
