/*
 *	aegis - project change supervisor
 *	Copyright (C) 1997, 1999, 2001-2003 Peter Miller;
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
 * MANIFEST: functions to manipulate relative comparisons
 */

#include <ac/string.h>

#include <aer/value/boolean.h>
#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <aer/value/real.h>
#include <aer/value/string.h>
#include <error.h> /* for assert */
#include <str.h>
#include <sub.h>
#include <trace.h>
#include <tree/diadic.h>
#include <tree/relative.h>



#define PAIR(a, b)	((a) * rpt_value_type_MAX + (b))


static rpt_value_ty *
lt_evaluate(tree_ty *tp, string_ty *path, struct stat *st)
{
    tree_diadic_ty  *this_thing;
    sub_context_ty  *scp;
    rpt_value_ty    *lv1;
    rpt_value_ty    *lv2;
    rpt_value_ty    *rv1;
    rpt_value_ty    *rv2;
    rpt_value_ty    *vp;
    string_ty       *s;

    /*
     * evaluate the left hand side
     */
    trace(("lt::evaluate()\n{\n"));
    this_thing = (tree_diadic_ty *)tp;
    lv1 = tree_evaluate(this_thing->left, path, st);
    if (lv1->method->type == rpt_value_type_error)
    {
	trace(("}\n"));
	return lv1;
    }

    /*
     * coerce the left hand side to an arithmetic type
     * (will not give error if can't, will copy instead)
     */
    lv2 = rpt_value_arithmetic(lv1);

    /*
     * evaluate the right hand side
     */
    rv1 = tree_evaluate(this_thing->right, path, st);
    if (rv1->method->type == rpt_value_type_error)
    {
	rpt_value_free(lv1);
	rpt_value_free(lv2);
	trace(("}\n"));
	return rv1;
    }

    /*
     * coerce the right hand side to an arithmetic type
     * (will not give error if can't, will copy instead)
     */
    rv2 = rpt_value_arithmetic(rv1);

    /*
     * what to do depends on
     * the types of the operands
     */
    switch (PAIR(lv2->method->type, rv2->method->type))
    {
    case PAIR(rpt_value_type_real, rpt_value_type_real):
	vp =
	    rpt_value_boolean
	    (
	       	rpt_value_real_query(lv2) < rpt_value_real_query(rv2)
	    );
	break;

    case PAIR(rpt_value_type_real, rpt_value_type_integer):
	vp =
	    rpt_value_boolean
	    (
	       	rpt_value_real_query(lv2) < rpt_value_integer_query(rv2)
	    );
	break;

    case PAIR(rpt_value_type_integer, rpt_value_type_real):
	vp =
	    rpt_value_boolean
	    (
	       	rpt_value_integer_query(lv2) < rpt_value_real_query(rv2)
	    );
	break;

    case PAIR(rpt_value_type_integer, rpt_value_type_integer):
	vp =
	    rpt_value_boolean
	    (
	       	rpt_value_integer_query(lv2) < rpt_value_integer_query(rv2)
	    );
	break;

    case PAIR(rpt_value_type_string, rpt_value_type_string):
	string_string:
	vp =
	    rpt_value_boolean
	    (
	       	strcmp
	       	(
		    rpt_value_string_query(lv2)->str_text,
		    rpt_value_string_query(rv2)->str_text
	       	)
	    <
	       	0
	    );
	break;

    default:
	/*
	 * try to compare as strings
	 */
	rpt_value_free(lv2);
	rpt_value_free(rv2);
	lv2 = rpt_value_stringize(lv1);
	rv2 = rpt_value_stringize(rv1);
	if
	(
	    lv2->method->type == rpt_value_type_string
	&&
	    rv2->method->type == rpt_value_type_string
	)
	    goto string_string;

	/*
	 * give up on this one
	 */
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name1", lv1->method->name);
	sub_var_set_charstar(scp, "Name2", rv1->method->name);
	s = subst_intl(scp, i18n("illegal comparison ($name1 < $name2)"));
	sub_context_delete(scp);
	vp = rpt_value_error(0, s);
	str_free(s);
	break;
    }
    rpt_value_free(lv1);
    rpt_value_free(lv2);
    rpt_value_free(rv1);
    rpt_value_free(rv2);
    trace(("return %08lX;\n", (long)vp));
    trace(("}\n"));
    return vp;
}


