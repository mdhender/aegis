/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate executes
 */

#include <aer/value/boolean.h>
#include <aer/value/string.h>
#include <function/execute.h>
#include <function/needs.h>
#include <os.h>
#include <trace.h>
#include <tree/list.h>
#include <tree/monadic.h>


static rpt_value_ty *evaluate _((tree_ty *, string_ty *, struct stat *));

static rpt_value_ty *
evaluate(tp, path, st)
    tree_ty	    *tp;
    string_ty	    *path;
    struct stat	    *st;
{
    tree_monadic_ty *this;
    rpt_value_ty    *vp;
    rpt_value_ty    *svp;
    int		    exit_status;

    trace(("tree::execute::evaluate\n"));
    this = (tree_monadic_ty *)tp;
    vp = tree_evaluate(this->arg, path, st);
    svp = rpt_value_stringize(vp);
    rpt_value_free(vp);
    os_become_orig();
    exit_status =
	os_execute_retcode
	(
	    rpt_value_string_query(svp),
	    OS_EXEC_FLAG_SILENT,
	    0
	);
    os_become_undo();
    rpt_value_free(svp);
    return rpt_value_boolean(exit_status == 0);
}


static int useful _((tree_ty *));

static int
useful(tp)
    tree_ty	    *tp;
{
    trace(("tree::execute::useful\n"));
    return 1;
}


static tree_method_ty method =
{
    sizeof(tree_monadic_ty),
    "execute",
    tree_monadic_destructor,
    tree_monadic_print,
    evaluate,
    useful,
    0, /* constant */
    0, /* optimize */
};


tree_ty *
function_execute(args)
    tree_list_ty    *args;
{
    trace(("tree::execute::new\n"));
    function_needs_one("execute", args);
    return tree_monadic_new(&method, args->item[0]);
}
