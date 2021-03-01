//
//	aegis - project change supervisor
//	Copyright (C) 1997, 2002-2005 Peter Miller;
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
// MANIFEST: functions to manipulate print tree nodes
//

#include <common/ac/stdio.h>

#include <libaegis/aer/value/boolean.h>
#include <libaegis/aer/value/string.h>
#include <aefind/function/needs.h>
#include <aefind/function/print.h>
#include <common/str.h>
#include <aefind/tree/list.h>
#include <aefind/tree/monadic.h>


static rpt_value_ty *
evaluate(tree_ty *tp, string_ty *path1, string_ty *path2, string_ty *path3,
    struct stat *st)
{
    tree_monadic_ty *this_thing;
    rpt_value_ty    *vp;
    rpt_value_ty    *svp;

    this_thing = (tree_monadic_ty *)tp;
    vp = tree_evaluate(this_thing->arg, path1, path2, path3, st);
    svp = rpt_value_stringize(vp);
    rpt_value_free(vp);
    printf("%s\n", rpt_value_string_query(svp)->str_text);
    rpt_value_free(svp);
    return rpt_value_boolean(1);
}


static int
useful(tree_ty *tp)
{
    return 1;
}


static tree_method_ty method =
{
    sizeof(tree_monadic_ty),
    "print",
    tree_monadic_destructor,
    tree_monadic_print,
    evaluate,
    useful,
    0, // constant
    0, // optimize
};


tree_ty *
function_print(tree_list_ty *args)
{
    function_needs_one("print", args);
    return tree_monadic_new(&method, args->item[0]);
}