static tree_method_ty lt_method =
{
    sizeof(tree_diadic_ty),
    "<",
    tree_diadic_destructor,
    tree_diadic_print,
    lt_evaluate,
    tree_diadic_useful,
    tree_diadic_constant,
    tree_diadic_optimize,
};


tree_ty *
tree_lt_new(tree_ty *left, tree_ty *right)
{
    return tree_diadic_new(&lt_method, left, right);
}


static rpt_value_ty *
le_evaluate(tree_ty *tp, string_ty *path, struct stat *st)
{
    tree_diadic_ty  *this_thing;
    sub_context_ty  *scp;
    rpt_value_ty    *lv1;
    rpt_value_ty    *lv2;
    rpt_value_ty    *rv1;
    rpt_value_ty    *rv2;
    rpt_value_ty    *vp;
    string_ty       *s;

    /*
     * evaluate the left hand side
     */
    trace(("le::evaluate()\n{\n"));
    this_thing = (tree_diadic_ty *)tp;
    lv1 = tree_evaluate(this_thing->left, path, st);
    if (lv1->method->type == rpt_value_type_error)
    {
	trace(("}\n"));
	return lv1;
    }

    /*
     * coerce the left hand side to an arithmetic type
     * (will not give error if can't, will copy instead)
     */
    lv2 = rpt_value_arithmetic(lv1);

    /*
     * evaluate the right hand side
     */
    rv1 = tree_evaluate(this_thing->right, path, st);
    if (rv1->method->type == rpt_value_type_error)
    {
	rpt_value_free(lv1);
	rpt_value_free(lv2);
	trace(("}\n"));
	return rv1;
    }

    /*
     * coerce the right hand side to an arithmetic type
     * (will not give error if can't, will copy instead)
     */
    rv2 = rpt_value_arithmetic(rv1);

    /*
     * what to do depends on
     * the types of the operands
     */
    switch (PAIR(lv2->method->type, rv2->method->type))
    {
    case PAIR(rpt_value_type_real, rpt_value_type_real):
	vp =
	    rpt_value_boolean
	    (
	       	rpt_value_real_query(lv2) <= rpt_value_real_query(rv2)
	    );
	break;

    case PAIR(rpt_value_type_real, rpt_value_type_integer):
	vp =
	    rpt_value_boolean
	    (
	       	rpt_value_real_query(lv2) <= rpt_value_integer_query(rv2)
	    );
	break;

    case PAIR(rpt_value_type_integer, rpt_value_type_real):
	vp =
	    rpt_value_boolean
	    (
	       	rpt_value_integer_query(lv2) <= rpt_value_real_query(rv2)
	    );
	break;

    case PAIR(rpt_value_type_integer, rpt_value_type_integer):
	vp =
	    rpt_value_boolean
	    (
	       	rpt_value_integer_query(lv2) <= rpt_value_integer_query(rv2)
	    );
	break;

    case PAIR(rpt_value_type_string, rpt_value_type_string):
	string_string:
	vp =
	    rpt_value_boolean
	    (
	       	strcmp
	       	(
		    rpt_value_string_query(lv2)->str_text,
		    rpt_value_string_query(rv2)->str_text
	       	)
	    <=
	       	0
	    );
	break;

    default:
	/*
	 * try to compare as strings
	 */
	rpt_value_free(lv2);
	rpt_value_free(rv2);
	lv2 = rpt_value_stringize(lv1);
	rv2 = rpt_value_stringize(rv1);
	if
	(
	    lv2->method->type == rpt_value_type_string
	&&
	    rv2->method->type == rpt_value_type_string
	)
	    goto string_string;

	/*
	 * give up on this one
	 */
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name1", lv1->method->name);
	sub_var_set_charstar(scp, "Name2", rv1->method->name);
	s = subst_intl(scp, i18n("illegal comparison ($name1 <= $name2)"));
	sub_context_delete(scp);
	vp = rpt_value_error(0, s);
	str_free(s);
	break;
    }
    rpt_value_free(lv1);
    rpt_value_free(lv2);
    rpt_value_free(rv1);
    rpt_value_free(rv2);
    trace(("return %08lX;\n", (long)vp));
    trace(("}\n"));
    return vp;
}


