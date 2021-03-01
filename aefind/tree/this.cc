//
//	aegis - project change supervisor
//	Copyright (C) 1997, 2002, 2004, 2005 Peter Miller;
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
// MANIFEST: functions to manipulate current pathname tree nodes
//

#include <ac/stdio.h>

#include <aer/value/string.h>
#include <error.h> // for assert
#include <str.h>
#include <trace.h>
#include <tree/this.h>
#include <tree/private.h>


struct tree_this_ty
{
    tree_ty inherited;
    int resolved;
};


static void
destructor(tree_ty *tp)
{
    trace(("tree::this::destructor\n"));
}


static void
print(tree_ty *tp)
{
    trace(("tree::this::destructor\n"));
    printf("{}");
}


static rpt_value_ty *
evaluate(tree_ty *tp, string_ty *unresolved_path, string_ty *path,
    string_ty *resolved_path, struct stat *st)
{
    assert(path);
    trace(("tree::this::evaluate\n"));
    tree_this_ty *ttp = (tree_this_ty *)tp;
    if (ttp->resolved == 0)
	return rpt_value_string(unresolved_path);
    if (ttp->resolved > 0)
	return rpt_value_string(resolved_path);
    return rpt_value_string(path);
}


static int
useful(tree_ty *tp)
{
    trace(("tree::this::useful\n"));
    return 0;
}


static int
constant(tree_ty *tp)
{
    trace(("tree::this::constant\n"));
    return 0;
}


static tree_method_ty method =
{
    sizeof(tree_this_ty),
    "this",
    destructor,
    print,
    evaluate,
    useful,
    constant,
    0, // optimize
};


tree_ty *
tree_this_new(int arg)
{
    tree_ty *result = tree_new(&method);
    tree_this_ty *ttp = (tree_this_ty *)result;
    ttp->resolved = arg;
    return result;
}
