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
 * MANIFEST: functions to list and modify change attributes
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/libintl.h>

#include <aeca.h>
#include <arglex2.h>
#include <cattr.h>
#include <commit.h>
#include <change.h>
#include <error.h>
#include <help.h>
#include <io.h>
#include <lock.h>
#include <progname.h>
#include <project.h>
#include <project_hist.h>
#include <os.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>
#include <str_list.h>


static void change_attributes_usage _((void));

static void
change_attributes_usage()
{
	char		*progname;

	progname = progname_get();
	fprintf
	(
		stderr,
		"usage: %s -Change_Attributes -File <attr-file> [ <option>... ]\n",
		progname
	);
	fprintf
	(
		stderr,
		"       %s -Change_Attributes -Edit [ <option>... ]\n",
		progname
	);
	fprintf
	(
		stderr,
		"       %s -Change_Attributes -List [ <option>... ]\n",
		progname
	);
	fprintf(stderr, "       %s -Change_Attributes -Help\n", progname);
	quit(1);
}


static void change_attributes_help _((void));

static void
change_attributes_help()
{
	help("aeca", change_attributes_usage);
}


static void cattr_copy _((cattr, cstate));

static void
cattr_copy(a, s)
	cattr	a;
	cstate	s;
{
	trace(("cattr_copy()\n{\n"/*}*/));
	if (!a->description && s->description)
	{
		a->description = str_copy(s->description);
		a->mask |= cattr_description_mask;
	}
	if (!a->brief_description && s->brief_description)
	{
		a->brief_description = str_copy(s->brief_description);
		a->mask |= cattr_brief_description_mask;
	}
	if (!(a->mask & cattr_cause_mask))
	{
		a->cause = s->cause;
		a->mask |= cattr_cause_mask;
	}
	if (!(a->mask & cattr_regression_test_exempt_mask))
	{
		a->regression_test_exempt = s->regression_test_exempt;
		a->mask |= cattr_regression_test_exempt_mask;
	}
	if (!(a->mask & cattr_test_exempt_mask))
	{
		a->test_exempt = s->test_exempt;
		a->mask |= cattr_test_exempt_mask;
	}
	if (!(a->mask & cattr_test_baseline_exempt_mask))
	{
		a->test_baseline_exempt = s->test_baseline_exempt;
		a->mask |= cattr_test_baseline_exempt_mask;
	}

	if (!a->architecture)
		a->architecture =
			(cattr_architecture_list)
			cattr_architecture_list_type.alloc();
	if (!a->architecture->length)
	{
		long		j;

		for (j = 0; j < s->architecture->length; ++j)
		{
			type_ty		*type_p;
			string_ty	**str_p;

			str_p =
				cattr_architecture_list_type.list_parse
				(
					a->architecture,
					&type_p
				);
			assert(type_p == &string_type);
			*str_p = str_copy(s->architecture->list[j]);
		}
	}

	if (s->copyright_years)
	{
		if (!a->copyright_years)
			a->copyright_years =
				(cattr_copyright_years_list)
				cattr_copyright_years_list_type.alloc();
		if (!a->copyright_years->length)
		{
			long		j;

			for (j = 0; j < s->copyright_years->length; ++j)
			{
				type_ty		*type_p;
				long		*int_p;

				int_p =
					cattr_copyright_years_list_type.list_parse
					(
						a->copyright_years,
						&type_p
					);
				assert(type_p == &integer_type);
				*int_p = s->copyright_years->list[j];
			}
		}
	}
	if (!a->version_previous && s->version_previous)
	{
		a->version_previous = str_copy(s->version_previous);
		a->mask |= cattr_version_previous_mask;
	}
	trace((/*{*/"}\n"));
}


static void change_attributes_list _((void));