static tree_method_ty le_method =
{
    sizeof(tree_diadic_ty),
    "<=",
    tree_diadic_destructor,
    tree_diadic_print,
    le_evaluate,
    tree_diadic_useful,
    tree_diadic_constant,
    tree_diadic_optimize,
};


tree_ty *
tree_le_new(tree_ty *left, tree_ty *right)
{
    return tree_diadic_new(&le_method, left, right);
}


static rpt_value_ty *
gt_evaluate(tree_ty *tp, string_ty *path, struct stat *st)
{
    tree_diadic_ty  *this_thing;
    sub_context_ty  *scp;
    rpt_value_ty    *lv1;
    rpt_value_ty    *lv2;
    rpt_value_ty    *rv1;
    rpt_value_ty    *rv2;
    rpt_value_ty    *vp;
    string_ty       *s;

    /*
     * evaluate the left hand side
     */
    trace(("gt::evaluate()\n{\n"));
    this_thing = (tree_diadic_ty *)tp;
    lv1 = tree_evaluate(this_thing->left, path, st);
    if (lv1->method->type == rpt_value_type_error)
    {
	trace(("}\n"));
	return lv1;
    }

    /*
     * coerce the left hand side to an arithmetic type
     * (will not give error if can't, will copy instead)
     */
    lv2 = rpt_value_arithmetic(lv1);

    /*
     * evaluate the right hand side
     */
    rv1 = tree_evaluate(this_thing->right, path, st);
    if (rv1->method->type == rpt_value_type_error)
    {
	rpt_value_free(lv1);
	rpt_value_free(lv2);
	trace(("}\n"));
	return rv1;
    }

    /*
     * coerce the right hand side to an arithmetic type
     * (will not give error if can't, will copy instead)
     */
    rv2 = rpt_value_arithmetic(rv1);

    /*
     * what to do depends on
     * the types of the operands
     */
    switch (PAIR(lv2->method->type, rv2->method->type))
    {
    case PAIR(rpt_value_type_real, rpt_value_type_real):
	vp =
	    rpt_value_boolean
	    (
	       	rpt_value_real_query(lv2) > rpt_value_real_query(rv2)
	    );
	break;

    case PAIR(rpt_value_type_real, rpt_value_type_integer):
	vp =
	    rpt_value_boolean
	    (
	       	rpt_value_real_query(lv2) > rpt_value_integer_query(rv2)
	    );
	break;

    case PAIR(rpt_value_type_integer, rpt_value_type_real):
	vp =
	    rpt_value_boolean
	    (
	       	rpt_value_integer_query(lv2) > rpt_value_real_query(rv2)
	    );
	break;

    case PAIR(rpt_value_type_integer, rpt_value_type_integer):
	vp =
	    rpt_value_boolean
	    (
	       	rpt_value_integer_query(lv2) > rpt_value_integer_query(rv2)
	    );
	break;

    case PAIR(rpt_value_type_string, rpt_value_type_string):
	string_string:
	vp =
	    rpt_value_boolean
	    (
	       	strcmp
	       	(
		    rpt_value_string_query(lv2)->str_text,
		    rpt_value_string_query(rv2)->str_text
	       	)
	    >
	       	0
	    );
	break;

    default:
	/*
	 * try to compare as strings
	 */
	rpt_value_free(lv2);
	rpt_value_free(rv2);
	lv2 = rpt_value_stringize(lv1);
	rv2 = rpt_value_stringize(rv1);
	if
	(
	    lv2->method->type == rpt_value_type_string
	&&
	    rv2->method->type == rpt_value_type_string
	)
	    goto string_string;

	/*
	 * give up on this one
	 */
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name1", lv1->method->name);
	sub_var_set_charstar(scp, "Name2", rv1->method->name);
	s = subst_intl(scp, i18n("illegal comparison ($name1 > $name2)"));
	sub_context_delete(scp);
	vp = rpt_value_error(0, s);
	str_free(s);
	break;
    }
    rpt_value_free(lv1);
    rpt_value_free(lv2);
    rpt_value_free(rv1);
    rpt_value_free(rv2);
    trace(("return %08lX;\n", (long)vp));
    trace(("}\n"));
    return vp;
}


