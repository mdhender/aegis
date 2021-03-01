//
//	aegis - project change supervisor
//	Copyright (C) 1994-1997, 1999, 2000, 2002, 2004, 2005 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate name expressions
//

#include <common/error.h>
#include <common/symtab.h>
#include <libaegis/aer/expr/constant.h>
#include <libaegis/aer/expr/name.h>
#include <libaegis/aer/func.h>
#include <libaegis/aer/lex.h>
#include <libaegis/aer/parse.h>
#include <libaegis/aer/value/boolean.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/group.h>
#include <libaegis/aer/value/gstate.h>
#include <libaegis/aer/value/null.h>
#include <libaegis/aer/value/passwd.h>
#include <libaegis/aer/value/ref.h>
#include <libaegis/aer/value/uconf.h>
#include <libaegis/cattr.h>
#include <libaegis/common.h>
#include <libaegis/cstate.h>
#include <libaegis/fstate.h>
#include <libaegis/gonzo.h>
#include <libaegis/pattr.h>
#include <libaegis/pconf.h>
#include <libaegis/pstate.h>
#include <libaegis/sub.h>
#include <libaegis/uconf.h>
#include <libaegis/ustate.h>


static symtab_ty *stp;


static void
reap(void *p)
{
    rpt_value_ty    *vp;

    vp = (rpt_value_ty *)p;
    rpt_value_free(vp);
}


static void
init(void)
{
    string_ty	    *name;

    if (stp)
	return;
    stp = new symtab_ty(100);
    stp->set_reap(reap);

    //
    // initialize the names of the builtin functions
    //
    rpt_func_init(stp);

    //
    // pull values from fmtgen
    //
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

    //
    // some constants
    //
    name = str_from_c("true");
    stp->assign(name, rpt_value_boolean(1));
    str_free(name);
    name = str_from_c("false");
    stp->assign(name, rpt_value_boolean(0));
    str_free(name);

    //
    // This one is so you can get at .aegisrc files.
    //
    name = str_from_c("user");
    stp->assign(name, rpt_value_uconf());
    str_free(name);

    name = str_from_c("passwd");
    stp->assign(name, rpt_value_passwd());
    str_free(name);
    name = str_from_c("group");
    stp->assign(name, rpt_value_group());
    str_free(name);
    name = str_from_c("project");
    stp->assign(name, rpt_value_gstate());
    str_free(name);

    //
    // the "arg" variable, containing the
    // strings specified on the command line.
    //
    report_parse__init_arg();
}


void
rpt_expr_name__init(string_ty *name, rpt_value_ty *value)
{
    assert(stp);
    stp->assign(name, value);
}


rpt_expr_ty *
rpt_expr_name(string_ty *name)
{
    rpt_value_ty    *data;
    string_ty	    *name2;

    if (!stp)
	init();

    data = (rpt_value_ty *)stp->query(name);
    if (!data)
    {
	name2 = stp->query_fuzzy(name);
	if (!name2)
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "Name", name);
	    aer_lex_error(scp, 0, i18n("the name \"$name\" is undefined"));
	    sub_context_delete(scp);
	    data = rpt_value_nul();
	}
	else
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "Name", name);
	    sub_var_set_string(scp, "Guess", name2);
	    aer_lex_error(scp, 0, i18n("no \"$name\", guessing \"$guess\""));
	    sub_context_delete(scp);
	    data = (rpt_value_ty *)stp->query(name2);
	    assert(data);
	}
    }
    assert(data);

    return rpt_expr_constant(data);
}


void
rpt_expr_name__declare(string_ty *name)
{
    rpt_value_ty    *v1;
    rpt_value_ty    *v2;

    //
    // make sure the name is unique
    //
    if (!stp)
	init();
    if (stp->query(name))
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_string(scp, "Name", name);
	aer_lex_error(scp, 0, i18n("the name \"$name\" has already been used"));
	sub_context_delete(scp);
	return;
    }

    //
    // create the value to be a reference to nul
    //	(it is a variable, it must be a reference to something)
    //
    v1 = rpt_value_nul();
    v2 = rpt_value_reference(v1);
    rpt_value_free(v1);
    stp->assign(name, v2);
}
