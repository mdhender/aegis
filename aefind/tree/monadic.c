/*
 *	aegis - project change supervisor
 *	Copyright (C) 1997, 2002, 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate monadic tree nodes
 */

#include <ac/stdio.h>

#include <tree/monadic.h>


tree_ty *
tree_monadic_new(tree_method_ty *mp, tree_ty *arg)
{
    tree_ty         *tp;
    tree_monadic_ty *this_thing;

    tp = tree_new(mp);
    this_thing = (tree_monadic_ty *)tp;
    this_thing->arg = tree_copy(arg);
    return tp;
}


void
tree_monadic_destructor(tree_ty *tp)
{
    tree_monadic_ty *this_thing;

    this_thing = (tree_monadic_ty *)tp;
    tree_delete(this_thing->arg);
}


void
tree_monadic_print(tree_ty *tp)
{
    tree_monadic_ty *this_thing;

    this_thing = (tree_monadic_ty *)tp;
    printf("%s ( ", tp->method->name);
    tree_print(this_thing->arg);
    printf(" )");
}


int
tree_monadic_useful(tree_ty *tp)
{
    tree_monadic_ty *this_thing;

    this_thing = (tree_monadic_ty *)tp;
    return tree_useful(this_thing->arg);
}


int
tree_monadic_constant(tree_ty *tp)
{
    tree_monadic_ty *this_thing;

    this_thing = (tree_monadic_ty *)tp;
    return tree_constant(this_thing->arg);
}


tree_ty *
tree_monadic_optimize(tree_ty *tp)
{
    tree_monadic_ty *this_thing;
    tree_ty         *arg;
    tree_ty	    *result;

    this_thing = (tree_monadic_ty *)tp;
    arg = tree_optimize(this_thing->arg);
    result = tree_monadic_new(tp->method, arg);
    tree_delete(arg);

    if (tree_constant(result))
    {
	tree_ty		*tp2;

	tp2 = tree_optimize_constant(result);
	tree_delete(result);
	result = tp2;
    }

    return result;
}