static tree_method_ty gt_method =
{
    sizeof(tree_diadic_ty),
    ">",
    tree_diadic_destructor,
    tree_diadic_print,
    gt_evaluate,
    tree_diadic_useful,
    tree_diadic_constant,
    tree_diadic_optimize,
};


tree_ty *
tree_gt_new(tree_ty *left, tree_ty *right)
{
    return tree_diadic_new(&gt_method, left, right);
}


static rpt_value_ty *
ge_evaluate(tree_ty *tp, string_ty *path, struct stat *st)
{
    tree_diadic_ty  *this_thing;
    sub_context_ty  *scp;
    rpt_value_ty    *lv1;
    rpt_value_ty    *lv2;
    rpt_value_ty    *rv1;
    rpt_value_ty    *rv2;
    rpt_value_ty    *vp;
    string_ty       *s;

    /*
     * evaluate the left hand side
     */
    trace(("ge::evaluate()\n{\n"));
    this_thing = (tree_diadic_ty *)tp;
    lv1 = tree_evaluate(this_thing->left, path, st);
    if (lv1->method->type == rpt_value_type_error)
    {
	trace(("}\n"));
	return lv1;
    }

    /*
     * coerce the left hand side to an arithmetic type
     * (will not give error if can't, will copy instead)
     */
    lv2 = rpt_value_arithmetic(lv1);

    /*
     * evaluate the right hand side
     */
    rv1 = tree_evaluate(this_thing->right, path, st);
    if (rv1->method->type == rpt_value_type_error)
    {
	rpt_value_free(lv1);
	rpt_value_free(lv2);
	trace(("}\n"));
	return rv1;
    }

    /*
     * coerce the right hand side to an arithmetic type
     * (will not give error if can't, will copy instead)
     */
    rv2 = rpt_value_arithmetic(rv1);

    /*
     * what to do depends on
     * the types of the operands
     */
    switch (PAIR(lv2->method->type, rv2->method->type))
    {
    case PAIR(rpt_value_type_real, rpt_value_type_real):
	vp =
	    rpt_value_boolean
	    (
	       	rpt_value_real_query(lv2) >= rpt_value_real_query(rv2)
	    );
	break;

    case PAIR(rpt_value_type_real, rpt_value_type_integer):
	vp =
	    rpt_value_boolean
	    (
	       	rpt_value_real_query(lv2) >= rpt_value_integer_query(rv2)
	    );
	break;

    case PAIR(rpt_value_type_integer, rpt_value_type_real):
	vp =
	    rpt_value_boolean
	    (
	       	rpt_value_integer_query(lv2) >= rpt_value_real_query(rv2)
	    );
	break;

    case PAIR(rpt_value_type_integer, rpt_value_type_integer):
	vp =
	    rpt_value_boolean
	    (
	       	rpt_value_integer_query(lv2) >= rpt_value_integer_query(rv2)
	    );
	break;

    case PAIR(rpt_value_type_string, rpt_value_type_string):
	string_string:
	vp =
	    rpt_value_boolean
	    (
	       	strcmp
	       	(
		    rpt_value_string_query(lv2)->str_text,
		    rpt_value_string_query(rv2)->str_text
	       	)
	    >=
	       	0
	    );
	break;

    default:
	/*
	 * try to compare as strings
	 */
	rpt_value_free(lv2);
	rpt_value_free(rv2);
	lv2 = rpt_value_stringize(lv1);
	rv2 = rpt_value_stringize(rv1);
	if
	(
	    lv2->method->type == rpt_value_type_string
	&&
	    rv2->method->type == rpt_value_type_string
	)
	    goto string_string;

	/*
	 * give up on this one
	 */
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name1", lv1->method->name);
	sub_var_set_charstar(scp, "Name2", rv1->method->name);
	s = subst_intl(scp, i18n("illegal comparison ($name1 >= $name2)"));
	sub_context_delete(scp);
	vp = rpt_value_error(0, s);
	str_free(s);
	break;
    }
    rpt_value_free(lv1);
    rpt_value_free(lv2);
    rpt_value_free(rv1);
    rpt_value_free(rv2);
    trace(("return %08lX;\n", (long)vp));
    trace(("}\n"));
    return vp;
}


static tree_method_ty ge_method =
{
    sizeof(tree_diadic_ty),
    ">=",
    tree_diadic_destructor,
    tree_diadic_print,
    ge_evaluate,
    tree_diadic_useful,
    tree_diadic_constant,
    tree_diadic_optimize,
};


