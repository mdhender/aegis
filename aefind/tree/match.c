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
 * MANIFEST: functions to manipulate match tree nodes
 */

#include <aer/value/boolean.h>
#include <aer/value/error.h>
#include <aer/value/string.h>
#include <gmatch.h>
#include <str.h>
#include <sub.h>
#include <tree/match.h>
#include <tree/diadic.h>


static rpt_value_ty *match_evaluate _((tree_ty *, string_ty *, struct stat *));

static rpt_value_ty *
match_evaluate(tp, path, st)
	tree_ty		*tp;
	string_ty	*path;
	struct stat	*st;
{
	tree_diadic_ty	*this;
	rpt_value_ty	*v1;
	rpt_value_ty	*v1s;
	rpt_value_ty	*v2;
	rpt_value_ty	*v2s;
	rpt_value_ty	*result;

	this = (tree_diadic_ty *)tp;

	v1 = tree_evaluate(this->left, path, st);
	v1s = rpt_value_stringize(v1);
	v2 = tree_evaluate(this->right, path, st);
	v2s = rpt_value_stringize(v2);

	if
	(
		v1s->method->type == rpt_value_type_string
	&&
		v2s->method->type == rpt_value_type_string
	)
	{
		result =
			rpt_value_boolean
			(
				gmatch
				(
					rpt_value_string_query(v1s)->str_text,
					rpt_value_string_query(v2s)->str_text
				)
			);
	}
	else
	{
		sub_context_ty	*scp;
		string_ty	*s;

		scp = sub_context_new();
		sub_var_set(scp, "Name1", "%s", v1->method->name);
		sub_var_set(scp, "Name2", "%s", v2->method->name);
		s = subst_intl(scp, i18n("illegal match ($name1 ~ $name2)"));
		sub_context_delete(scp);
		result = rpt_value_error(0, s);
		str_free(s);
	}
	rpt_value_free(v1);
	rpt_value_free(v2);
	rpt_value_free(v1s);
	rpt_value_free(v2s);
	return result;
}


static tree_method_ty match_method =
{
	sizeof(tree_diadic_ty),
	"~",
	tree_diadic_destructor,
	tree_diadic_print,
	match_evaluate,
	tree_diadic_useful,
	tree_diadic_constant,
	tree_diadic_optimize,
};


tree_ty *
tree_match_new(left, right)
	tree_ty		*left;
	tree_ty		*right;
{
	return tree_diadic_new(&match_method, left, right);
}
