/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1995, 1996, 1997, 1999, 2000 Peter Miller;
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
 * MANIFEST: functions to manipulate name expressions
 */

#include <aer/expr/constant.h>
#include <aer/expr/name.h>
#include <aer/func.h>
#include <aer/lex.h>
#include <aer/parse.h>
#include <aer/value/boolean.h>
#include <aer/value/error.h>
#include <aer/value/group.h>
#include <aer/value/gstate.h>
#include <aer/value/null.h>
#include <aer/value/passwd.h>
#include <aer/value/ref.h>
#include <aer/value/uconf.h>
#include <cattr.h>
#include <common.h>
#include <cstate.h>
#include <error.h>
#include <fstate.h>
#include <gonzo.h>
#include <pattr.h>
#include <pconf.h>
#include <pstate.h>
#include <sub.h>
#include <symtab.h>
#include <uconf.h>
#include <ustate.h>


static symtab_ty *stp;


static void reap _((void *));

static void
reap(p)
	void		*p;
{
	rpt_value_ty	*vp;

	vp = p;
	rpt_value_free(vp);
}


static void init _((void));

static void
init()
{
	string_ty	*name;

	if (stp)
		return;
	stp = symtab_alloc(100);
	stp->reap = reap;

	/*
	 * initialize the names of the builtin functions
	 */
	rpt_func_init(stp);

	/*
	 * pull values from fmtgen
	 */
	cattr__rpt_init();
	common__rpt_init();
	cstate__rpt_init();
	fstate__rpt_init();
	gstate__rpt_init();
	pattr__rpt_init();
	pconf__rpt_init();
	pstate__rpt_init();
	uconf__rpt_init();
	ustate__rpt_init();

	/*
	 * some constants
	 */
	name = str_from_c("true");
	symtab_assign(stp, name, rpt_value_boolean(1));
	str_free(name);
	name = str_from_c("false");
	symtab_assign(stp, name, rpt_value_boolean(0));
	str_free(name);

	/*
	 * This one is so you can get at .aegisrc files.
	 */
	name = str_from_c("user");
	symtab_assign(stp, name, rpt_value_uconf());
	str_free(name);

	name = str_from_c("passwd");
	symtab_assign(stp, name, rpt_value_passwd());
	str_free(name);
	name = str_from_c("group");
	symtab_assign(stp, name, rpt_value_group());
	str_free(name);
	name = str_from_c("project");
	symtab_assign(stp, name, rpt_value_gstate());
	str_free(name);

	/*
	 * the ``arg'' variable, containing the
	 * strings specified on the command line.
	 */
	report_parse__init_arg();
}


void
rpt_expr_name__init(name, value)
	string_ty	*name;
	rpt_value_ty	*value;
{
	assert(stp);
	symtab_assign(stp, name, value);
}


rpt_expr_ty *
rpt_expr_name(name)
	string_ty	*name;
{
	rpt_value_ty	*data;
	string_ty	*name2;

	if (!stp)
		init();

	data = symtab_query(stp, name);
	if (!data)
	{
		name2 = symtab_query_fuzzy(stp, name);
		if (!name2)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set_string(scp, "Name", name);
			aer_lex_error(scp, 0, i18n("the name \"$name\" is undefined"));
			sub_context_delete(scp);
			data = rpt_value_nul();
		}
		else
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set_string(scp, "Name", name);
			sub_var_set_string(scp, "Guess", name2);
			aer_lex_error(scp, 0, i18n("no \"$name\", guessing \"$guess\""));
			sub_context_delete(scp);
			data = symtab_query(stp, name2);
			assert(data);
		}
	}
	assert(data);

	return rpt_expr_constant(data);
}


void
rpt_expr_name__declare(name)
	string_ty	*name;
{
	rpt_value_ty	*v1;
	rpt_value_ty	*v2;

	/*
	 * make sure the name is unique
	 */
	if (!stp)
		init();
	if (symtab_query(stp, name))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set_string(scp, "Name", name);
		aer_lex_error(scp, 0, i18n("the name \"$name\" has already been used"));
		sub_context_delete(scp);
		return;
	}

	/*
	 * create the value to be a reference to nul
	 *	(it is a variable, it must be a reference to something)
	 */
	v1 = rpt_value_nul();
	v2 = rpt_value_reference(v1);
	rpt_value_free(v1);
	symtab_assign(stp, name, v2);
}
