/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994 Peter Miller.
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
 * MANIFEST: functions to manipulate changes
 */

#include <aer/expr.h>
#include <aer/func/change.h>
#include <aer/func/project.h>
#include <aer/value/boolean.h>
#include <aer/value/integer.h>
#include <aer/value/string.h>
#include <error.h>
#include <project.h>
#include <user.h>

static long	change_number;
static int	change_number_set;


static void grab _((void));

static void
grab()
{
	rpt_value_ty	*vp;
	string_ty	*project_name;
	project_ty	*pp;
	user_ty		*up;

	assert(!change_number);
	assert(!change_number_set);

	/*
	 * find the project name
	 *	(this could be done nicer)
	 */
	vp = rpt_func_project_name.run((rpt_expr_ty *)0, 0, (rpt_value_ty **)0);
	project_name = rpt_value_string_query(vp);

	/*
	 * find the project
	 */
	pp = project_alloc(project_name);
	project_bind_existing(pp);

	/*
	 * allocate the user
	 */
	up = user_executing(pp);

	/*
	 * get the default change number
	 */
	change_number = user_default_change(up);

	/*
	 * clean up
	 */
	user_free(up);
	project_free(pp);
	rpt_value_free(vp);
}


static int change_number_valid _((rpt_expr_ty *));

static int
change_number_valid(ep)
	rpt_expr_ty	*ep;
{
	return (ep->nchild == 0);
}


static rpt_value_ty *change_number_run _((rpt_expr_ty *, size_t,
	rpt_value_ty **));

static rpt_value_ty *
change_number_run(ep, argc, argv)
	rpt_expr_ty	*ep;
	size_t		argc;
	rpt_value_ty	**argv;
{
	assert(argc == 0);
	if (!change_number && !change_number_set)
		grab();
	return rpt_value_integer(change_number);
}


rpt_func_ty rpt_func_change_number =
{
	"change_number",
	0, /* not optimizable */
	change_number_valid,
	change_number_run,
};


static int change_number_set_valid _((rpt_expr_ty *));

static int
change_number_set_valid(ep)
	rpt_expr_ty	*ep;
{
	return (ep->nchild == 0);
}


static rpt_value_ty *change_number_set_run _((rpt_expr_ty *, size_t,
	rpt_value_ty **));

static rpt_value_ty *
change_number_set_run(ep, argc, argv)
	rpt_expr_ty	*ep;
	size_t		argc;
	rpt_value_ty	**argv;
{
	assert(argc == 0);
	return rpt_value_boolean(change_number_set);
}


rpt_func_ty rpt_func_change_number_set =
{
	"change_number_set",
	0, /* not optimizable */
	change_number_set_valid,
	change_number_set_run,
};


void
report_parse_change_set(n)
	long		n;
{
	assert(!n);
	assert(n > 0);
	change_number = n;
	change_number_set = 1;
}
