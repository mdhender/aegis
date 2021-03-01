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
 * MANIFEST: functions to manipulate diadic tree nodes
 */

#include <ac/stdio.h>

#include <tree/diadic.h>


tree_ty *
tree_diadic_new(mp, left, right)
	tree_method_ty	*mp;
	tree_ty		*left;
	tree_ty		*right;
{
	tree_ty		*tp;
	tree_diadic_ty	*this;

	tp = tree_new(mp);
	this = (tree_diadic_ty *)tp;
	this->left = tree_copy(left);
	this->right = tree_copy(right);
	return tp;
}


void
tree_diadic_destructor(tp)
	tree_ty		*tp;
{
	tree_diadic_ty	*this;

	this = (tree_diadic_ty *)tp;
	tree_delete(this->left);
	tree_delete(this->right);
}


void
tree_diadic_print(tp)
	tree_ty		*tp;
{
	tree_diadic_ty	*this;

	this = (tree_diadic_ty *)tp;
	printf("( ");
	tree_print(this->left);
	printf(" %s ", tp->method->name);
	tree_print(this->right);
	printf(" )");
}


int
tree_diadic_useful(tp)
	tree_ty		*tp;
{
	tree_diadic_ty	*this;

	this = (tree_diadic_ty *)tp;
	return (tree_useful(this->left) || tree_useful(this->right));
}


int
tree_diadic_constant(tp)
	tree_ty		*tp;
{
	tree_diadic_ty	*this;

	this = (tree_diadic_ty *)tp;
	return (tree_constant(this->left) && tree_constant(this->right));
}


tree_ty *
tree_diadic_optimize(tp)
	tree_ty		*tp;
{
	tree_diadic_ty	*this;
	tree_ty		*left;
	tree_ty		*right;
	tree_ty		*result;

	this = (tree_diadic_ty *)tp;
	left = tree_optimize(this->left);
	right = tree_optimize(this->right);
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
