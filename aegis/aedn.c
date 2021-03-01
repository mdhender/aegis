/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994 Peter Miller;
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
 * MANIFEST: functions to implement the 'aegis -Delta_Name' command
 */

#include <stdio.h>

#include <aedn.h>
#include <ael.h>
#include <arglex2.h>
#include <commit.h>
#include <error.h>
#include <help.h>
#include <lock.h>
#include <option.h>
#include <project.h>
#include <pstate.h>
#include <user.h>
#include <trace.h>


static void delta_name_usage _((void));

static void
delta_name_usage()
{
	char		*progname;

	progname = option_progname_get();
	fprintf
	(
		stderr,
	"usage: %s -Delta_Name [ <option>... ][ <delta_number> ] <string>...\n",
		progname
	);
	fprintf
	(
		stderr,
		"       %s -Delta_Name -List [ <option>... ]\n",
		progname
	);
	fprintf(stderr, "       %s -Delta_Name -Help\n", progname);
	quit(1);
}


static void delta_name_help _((void));

static void
delta_name_help()
{
	static char *text[] =
	{
#include <../man1/aedn.h>
	};

	help(text, SIZEOF(text), delta_name_usage);
}


static void delta_name_list _((void));

static void
delta_name_list()
{
	string_ty	*project_name;
	long		change_number;

	trace(("delta_name_list()\n{\n"/*}*/));
	arglex();
	project_name = 0;
	change_number = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(delta_name_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				delta_name_usage();
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
				delta_name_usage();
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	list_project_history(project_name, change_number);
	if (project_name)
		str_free(project_name);
	trace((/*{*/"}\n"));
}


static void delta_name_main _((void));

static void
delta_name_main()
{
	string_ty	*project_name;
	long		delta_number;
	string_ty	*delta_name;
	int		stomp;
	project_ty	*pp;
	user_ty		*up;
	pstate		pstate_data;
	pstate_history_list h;
	pstate_history	he;
	string_ty	**addr_p;
	type_ty		*type_p;
	size_t		j;

	trace(("delta_name_main()\n{\n"/*}*/));
	project_name = 0;
	delta_number = 0;
	delta_name = 0;
	stomp = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(delta_name_usage);
			continue;

		case arglex_token_overwriting:
			if (stomp)
			{
				duplicate:
				fatal
				(
					"duplicate %s option",
					arglex_value.alv_string
				);
			}
			stomp = 1;
			break;

		case arglex_token_string:
			if (delta_name)
			{
				error("too many delta names specified");
				delta_name_usage();
			}
			delta_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_number:
			if (delta_number)
			{
				error("too many delta numbers specified");
				delta_name_usage();
			}
			delta_number = arglex_value.alv_number;
			if (delta_number < 1)
				fatal("delta %ld out of range", delta_number);
			break;

		case arglex_token_project:
			if (project_name)
				goto duplicate;
			if (arglex() != arglex_token_string)
				delta_name_usage();
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	if (!delta_name)
		fatal("no delta name specified");

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
	 * lock the project file
	 */
	project_pstate_lock_prepare(pp);
	lock_take();
	pstate_data = project_pstate_get(pp);
	h = pstate_data->history;

	/*
	 * it is an error if the user is not a project administrator
	 */
	if (!project_administrator_query(pp, user_name(up)))
	{
		project_fatal
		(
			pp,
			"user \"%S\" is not an administrator",
			user_name(up)
		);
	}

	/*
	 * it is an error if the delta does not exist
	 */
	he = 0;
	if (delta_number)
	{
		for (j = 0; j < h->length; ++j)
		{
			if (h->list[j]->delta_number == delta_number)
			{
				he = h->list[j];
				break;
			}
		}
		if (!he)
			project_fatal(pp, "delta number %ld unknown", delta_number);
	}
	else
	{
		if (!h || !h->length)
		{
			project_fatal
			(
				pp,
"no integrations have been completed, \
so it is not yet possible to assign a delta name"
			);
		}
		he = h->list[h->length - 1];
		delta_number = he->delta_number;
	}

	/*
	 * make sure the name has not been used already
	 */
	for (j = 0; j < h->length; ++j)
	{
		pstate_history	he2;
		pstate_history_name_list nlp;
		size_t		k, m;

		he2 = h->list[j];
		nlp = he2->name;
		if (!nlp || !nlp->length)
			continue;
		for (k = 0; k < nlp->length; ++k)
		{
			/*
			 * see if the name matches
			 */
			if (!str_equal(nlp->list[k], delta_name))
				continue;

			/*
			 * it is an error if the name has been used and
			 * the -OverWriting option was not given
			 */
			if (!stomp && he2->delta_number != delta_number)
			{
				project_fatal
				(
					pp,
"the delta name \"%S\" is already assigned to delta number %ld; \
to reassign this name to delta number %ld \
you must use the -OverWriting option",
					delta_name,
					he2->delta_number,
					delta_number
				);
			}

			/*
			 * remove the name from the list
			 */
			str_free(nlp->list[k]);
			for (m = k + 1; m < nlp->length; ++m)
				nlp->list[m - 1] = nlp->list[m];
			nlp->length--;
			if (nlp->length == 0)
			{
				he2->name = 0;
				pstate_history_name_list_type.free(nlp);
				break;
			}
			k--;
		}
	}

	/*
	 * add the name to the selected history entry
	 */
	if (!he->name)
		he->name = pstate_history_name_list_type.alloc();
	addr_p = pstate_history_name_list_type.list_parse(he->name, &type_p);
	assert(type_p == &string_type);
	*addr_p = str_copy(delta_name);

	/*
	 * release the locks
	 */
	project_pstate_write(pp);
	commit();
	lock_release();

	/*
	 * verbose success message
	 */
	project_verbose
	(
		pp,
		"delta %ld may now be refered to as delta \"%S\"",
		delta_number,
		delta_name
	);
	project_free(pp);
	user_free(up);
	str_free(delta_name);
	trace((/*{*/"}\n"));
}


void
delta_name_assignment()
{
	trace(("delta_name()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		delta_name_main();
		break;

	case arglex_token_help:
		delta_name_help();
		break;

	case arglex_token_list:
		delta_name_list();
		break;
	}
	trace((/*{*/"}\n"));
}
