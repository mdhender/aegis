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
 * MANIFEST: functions to manipulate print tree nodes
 */

#include <ac/stdio.h>

#include <aer/value/boolean.h>
#include <aer/value/string.h>
#include <function/needs.h>
#include <function/print.h>
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

	this = (tree_monadic_ty *)tp;
	vp = tree_evaluate(this->arg, path, st);
	svp = rpt_value_stringize(vp);
	rpt_value_free(vp);
	printf("%s\n", rpt_value_string_query(svp)->str_text);
	rpt_value_free(svp);
	return rpt_value_boolean(1);
}


static int useful _((tree_ty *));

static int
useful(tp)
	tree_ty		*tp;
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
	0, /* constant */
	0, /* optimize */
};


tree_ty *
function_print(args)
	tree_list_ty	*args;
{
	function_needs_one("print", args);
	return tree_monadic_new(&method, args->item[0]);
}