static void
change_attributes_list()
{
	string_ty	*project_name;
	project_ty	*pp;
	cattr		cattr_data;
	cstate		cstate_data;
	long		change_number;
	change_ty	*cp;
	user_ty		*up;

	trace(("change_attributes_list()\n{\n"/*}*/));
	arglex();
	project_name = 0;
	change_number = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(change_attributes_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				option_needs_number(arglex_token_change, change_attributes_usage);
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				duplicate_option_by_name(arglex_token_change, change_attributes_usage);
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
			if (project_name)
				duplicate_option(change_attributes_usage);
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_project, change_attributes_usage);
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
	 * build the cattr data
	 */
	cstate_data = change_cstate_get(cp);
	cattr_data = (cattr)cattr_type.alloc();
	cattr_copy(cattr_data, cstate_data);

	/*
	 * print the cattr data
	 */
	cattr_write_file((char *)0, cattr_data, 0);
	cattr_type.free(cattr_data);
	project_free(pp);
	change_free(cp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
cattr_verify(fn, d)
	char		*fn;
	cattr		d;
{
	if (!d->brief_description)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%s", fn);
		sub_var_set(scp, "FieLD_Name", "brief_description");
		fatal_intl(scp, i18n("$filename: contains no \"$field_name\" field"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	if (!(d->mask & cattr_cause_mask))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%s", fn);
		sub_var_set(scp, "FieLD_Name", "cause");
		fatal_intl(scp, i18n("$filename: contains no \"$field_name\" field"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}
}


void
cattr_edit(dp, et)
	cattr		*dp;
	int		et;
{
	sub_context_ty	*scp;
	cattr		d;
	string_ty	*filename;
	string_ty	*msg;

	/*
	 * write attributes to temporary file
	 */
	d = *dp;
	assert(d);
	filename = os_edit_filename(1);
	os_become_orig();
	cattr_write_file(filename->str_text, d, 0);
	cattr_type.free(d);

	/*
	 * an error message to issue if anything goes wrong
	 */
	scp = sub_context_new();
	sub_var_set(scp, "File_Name", "%S", filename);
	msg = subst_intl(scp, i18n("attributes in $filename"));
	sub_context_delete(scp);
	undo_message(msg);
	str_free(msg);

	/*
	 * edit the file
	 */
	os_edit(filename, et);

	/*
	 * read it in again
	 */
	d = cattr_read_file(filename->str_text);
	commit_unlink_errok(filename);
	os_become_undo();
	cattr_verify(filename->str_text, d);
	str_free(filename);
	*dp = d;
}


static void check_permissions _((change_ty *, user_ty *));

static void
check_permissions(cp, up)
	change_ty	*cp;
	user_ty		*up;
{
	project_ty	*pp;
	cstate		cstate_data;

	pp = cp->pp;
	cstate_data = change_cstate_get(cp);

	if
	(
		!project_administrator_query(pp, user_name(up))
	&&
		(
			cstate_data->state != cstate_state_being_developed
		||
			!str_equal(change_developer_name(cp), user_name(up))
		)
	)
	{
		change_fatal(cp, 0, i18n("bad ca, not auth"));
	}
}


static void change_attributes_main _((void));

static void
change_attributes_main()
{
	sub_context_ty	*scp;
	string_ty	*project_name;
	project_ty	*pp;
	cattr		cattr_data = 0;
	cstate		cstate_data;
	pconf		pconf_data;
	long		change_number;
	change_ty	*cp;
	user_ty		*up;
	edit_ty		edit;

	trace(("change_attributes_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	edit = edit_not_set;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(change_attributes_usage);
			continue;

		case arglex_token_string:
			scp = sub_context_new();
			sub_var_set(scp, "Name", "%s", arglex_token_name(arglex_token_file));
			error_intl(scp, i18n("warning: use $name option"));
			sub_context_delete(scp);
			if (cattr_data)
				fatal_intl(0, i18n("too many files"));
			goto read_attr_file;

		case arglex_token_file:
			if (cattr_data)
				duplicate_option(change_attributes_usage);
			if (arglex() != arglex_token_string)
				option_needs_file(arglex_token_file, change_attributes_usage);
			read_attr_file:
			os_become_orig();
			cattr_data = cattr_read_file(arglex_value.alv_string);
			os_become_undo();
			assert(cattr_data);
			break;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				option_needs_number(arglex_token_change, change_attributes_usage);
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				duplicate_option_by_name(arglex_token_change, change_attributes_usage);
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
			if (project_name)
				duplicate_option(change_attributes_usage);
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_project, change_attributes_usage);
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_edit:
			if (edit == edit_foreground)
				duplicate_option(change_attributes_usage);
			if (edit != edit_not_set)
			{
				too_many_edits:
				mutually_exclusive_options
				(
					arglex_token_edit,
					arglex_token_edit_bg,
					change_attributes_usage
				);
			}
			edit = edit_foreground;
			break;

		case arglex_token_edit_bg:
			if (edit == edit_background)
				duplicate_option(change_attributes_usage);
			if (edit != edit_not_set)
				goto too_many_edits;
			edit = edit_background;
			break;

		case arglex_token_wait:
		case arglex_token_wait_not:
			user_lock_wait_argument(change_attributes_usage);
			break;
		}
		arglex();
	}
	if (!cattr_data && edit == edit_not_set)
	{
		scp = sub_context_new();
		sub_var_set(scp, "Name1", "%s", arglex_token_name(arglex_token_file));
		sub_var_set(scp, "Name2", "%s", arglex_token_name(arglex_token_edit));
		error_intl(scp, i18n("warning: no $name1, assuming $name2"));
		sub_context_delete(scp);
		edit = edit_foreground;
	}
	if (edit != edit_not_set && !cattr_data)
		cattr_data = (cattr)cattr_type.alloc();

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
	 * edit the attributes
	 */
	if (edit != edit_not_set)
	{
		/*
		 * make sure they are allowed to,
		 * to avoid a wasted edit
		 */
		check_permissions(cp, up);

		/*
		 * fill in any other fields
		 */
		cstate_data = change_cstate_get(cp);
		cattr_copy(cattr_data, cstate_data);

		/*
		 * edit the attributes
		 */
		scp = sub_context_new();
		sub_var_set(scp, "Name", "%S", project_name_get(pp));
		sub_var_set(scp, "Number", "%ld", magic_zero_decode(change_number));
		io_comment_append(scp, i18n("Project $name, Change $number"));
		sub_context_delete(scp);
		cattr_edit(&cattr_data, edit);
	}

	/*
	 * lock the change
	 */
	change_cstate_lock_prepare(cp);
	lock_take();
	cstate_data = change_cstate_get(cp);
	pconf_data = change_pconf_get(cp, 0);

	/*
	 * make sure they are allowed to
	 * (even if edited, could have changed during edit)
	 */
	check_permissions(cp, up);

	/*
	 * copy the attributes across
	 */
	if (cattr_data->description)
	{
		if (cstate_data->description)
			str_free(cstate_data->description);
		cstate_data->description = str_copy(cattr_data->description);
	}
	if (cattr_data->brief_description)
	{
		if (cstate_data->brief_description)
			str_free(cstate_data->brief_description);
		cstate_data->brief_description =
			str_copy(cattr_data->brief_description);
	}
	if (cattr_data->mask & cattr_cause_mask)
		cstate_data->cause = cattr_data->cause;
	if (project_administrator_query(pp, user_name(up)))
	{
		if (cattr_data->mask & cattr_test_exempt_mask)
			cstate_data->test_exempt = cattr_data->test_exempt;
		if (cattr_data->mask & cattr_test_baseline_exempt_mask)
			cstate_data->test_baseline_exempt =
				cattr_data->test_baseline_exempt;
		if (cattr_data->mask & cattr_regression_test_exempt_mask)
			cstate_data->regression_test_exempt =
				cattr_data->regression_test_exempt;
	}
	else
	{
		if
		(
			(
				cattr_data->test_exempt
			&&
				!cstate_data->test_exempt
			)
		||
			(
				cattr_data->test_baseline_exempt
			&&
				!cstate_data->test_baseline_exempt
			)
		||
			(
				cattr_data->regression_test_exempt
			&&
				!cstate_data->regression_test_exempt
			)
		)
		{
			change_fatal(cp, 0, i18n("bad ca, no test exempt"));
		}
		else
		{
			/*
			 * developers may remove exemptions
			 */
			if
			(
				(cattr_data->mask & cattr_test_exempt_mask)
			&&
				!cattr_data->test_exempt
			)
				cstate_data->test_exempt = 0;
			if
			(
				(
					cattr_data->mask
				&
					cattr_test_baseline_exempt_mask
				)
			&&
				!cattr_data->test_baseline_exempt
			)
				cstate_data->test_baseline_exempt = 0;
			if
			(
				(
					cattr_data->mask
				&
					cattr_regression_test_exempt_mask
				)
			&&
				!cattr_data->regression_test_exempt
			)
				cstate_data->regression_test_exempt = 0;
		}
	}

	/*
	 * copy the architecture across
	 */
	if (cattr_data->architecture && cattr_data->architecture->length)
	{
		string_list_ty		caarch;
		string_list_ty		pcarch;
		long		j;

		/*
		 * make sure they did not name architectures
		 * we have never heard of
		 */
		string_list_constructor(&caarch);
		for (j = 0; j < cattr_data->architecture->length; ++j)
			string_list_append(&caarch, cattr_data->architecture->list[j]);

		string_list_constructor(&pcarch);
		assert(pconf_data->architecture);
		assert(pconf_data->architecture->length);
		for (j = 0; j < pconf_data->architecture->length; ++j)
		{
			string_list_append
			(
				&pcarch,
				pconf_data->architecture->list[j]->name
			);
		}

		if (!string_list_subset(&caarch, &pcarch))
			fatal_intl(0, i18n("bad ca, unknown architecture"));
		string_list_destructor(&pcarch);

		/*
		 * developers may remove architecture exemptions
		 * but may not grant them
		 */
		if (!project_administrator_query(pp, user_name(up)))
		{
			string_list_ty		csarch;

			string_list_constructor(&csarch);
			for (j = 0; j < cstate_data->architecture->length; ++j)
			{
				string_list_append
				(
					&csarch,
					cstate_data->architecture->list[j]
				);
			}

			if (!string_list_subset(&csarch, &caarch))
				fatal_intl(0, i18n("bad ca, no arch exempt"));
			string_list_destructor(&csarch);
		}
		string_list_destructor(&caarch);

		/*
		 * copy the architecture names across
		 */
		change_architecture_clear(cp);
		for (j = 0; j < cattr_data->architecture->length; ++j)
		{
			change_architecture_add
			(
				cp,
				cattr_data->architecture->list[j]
			);
		}
	}

	/*
	 * copy the copyright years list across
	 */
	if (cattr_data->copyright_years && cattr_data->copyright_years->length)
	{
		long	j;

		if (cstate_data->copyright_years)
			cstate_copyright_years_list_type.free(cstate_data->copyright_years);
		cstate_data->copyright_years =
			cstate_copyright_years_list_type.alloc();
		for (j = 0; j < cattr_data->copyright_years->length; ++j)
		{
			type_ty		*type_p;
			long		*int_p;

			int_p =
				cstate_copyright_years_list_type.list_parse
				(
					cstate_data->copyright_years,
					&type_p
				);
			assert(type_p == &integer_type);
			*int_p = cattr_data->copyright_years->list[j];
		}
	}

	/*
	 * copy the previous version across
	 */
	if (cattr_data->version_previous)
	{
		if (cstate_data->version_previous)
			str_free(cstate_data->version_previous);
		cstate_data->version_previous =
			str_copy(cattr_data->version_previous);
	}

	cattr_type.free(cattr_data);
	change_cstate_write(cp);
	commit();
	lock_release();
	change_verbose(cp, 0, i18n("attributes changed"));
	project_free(pp);
	change_free(cp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
change_attributes()
{
	trace(("change_attributes()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		change_attributes_main();
		break;

	case arglex_token_help:
		change_attributes_help();
		break;

	case arglex_token_list:
		change_attributes_list();
		break;
	}
	trace((/*{*/"}\n"));
}
