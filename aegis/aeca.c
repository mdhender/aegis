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
 * MANIFEST: functions to list and modify change attributes
 */

#include <stdio.h>
#include <ac/stdlib.h>

#include <aeca.h>
#include <arglex2.h>
#include <cattr.h>
#include <commit.h>
#include <change.h>
#include <error.h>
#include <help.h>
#include <lock.h>
#include <project.h>
#include <option.h>
#include <os.h>
#include <trace.h>
#include <undo.h>
#include <user.h>
#include <word.h>


static void change_attributes_usage _((void));

static void
change_attributes_usage()
{
	char		*progname;

	progname = option_progname_get();
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
	static char *text[] =
	{
#include <../man1/aeca.h>
	};

	help(text, SIZEOF(text), change_attributes_usage);
}


static void cattr_copy _((cattr, cstate));

static void
cattr_copy(a, s)
	cattr	a;
	cstate	s;
{
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
				change_attributes_usage();
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				fatal("duplicate -Change option");
			change_number = arglex_value.alv_number;
			if (change_number < 1)
				fatal("change %ld out of range", change_number);
			break;

		case arglex_token_project:
			if (project_name)
				fatal("duplicate -Project option");
			if (arglex() != arglex_token_string)
				change_attributes_usage();
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
	cattr_write_file((char *)0, cattr_data);
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
		fatal("%s: contains no \"brief_description\" field", fn);
	if (!(d->mask & cattr_cause_mask))
		fatal("%s: contains no \"cause\" field", fn);
}


void
cattr_edit(dp)
	cattr		*dp;
{
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
	cattr_write_file(filename->str_text, d);
	cattr_type.free(d);

	/*
	 * an error message to issue if anything goes wrong
	 */
	msg = str_format("attributes text left in the \"%S\" file", filename);
	undo_message(msg);
	str_free(msg);

	/*
	 * edit the file
	 */
	os_edit(filename);

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
		change_fatal
		(
			cp,
"attributes may only be changed by a project administrator, \
or by the developer during development"
		);
	}
}


static void change_attributes_main _((void));

static void
change_attributes_main()
{
	string_ty	*project_name;
	project_ty	*pp;
	cattr		cattr_data = 0;
	cstate		cstate_data;
	pconf		pconf_data;
	long		change_number;
	change_ty	*cp;
	user_ty		*up;
	int		edit;

	trace(("change_attributes_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	edit = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(change_attributes_usage);
			continue;

		case arglex_token_string:
			error
			(
"warning: please use the -File option when specifying an attributes file, \
the unadorned form is now obsolescent"
			);
			if (cattr_data)
				fatal("too many files named");
			goto read_attr_file;

		case arglex_token_file:
			if (cattr_data)
				goto duplicate;
			if (arglex() != arglex_token_string)
			{
				error
				(
				 "the -File option requires a filename argument"
				);
				change_attributes_usage();
			}
			read_attr_file:
			os_become_orig();
			cattr_data = cattr_read_file(arglex_value.alv_string);
			os_become_undo();
			assert(cattr_data);
			break;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				change_attributes_usage();
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				fatal("duplicate -Change option");
			change_number = arglex_value.alv_number;
			if (change_number < 1)
				fatal("change %ld out of range", change_number);
			break;

		case arglex_token_project:
			if (project_name)
				goto duplicate;
			if (arglex() != arglex_token_string)
				change_attributes_usage();
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
	if (!cattr_data && !edit)
	{
		error("warning: no -File specified, assuming -Edit desired");
		++edit;
	}
	if (edit && !cattr_data)
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
	if (edit)
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
		cattr_edit(&cattr_data);
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
	trace(("mark\n"));
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
			change_fatal
			(
				cp,
		   "only project administrators may exempt changes from testing"
			);
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
		wlist		caarch;
		wlist		pcarch;
		long		j;

		/*
		 * make sure they did not name architectures
		 * we have never heard of
		 */
		trace(("mark\n"));
		wl_zero(&caarch);
		for (j = 0; j < cattr_data->architecture->length; ++j)
			wl_append(&caarch, cattr_data->architecture->list[j]);

		wl_zero(&pcarch);
		assert(pconf_data->architecture);
		assert(pconf_data->architecture->length);
		for (j = 0; j < pconf_data->architecture->length; ++j)
		{
			wl_append
			(
				&pcarch,
				pconf_data->architecture->list[j]->name
			);
		}

		if (!wl_subset(&caarch, &pcarch))
		{
			fatal
			(
			      "architecture contains variations not in project"
			);
		}
		wl_free(&pcarch);

		/*
		 * developers may remove architecture exemptions
		 * but may not grant them
		 */
		trace(("mark\n"));
		if (!project_administrator_query(pp, user_name(up)))
		{
			wlist		csarch;

			wl_zero(&csarch);
			for (j = 0; j < cstate_data->architecture->length; ++j)
			{
				wl_append
				(
					&csarch,
					cstate_data->architecture->list[j]
				);
			}

			if (!wl_subset(&csarch, &caarch))
			{
				fatal
				(
			      "developers may not grant architecture exemptions"
				);
			}
			wl_free(&csarch);
		}
		trace(("mark\n"));
		wl_free(&caarch);

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

	cattr_type.free(cattr_data);
	change_cstate_write(cp);
	commit();
	lock_release();
	change_verbose(cp, "attributes changed");
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