tree_ty *
tree_ge_new(tree_ty *left, tree_ty *right)
{
    return tree_diadic_new(&ge_method, left, right);
}


static rpt_value_ty *
eq_evaluate(tree_ty *tp, string_ty *path, struct stat *st)
{
    tree_diadic_ty  *this_thing;
    sub_context_ty  *scp;
    rpt_value_ty    *lv1;
    rpt_value_ty    *lv2;
    rpt_value_ty    *rv1;
    rpt_value_ty    *rv2;
    rpt_value_ty    *vp;
    string_ty       *s;

    /*
     * evaluate the left hand side
     */
    trace(("eq::evaluate()\n{\n"));
    this_thing = (tree_diadic_ty *)tp;
    lv1 = tree_evaluate(this_thing->left, path, st);
    if (lv1->method->type == rpt_value_type_error)
    {
	trace(("}\n"));
	return lv1;
    }

    /*
     * coerce the left hand side to an arithmetic type
     * (will not give error if can't, will copy instead)
     */
    lv2 = rpt_value_arithmetic(lv1);

    /*
     * evaluate the right hand side
     */
    rv1 = tree_evaluate(this_thing->right, path, st);
    if (rv1->method->type == rpt_value_type_error)
    {
	rpt_value_free(lv1);
	rpt_value_free(lv2);
	trace(("}\n"));
	return rv1;
    }

    /*
     * coerce the right hand side to an arithmetic type
     * (will not give error if can't, will copy instead)
     */
    rv2 = rpt_value_arithmetic(rv1);

    /*
     * what to do depends on
     * the types of the operands
     */
    switch (PAIR(lv2->method->type, rv2->method->type))
    {
    case PAIR(rpt_value_type_real, rpt_value_type_real):
	vp =
	    rpt_value_boolean
	    (
	       	rpt_value_real_query(lv2) == rpt_value_real_query(rv2)
	    );
	break;

    case PAIR(rpt_value_type_real, rpt_value_type_integer):
	vp =
	    rpt_value_boolean
	    (
	       	rpt_value_real_query(lv2) == rpt_value_integer_query(rv2)
	    );
	break;

    case PAIR(rpt_value_type_integer, rpt_value_type_real):
	vp =
	    rpt_value_boolean
	    (
	       	rpt_value_integer_query(lv2) == rpt_value_real_query(rv2)
	    );
	break;

    case PAIR(rpt_value_type_integer, rpt_value_type_integer):
	vp =
	    rpt_value_boolean
	    (
	       	rpt_value_integer_query(lv2) == rpt_value_integer_query(rv2)
	    );
	break;

    case PAIR(rpt_value_type_string, rpt_value_type_string):
	string_string:
	vp =
	    rpt_value_boolean
	    (
	       	str_equal
	       	(
		    rpt_value_string_query(lv2),
		    rpt_value_string_query(rv2)
	       	)
	    );
	break;

    default:
	/*
	 * try to compare as strings
	 */
	rpt_value_free(lv2);
	rpt_value_free(rv2);
	lv2 = rpt_value_stringize(lv1);
	rv2 = rpt_value_stringize(rv1);
	if
	(
	    lv2->method->type == rpt_value_type_string
	&&
	    rv2->method->type == rpt_value_type_string
	)
	    goto string_string;

	/*
	 * give up on this one
	 */
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name1", lv1->method->name);
	sub_var_set_charstar(scp, "Name2", rv1->method->name);
	s = subst_intl(scp, i18n("illegal comparison ($name1 == $name2)"));
	sub_context_delete(scp);
	vp = rpt_value_error(0, s);
	str_free(s);
	break;
    }
    rpt_value_free(lv1);
    rpt_value_free(lv2);
    rpt_value_free(rv1);
    rpt_value_free(rv2);
    trace(("return %08lX;\n", (long)vp));
    trace(("}\n"));
    return vp;
}


static tree_method_ty eq_method =
{
    sizeof(tree_diadic_ty),
    "==",
    tree_diadic_destructor,
    tree_diadic_print,
    eq_evaluate,
    tree_diadic_useful,
    tree_diadic_constant,
    tree_diadic_optimize,
};


