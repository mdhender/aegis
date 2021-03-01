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
 * MANIFEST: functions to implement new change
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/time.h>

#include <aeca.h>
#include <aenc.h>
#include <ael.h>
#include <arglex2.h>
#include <cattr.h>
#include <change.h>
#include <change_bran.h>
#include <col.h>
#include <commit.h>
#include <common.h>
#include <error.h>
#include <file.h>
#include <help.h>
#include <io.h>
#include <lock.h>
#include <os.h>
#include <progname.h>
#include <project.h>
#include <project_hist.h>
#include <sub.h>
#include <trace.h>
#include <user.h>
#include <str_list.h>


static void new_change_usage _((void));

static void
new_change_usage()
{
	char	*progname;

	progname = progname_get();
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
	help("aenc", new_change_usage);
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
				option_needs_name(arglex_token_project, new_change_usage);
			if (project_name)
				duplicate_option_by_name(arglex_token_project, new_change_usage);
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


static void cattr_defaults _((cattr, project_ty *, pconf));

static void
cattr_defaults(a, pp, pc)
	cattr		a;
	project_ty	*pp;
	pconf		pc;
{
	trace(("cattr_defaults(a = %08lX, pp = %08lX, pc = %08lX)\n{\n"/*}*/,
		(long)a, (long)pp, (long)pc));
	if
	(
		a->cause == change_cause_internal_improvement
	||
		a->cause == change_cause_external_improvement
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
		a->test_exempt = project_default_test_exemption_get(pp);
		a->mask |= cattr_test_exempt_mask;
	}
	if (!(a->mask & cattr_test_baseline_exempt_mask))
	{
		a->test_baseline_exempt =
			project_default_test_exemption_get(pp);
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


void
new_change_check_permission(pp, up)
	project_ty	*pp;
	user_ty		*up;
{
	/*
	 * it is an error if
	 * the user is not an administrator for the project.
	 */
	if
	(
		!project_administrator_query(pp, user_name(up))
	&&
		(
			!project_developers_may_create_changes_get(pp)
		||
			!project_developer_query(pp, user_name(up))
		)
	)
	{
		project_fatal(pp, 0, i18n("not an administrator"));
	}
}


static void new_change_main _((void));

static void
new_change_main()
{
	sub_context_ty	*scp;
	cstate		cstate_data;
	cstate_history	history_data;
	cattr		cattr_data;
	string_ty	*project_name;
	project_ty	*pp;
	long		change_number;
	change_ty	*cp;
	user_ty		*up;
	edit_ty		edit;
	long		j;
	pconf		pconf_data;
	string_list_ty	carch;
	string_list_ty	darch;
	string_list_ty	parch;
	char		*output;

	trace(("new_change_main()\n{\n"/*}*/));
	cattr_data = 0;
	project_name = 0;
	edit = edit_not_set;
	change_number = 0;
	output = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(new_change_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				option_needs_number(arglex_token_change, new_change_usage);
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				duplicate_option_by_name(arglex_token_change, new_change_usage);
			change_number = arglex_value.alv_number;
			if (change_number == 0)
				change_number = MAGIC_ZERO;
			else if (change_number < 1)
			{
				scp = sub_context_new();
				sub_var_set(scp, "Number", "%ls", change_number);
				fatal_intl(scp, i18n("change $number out of range"));
				/* NOTREACHED */
				sub_context_delete(scp);
			}
			break;

		case arglex_token_string:
			scp = sub_context_new();
			sub_var_set(scp, "Name", "%s", arglex_token_name(arglex_token_file));
			error_intl(scp, i18n("warning: use $name option"));
			sub_context_delete(scp);
			if (cattr_data)
				fatal_intl(0, i18n("too many files"));
			goto read_input_file;

		case arglex_token_file:
			if (cattr_data)
				duplicate_option(new_change_usage);
			if (arglex() != arglex_token_string)
				option_needs_file(arglex_token_file, new_change_usage);
			read_input_file:
			os_become_orig();
			cattr_data = cattr_read_file(arglex_value.alv_string);
			os_become_undo();
			cattr_verify(arglex_value.alv_string, cattr_data);
			break;

		case arglex_token_project:
			if (project_name)
				duplicate_option(new_change_usage);
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_project, new_change_usage);
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_edit:
			if (edit == edit_foreground)
				duplicate_option(new_change_usage);
			if (edit != edit_not_set)
			{
				too_many_edits:
				mutually_exclusive_options
				(
					arglex_token_edit,
					arglex_token_edit_bg,
					new_change_usage
				);
			}
			edit = edit_foreground;
			break;

		case arglex_token_edit_bg:
			if (edit == edit_background)
				duplicate_option(new_change_usage);
			if (edit != edit_not_set)
				goto too_many_edits;
			edit = edit_background;
			break;

		case arglex_token_wait:
		case arglex_token_wait_not:
			user_lock_wait_argument(new_change_usage);
			break;

		case arglex_token_output:
			if (output)
				duplicate_option(new_change_usage);
			switch (arglex())
			{
			default:
				option_needs_file(arglex_token_output, new_change_usage);
				/* NOTREACHED */

			case arglex_token_string:
				output = arglex_value.alv_string;
				break;

			case arglex_token_stdio:
				output = "";
				break;
			}
			break;
		}
		arglex();
	}
	if (change_number && output)
	{
		mutually_exclusive_options
		(
			arglex_token_change,
			arglex_token_output,
			new_change_usage
		);
	}
	if (edit != edit_not_set && cattr_data)
	{
		mutually_exclusive_options
		(
			(
				edit == edit_foreground
			?
				arglex_token_edit
			:
				arglex_token_edit_bg
			),
			arglex_token_file,
			new_change_usage
		);
	}
	if (edit == edit_not_set && !cattr_data)
	{
		scp = sub_context_new();
		sub_var_set(scp, "Name1", "%s", arglex_token_name(arglex_token_file));
		sub_var_set(scp, "Name2", "%s", arglex_token_name(arglex_token_edit));
		error_intl(scp, i18n("warning: no $name1, assuming $name2"));
		sub_context_delete(scp);
		edit = edit_foreground;
	}

	/*
	 * locate project data
	 */
	if (!project_name)
		fatal_intl(0, i18n("no project name"));
	pp = project_alloc(project_name);
	str_free(project_name);
	project_bind_existing(pp);

	/*
	 * make sure this branch of the project is still active
	 */
	if (!change_is_a_branch(project_change_get(pp)))
		project_fatal(pp, 0, i18n("branch completed"));

	/*
	 * locate user data
	 */
	up = user_executing(pp);
	
	/*
	 * see if must invoke editor
	 */
	if (edit != edit_not_set)
	{
		/*
		 * make sure they are allowed to,
		 * to avoid a wasted edit
		 */
		new_change_check_permission(pp, up);

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
		cp = change_alloc(pp, TRUNK_CHANGE_NUMBER - 1);
		change_bind_new(cp);
		cstate_data = change_cstate_get(cp);
		cstate_data->state = cstate_state_awaiting_development;
		pconf_data = change_pconf_get(cp, 0);
		cattr_defaults(cattr_data, pp, pconf_data);
		change_free(cp);

		/*
		 * edit the attributes
		 */
		scp = sub_context_new();
		sub_var_set(scp, "Name", "%S", project_name_get(pp));
		io_comment_append(scp, i18n("Project $name"));
		io_comment_append(scp, i18n("nc dflt hint"));
		sub_context_delete(scp);
		cattr_edit(&cattr_data, edit);
	}

	/*
	 * Lock the project state file.
	 * Block if necessary.
	 */
	project_pstate_lock_prepare(pp);
	lock_take();

	/*
	 * make sure they are allowed to
	 * (even if edited, may have changed while editing)
	 */
	new_change_check_permission(pp, up);

	/*
	 * Add another row to the change table.
	 */
	if (!change_number)
		change_number = project_next_change_number(pp, 1);
	else
	{
		if (project_change_number_in_use(pp, change_number))
		{
			scp = sub_context_new();
			sub_var_set(scp, "Number", "%ld", magic_zero_decode(change_number));
			project_fatal(pp, scp, i18n("change $number used"));
			/* NOTREACHED */
			sub_context_delete(scp);
		}
	}
	cp = change_alloc(pp, change_number);
	change_bind_new(cp);
	cstate_data = change_cstate_get(cp);
	cstate_data->state = cstate_state_awaiting_development;
	pconf_data = change_pconf_get(cp, 0);
	if (!pconf_data->build_command)
	{
		/*
		 * There is no build command.  This means that the project
		 * does not yet have a ``config'' file.  From this,
		 * infer that this is the first change of the project.
		 *
		 * It has to be the first change because: aeb will fail
		 * when it can't find a build_command in the non-existent
		 * config file, so you can't aede until there is a
		 * valid config file, so you can't have a baseline with
		 * *anything* in it unless it also has a valid config
		 * file.  Ergo, there have been no integrations yet;
		 * we must be the first change.
		 *
		 * There could be a couple of changes created before one
		 * of them is sucessfully integrated, but it doesn't
		 * happen all that often, so I'm not going to worry
		 * about it.
		 */
		cattr_data->cause = change_cause_internal_enhancement;
		cattr_data->test_baseline_exempt = 1;
		cattr_data->mask |= cattr_test_baseline_exempt_mask;
		cattr_data->regression_test_exempt = 1;
		cattr_data->mask |= cattr_regression_test_exempt_mask;
	}
	cattr_defaults(cattr_data, pp, pconf_data);

	/*
	 * when developers create changes,
	 * they may not give themselves a testing exemption,
	 * or a architecture exemption,
	 * only administrators may do that.
	 *
	 * Don't worry about this stuff for the very first change of
	 * a project.
	 */
	if
	(
		pconf_data->build_command
	&&
		!project_administrator_query(pp, user_name(up))
	)
	{
		/*
		 * If they are asking for default behaviour, don't complain.
		 * (e.g. admin may have given general testing exemption)
		 */
		cattr dflt = cattr_type.alloc();
		dflt->cause = cattr_data->cause;
		cattr_defaults(dflt, pp, pconf_data);

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
			fatal_intl(0, i18n("bad ca, no test exempt"));
		assert(cattr_data->architecture);
		assert(cattr_data->architecture->length);
		assert(dflt->architecture);
		assert(dflt->architecture->length);

		string_list_constructor(&carch);
		for (j = 0; j < cattr_data->architecture->length; ++j)
			string_list_append(&carch, cattr_data->architecture->list[j]);
		string_list_constructor(&darch);
		for (j = 0; j < dflt->architecture->length; ++j)
			string_list_append(&darch, dflt->architecture->list[j]);
		if (!string_list_equal(&carch, &darch))
			fatal_intl(0, i18n("bad ca, no arch exempt"));
		string_list_destructor(&carch);
		string_list_destructor(&darch);
		cattr_type.free(dflt);
	}

	/*
	 * make sure the architecture list only covers
	 * variations in the project's architecture list
	 */
	assert(cattr_data->architecture);
	string_list_constructor(&carch);
	for (j = 0; j < cattr_data->architecture->length; ++j)
		string_list_append(&carch, cattr_data->architecture->list[j]);
	assert(pconf_data->architecture);
	string_list_constructor(&parch);
	for (j = 0; j < pconf_data->architecture->length; ++j)
		string_list_append(&parch, pconf_data->architecture->list[j]->name);
	if (!string_list_subset(&carch, &parch))
		fatal_intl(0, i18n("bad ca, unknown architecture"));
	string_list_destructor(&carch);
	string_list_destructor(&parch);

	/*
	 * set change state from the attributes
	 * Create the change history.
	 */
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
	change_architecture_clear(cp);
	for (j = 0; j < cattr_data->architecture->length; ++j)
		change_architecture_add(cp, cattr_data->architecture->list[j]);
	cattr_type.free(cattr_data);

	/*
	 * Write out the change file.
	 * There is no need to lock this file
	 * as it does not exist yet;
	 * the project state file, with the number in it, is locked.
	 */
	change_cstate_write(cp);

	/*
	 * Add the change to the list of existing changes.
	 * Increment the next_change_number.
	 * and write pstate back out.
	 */
	project_change_append(pp, change_number, 0);

	/*
	 * If there is an output option,
	 * write the change number to the file.
	 */
	if (output)
	{
		string_ty	*content;

		content = str_format("%ld", magic_zero_decode(change_number));
		if (*output)
		{
			string_ty	*fn;
			
			user_become(up);
			fn = str_from_c(output);
			file_from_string(fn, content, 0644);
			str_free(fn);
			user_become_undo();
		}
		else
			cat_string_to_stdout(content);
		str_free(content);
	}

	/*
	 * Unlock the pstate file.
	 */
	project_pstate_write(pp);
	commit();
	lock_release();

	/*
	 * verbose success message
	 */
	change_verbose(cp, 0, i18n("new change complete"));
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
