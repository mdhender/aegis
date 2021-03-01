/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 2003 Peter Miller.
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
 * MANIFEST: functions to impliment the builtin project function
 */

#include <aer/expr.h>
#include <aer/func/project.h>
#include <aer/value/boolean.h>
#include <aer/value/string.h>
#include <error.h>
#include <str.h>
#include <user.h>


static string_ty *project_name;
static int	project_name_set;


void
report_parse_project_set(string_ty *s)
{
	assert(!project_name);
	project_name = str_copy(s);
	project_name_set = 1;
}


static int
project_name_validate(rpt_expr_ty *ep)
{
	return (ep->nchild == 0);
}


static rpt_value_ty *
project_name_run(rpt_expr_ty *ep, size_t argc, rpt_value_ty **argv)
{
	assert(argc == 0);
	if (!project_name)
		project_name = user_default_project();
	return rpt_value_string(project_name);
}


rpt_func_ty rpt_func_project_name =
{
	"project_name",
	0, /* not foldable */
	project_name_validate,
	project_name_run,
};


static int
project_name_set_validate(rpt_expr_ty *ep)
{
	return (ep->nchild == 0);
}


static rpt_value_ty *
project_name_set_run(rpt_expr_ty *ep, size_t argc, rpt_value_ty **argv)
{
	assert(argc == 0);
	return rpt_value_boolean(project_name_set);
}


rpt_func_ty rpt_func_project_name_set =
{
	"project_name_set",
	0, /* not foldable */
	project_name_set_validate,
	project_name_set_run,
};
