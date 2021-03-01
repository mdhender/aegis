/*
 *	aegis - project change supervisor
 *	Copyright (C) 1997 Peter Miller;
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
tree_monadic_new(mp, arg)
	tree_method_ty	*mp;
	tree_ty		*arg;
{
	tree_ty		*tp;
	tree_monadic_ty	*this;

	tp = tree_new(mp);
	this = (tree_monadic_ty *)tp;
	this->arg = tree_copy(arg);
	return tp;
}


void
tree_monadic_destructor(tp)
	tree_ty		*tp;
{
	tree_monadic_ty	*this;

	this = (tree_monadic_ty *)tp;
	tree_delete(this->arg);
}


void
tree_monadic_print(tp)
	tree_ty		*tp;
{
	tree_monadic_ty	*this;

	this = (tree_monadic_ty *)tp;
	printf("%s ( ", tp->method->name);
	tree_print(this->arg);
	printf(" )");
}


int
tree_monadic_useful(tp)
	tree_ty		*tp;
{
	tree_monadic_ty	*this;

	this = (tree_monadic_ty *)tp;
	return tree_useful(this->arg);
}


int
tree_monadic_constant(tp)
	tree_ty		*tp;
{
	tree_monadic_ty	*this;

	this = (tree_monadic_ty *)tp;
	return tree_constant(this->arg);
}


tree_ty *
tree_monadic_optimize(tp)
	tree_ty		*tp;
{
	tree_monadic_ty	*this;
	tree_ty		*arg;
	tree_ty		*result;

	this = (tree_monadic_ty *)tp;
	arg = tree_optimize(this->arg);
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
