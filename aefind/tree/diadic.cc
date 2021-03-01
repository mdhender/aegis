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
// MANIFEST: functions to manipulate diadic tree nodes
//

#include <common/ac/stdio.h>

#include <aefind/tree/diadic.h>


tree_ty *
tree_diadic_new(tree_method_ty *mp, tree_ty *left, tree_ty *right)
{
    tree_ty         *tp;
    tree_diadic_ty  *this_thing;

    tp = tree_new(mp);
    this_thing = (tree_diadic_ty *)tp;
    this_thing->left = tree_copy(left);
    this_thing->right = tree_copy(right);
    return tp;
}


void
tree_diadic_destructor(tree_ty *tp)
{
    tree_diadic_ty  *this_thing;

    this_thing = (tree_diadic_ty *)tp;
    tree_delete(this_thing->left);
    tree_delete(this_thing->right);
}


void
tree_diadic_print(tree_ty *tp)
{
    tree_diadic_ty  *this_thing;

    this_thing = (tree_diadic_ty *)tp;
    printf("( ");
    tree_print(this_thing->left);
    printf(" %s ", tp->method->name);
    tree_print(this_thing->right);
    printf(" )");
}


int
tree_diadic_useful(tree_ty *tp)
{
    tree_diadic_ty  *this_thing;

    this_thing = (tree_diadic_ty *)tp;
    return (tree_useful(this_thing->left) || tree_useful(this_thing->right));
}


int
tree_diadic_constant(tree_ty *tp)
{
    tree_diadic_ty  *this_thing;

    this_thing = (tree_diadic_ty *)tp;
    return
        (tree_constant(this_thing->left) && tree_constant(this_thing->right));
}


tree_ty *
tree_diadic_optimize(tree_ty *tp)
{
    tree_diadic_ty  *this_thing;
    tree_ty	    *left;
    tree_ty	    *right;
    tree_ty	    *result;

    this_thing = (tree_diadic_ty *)tp;
    left = tree_optimize(this_thing->left);
    right = tree_optimize(this_thing->right);
    result = tree_diadic_new(tp->method, left, right);
    tree_delete(left);
    tree_delete(right);

    if (tree_constant(result))
    {
	tree_ty		*tp2;

	tp2 = tree_optimize_constant(result);
	tree_delete(result);
	result = tp2;
    }

    return result;
}
