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
 * MANIFEST: functions to manipulate trees
 */

#include <ac/string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <aer/value.h>
#include <error.h> /* for assert */
#include <mem.h>
#include <str.h>
#include <tree/constant.h>
#include <tree/private.h>


tree_ty *
tree_copy(tp)
	tree_ty		*tp;
{
	assert(tp->reference_count > 0);
	tp->reference_count++;
	return tp;
}


void
tree_delete(tp)
	tree_ty		*tp;
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
tree_print(tp)
	tree_ty		*tp;
{
	assert(tp->reference_count > 0);
	assert(tp->method->print);
	tp->method->print(tp);
}


rpt_value_ty *
tree_evaluate(tp, pathname, st)
	tree_ty		*tp;
	string_ty	*pathname;
	struct stat	*st;
{
	assert(tp->reference_count > 0);
	assert(tp->method->evaluate);
	return tp->method->evaluate(tp, pathname, st);
}


rpt_value_ty *
tree_evaluate_constant(tp)
	tree_ty		*tp;
{
	string_ty	*pathname;
	struct stat	st;
	rpt_value_ty	*vp;

	pathname = str_from_c("\377");
	memset(&st, 0, sizeof(st));
	vp = tree_evaluate(tp, pathname, &st);
	str_free(pathname);
	return vp;
}


int
tree_useful(tp)
	tree_ty		*tp;
{
	assert(tp->reference_count > 0);
	assert(tp->method->useful);
	return tp->method->useful(tp);
}


int
tree_constant(tp)
	tree_ty		*tp;
{
	assert(tp->reference_count > 0);
	if (!tp->method->constant)
		return 0;
	return tp->method->constant(tp);
}


tree_ty *
tree_optimize(tp)
	tree_ty		*tp;
{
	if (!tp->method->optimize)
		return tree_copy(tp);
	return tp->method->optimize(tp);
}


tree_ty *
tree_optimize_constant(tp)
	tree_ty		*tp;
{
	rpt_value_ty	*vp;
	tree_ty		*result;

	vp = tree_evaluate_constant(tp);
	result = tree_constant_new(vp);
	rpt_value_free(vp);
	return result;
}
