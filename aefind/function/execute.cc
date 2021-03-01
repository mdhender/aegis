//
//	aegis - project change supervisor
//	Copyright (C) 2002-2005 Peter Miller;
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
// MANIFEST: functions to manipulate executes
//

#include <libaegis/aer/value/boolean.h>
#include <libaegis/aer/value/string.h>
#include <aefind/function/execute.h>
#include <aefind/function/needs.h>
#include <libaegis/os.h>
#include <common/trace.h>
#include <aefind/tree/list.h>
#include <aefind/tree/monadic.h>


static rpt_value_ty *
evaluate(tree_ty *tp, string_ty *path1, string_ty *path2, string_ty *path3,
    struct stat *st)
{
    tree_monadic_ty *this_thing;
    rpt_value_ty    *vp;
    rpt_value_ty    *svp;
    int		    exit_status;

    trace(("tree::execute::evaluate\n"));
    this_thing = (tree_monadic_ty *)tp;
    vp = tree_evaluate(this_thing->arg, path1, path2, path3, st);
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


static int
useful(tree_ty *tp)
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
    0, // constant
    0, // optimize
};


tree_ty *
function_execute(tree_list_ty *args)
{
    trace(("tree::execute::new\n"));
    function_needs_one("execute", args);
    return tree_monadic_new(&method, args->item[0]);
}