tree_ty *
tree_eq_new(tree_ty *left, tree_ty *right)
{
    return tree_diadic_new(&eq_method, left, right);
}


static rpt_value_ty *
ne_evaluate(tree_ty *tp, string_ty *path, struct stat *st)
{
    tree_diadic_ty  *this_thing;
    sub_context_ty  *scp;
    rpt_value_ty    *lv1;
    rpt_value_ty    *lv2;
    rpt_value_ty    *rv1;
    rpt_value_ty    *rv2;
    rpt_value_ty    *vp;
    string_ty       *s;

    /*
     * evaluate the left hand side
     */
    trace(("ne::evaluate()\n{\n"));
    this_thing = (tree_diadic_ty *)tp;
    lv1 = tree_evaluate(this_thing->left, path, st);
    if (lv1->method->type == rpt_value_type_error)
    {
	trace(("}\n"));
	return lv1;
    }

    /*
     * coerce the left hand side to an arithmetic type
     * (will not give error if can't, will copy instead)
     */
    lv2 = rpt_value_arithmetic(lv1);

    /*
     * evaluate the right hand side
     */
    rv1 = tree_evaluate(this_thing->right, path, st);
    if (rv1->method->type == rpt_value_type_error)
    {
	rpt_value_free(lv1);
	rpt_value_free(lv2);
	trace(("}\n"));
	return rv1;
    }

    /*
     * coerce the right hand side to an arithmetic type
     * (will not give error if can't, will copy instead)
     */
    rv2 = rpt_value_arithmetic(rv1);

    /*
     * what to do depends on
     * the types of the operands
     */
    switch (PAIR(lv2->method->type, rv2->method->type))
    {
    case PAIR(rpt_value_type_real, rpt_value_type_real):
	vp =
	    rpt_value_boolean
	    (
	       	rpt_value_real_query(lv2) != rpt_value_real_query(rv2)
	    );
	break;

    case PAIR(rpt_value_type_real, rpt_value_type_integer):
	vp =
	    rpt_value_boolean
	    (
	       	rpt_value_real_query(lv2) != rpt_value_integer_query(rv2)
	    );
	break;

    case PAIR(rpt_value_type_integer, rpt_value_type_real):
	vp =
	    rpt_value_boolean
	    (
	       	rpt_value_integer_query(lv2) != rpt_value_real_query(rv2)
	    );
	break;

    case PAIR(rpt_value_type_integer, rpt_value_type_integer):
	vp =
	    rpt_value_boolean
	    (
	       	rpt_value_integer_query(lv2) != rpt_value_integer_query(rv2)
	    );
	break;

    case PAIR(rpt_value_type_string, rpt_value_type_string):
	string_string:
	vp =
	    rpt_value_boolean
	    (
	       	!str_equal
	       	(
		    rpt_value_string_query(lv2),
		    rpt_value_string_query(rv2)
	       	)
	    );
	break;

    default:
	/*
	 * try to compare as strings
	 */
	rpt_value_free(lv2);
	rpt_value_free(rv2);
	lv2 = rpt_value_stringize(lv1);
	rv2 = rpt_value_stringize(rv1);
	if
	(
	    lv2->method->type == rpt_value_type_string
	&&
	    rv2->method->type == rpt_value_type_string
	)
	    goto string_string;

	/*
	 * give up on this one
	 */
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name1", lv1->method->name);
	sub_var_set_charstar(scp, "Name2", rv1->method->name);
	s = subst_intl(scp, i18n("illegal comparison ($name1 != $name2)"));
	sub_context_delete(scp);
	vp = rpt_value_error(0, s);
	str_free(s);
	break;
    }
    rpt_value_free(lv1);
    rpt_value_free(lv2);
    rpt_value_free(rv1);
    rpt_value_free(rv2);
    trace(("return %08lX;\n", (long)vp));
    trace(("}\n"));
    return vp;
}


static tree_method_ty ne_method =
{
    sizeof(tree_diadic_ty),
    "!=",
    tree_diadic_destructor,
    tree_diadic_print,
    ne_evaluate,
    tree_diadic_useful,
    tree_diadic_constant,
    tree_diadic_optimize,
};


tree_ty *
tree_ne_new(tree_ty *left, tree_ty *right)
{
    return tree_diadic_new(&ne_method, left, right);
}
