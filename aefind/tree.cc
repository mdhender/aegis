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
// MANIFEST: functions to manipulate trees
//

#include <common/ac/string.h>
#include <common/ac/sys/types.h>
#include <sys/stat.h>

#include <libaegis/aer/value.h>
#include <common/error.h> // for assert
#include <common/mem.h>
#include <common/str.h>
#include <aefind/tree/constant.h>
#include <aefind/tree/private.h>


tree_ty *
tree_copy(tree_ty *tp)
{
    assert(tp->reference_count > 0);
    tp->reference_count++;
    return tp;
}


void
tree_delete(tree_ty *tp)
{
    assert(tp->reference_count > 0);
    tp->reference_count--;
    if (tp->reference_count > 0)
	return;
    assert(tp->method->destructor);
    tp->method->destructor(tp);
    mem_free(tp);
}


void
tree_print(tree_ty *tp)
{
    assert(tp->reference_count > 0);
    assert(tp->method->print);
    tp->method->print(tp);
}


rpt_value_ty *
tree_evaluate(tree_ty *tp, string_ty *pathname_unresolved, string_ty *pathname,
    string_ty *pathname_resolved, struct stat *st)
{
    assert(tp->reference_count > 0);
    assert(tp->method->evaluate);
    return
       	tp->method->evaluate
	(
	    tp,
	    pathname_unresolved,
	    pathname,
	    pathname_resolved,
	    st
	);
}


rpt_value_ty *
tree_evaluate_constant(tree_ty *tp)
{
    string_ty       *pathname;
    struct stat     st;
    rpt_value_ty    *vp;

    pathname = str_from_c("\377");
    memset(&st, 0, sizeof(st));
    vp = tree_evaluate(tp, pathname, pathname, pathname, &st);
    str_free(pathname);
    return vp;
}


int
tree_useful(tree_ty *tp)
{
    assert(tp->reference_count > 0);
    assert(tp->method->useful);
    return tp->method->useful(tp);
}


int
tree_constant(tree_ty *tp)
{
    assert(tp->reference_count > 0);
    if (!tp->method->constant)
	return 0;
    return tp->method->constant(tp);
}


tree_ty *
tree_optimize(tree_ty *tp)
{
    if (!tp->method->optimize)
	return tree_copy(tp);
    return tp->method->optimize(tp);
}


tree_ty *
tree_optimize_constant(tree_ty *tp)
{
    rpt_value_ty    *vp;
    tree_ty         *result;

    vp = tree_evaluate_constant(tp);
    result = tree_constant_new(vp);
    rpt_value_free(vp);
    return result;
}
