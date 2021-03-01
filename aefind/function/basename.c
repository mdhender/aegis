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
 * MANIFEST: functions to manipulate basename tree nodes
 */

#include <ac/stdio.h>
#include <ac/string.h>

#include <aer/value/string.h>
#include <function/basename.h>
#include <function/needs.h>
#include <str.h>
#include <tree/list.h>
#include <tree/monadic.h>


static rpt_value_ty *evaluate _((tree_ty *, string_ty *, struct stat *));

static rpt_value_ty *
evaluate(tp, path, st)
	tree_ty		*tp;
	string_ty	*path;
	struct stat	*st;
{
	tree_monadic_ty	*this;
	rpt_value_ty	*vp;
	rpt_value_ty	*svp;
	rpt_value_ty	*result;
	string_ty	*s;
	char		*cp;

	this = (tree_monadic_ty *)tp;
	vp = tree_evaluate(this->arg, path, st);
	svp = rpt_value_stringize(vp);
	rpt_value_free(vp);

	s = rpt_value_string_query(svp);
	cp = strrchr(s->str_text, '/');
	if (cp)
		++cp;
	else
		cp = s->str_text;
	s = str_from_c(cp);
	rpt_value_free(svp);

	result = rpt_value_string(s);
	str_free(s);
	return result;
}


static tree_method_ty method =
{
	sizeof(tree_monadic_ty),
	"basename",
	tree_monadic_destructor,
	tree_monadic_print,
	evaluate,
	tree_monadic_useful,
	tree_monadic_constant,
	tree_monadic_optimize,
};


tree_ty *
function_basename(args)
	tree_list_ty	*args;
{
	function_needs_one("basename", args);
	return tree_monadic_new(&method, args->item[0]);
}
