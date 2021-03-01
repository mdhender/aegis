/*
 *	aegis - project change supervisor
 *	Copyright (C) 1997, 1999, 2002, 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate logical trees
 */

#include <ac/stdio.h>

#include <aer/value/boolean.h>
#include <aer/value/error.h>
#include <str.h>
#include <sub.h>
#include <tree/constant.h>
#include <tree/diadic.h>
#include <tree/logical.h>
#include <tree/monadic.h>


static rpt_value_ty *
and_evaluate(tree_ty *tp, string_ty *path, struct stat *st)
{
    tree_diadic_ty  *this_thing;
    rpt_value_ty    *v1;
    rpt_value_ty    *v1b;
    rpt_value_ty    *v2;
    rpt_value_ty    *v2b;

    this_thing = (tree_diadic_ty *)tp;

    v1 = tree_evaluate(this_thing->left, path, st);
    if (v1->method->type == rpt_value_type_error)
	return v1;
    v1b = rpt_value_booleanize(v1);
    if (v1b->method->type != rpt_value_type_boolean)
    {
	sub_context_ty	*scp;
	string_ty	*s;

	scp = sub_context_new();
	rpt_value_free(v1b);
	sub_var_set_charstar(scp, "Name", v1->method->name);
	rpt_value_free(v1);
	s =
	    subst_intl
	    (
	       	scp,
		i18n("boolean value required for logical and (was given $name)")
	    );
	sub_context_delete(scp);
	v1b = rpt_value_error(0, s);
	str_free(s);
	return v1b;
    }
    rpt_value_free(v1);
    if (!rpt_value_boolean_query(v1b))
	return v1b;
    rpt_value_free(v1b);

    v2 = tree_evaluate(this_thing->right, path, st);
    if (v2->method->type == rpt_value_type_error)
	return v2;
    v2b = rpt_value_booleanize(v2);
    if (v2b->method->type != rpt_value_type_boolean)
    {
	sub_context_ty	*scp;
	string_ty	*s;

	scp = sub_context_new();
	rpt_value_free(v2b);
	sub_var_set_charstar(scp, "Name", v2->method->name);
	rpt_value_free(v2);
	s =
	    subst_intl
	    (
	       	scp,
		i18n("boolean value required for logical and (was given $name)")
	    );
	sub_context_delete(scp);
	v2b = rpt_value_error(0, s);
	str_free(s);
	return v2b;
    }
    rpt_value_free(v2);
    return v2b;
}


static tree_method_ty and_method =
{
    sizeof(tree_diadic_ty),
    "&&",
    tree_diadic_destructor,
    tree_diadic_print,
    and_evaluate,
    tree_diadic_useful,
    tree_diadic_constant,
    tree_diadic_optimize,
};


tree_ty *
tree_and_new(tree_ty *left, tree_ty *right)
{
    return tree_diadic_new(&and_method, left, right);
}


static rpt_value_ty *
or_evaluate(tree_ty *tp, string_ty *path, struct stat *st)
{
    tree_diadic_ty  *this_thing;
    rpt_value_ty    *v1;
    rpt_value_ty    *v1b;
    rpt_value_ty    *v2;
    rpt_value_ty    *v2b;

    this_thing = (tree_diadic_ty *)tp;

    v1 = tree_evaluate(this_thing->left, path, st);
    if (v1->method->type == rpt_value_type_error)
	return v1;
    v1b = rpt_value_booleanize(v1);
    if (v1b->method->type != rpt_value_type_boolean)
    {
	sub_context_ty	*scp;
	string_ty	*s;

	scp = sub_context_new();
	rpt_value_free(v1b);
	sub_var_set_charstar(scp, "Name", v1->method->name);
	rpt_value_free(v1);
	s =
	    subst_intl
	    (
	       	scp,
		i18n("boolean value required for logical or (was given $name)")
	    );
	sub_context_delete(scp);
	v1b = rpt_value_error(0, s);
	str_free(s);
	return v1b;
    }
    rpt_value_free(v1);
    if (rpt_value_boolean_query(v1b))
	return v1b;
    rpt_value_free(v1b);

    v2 = tree_evaluate(this_thing->right, path, st);
    if (v2->method->type == rpt_value_type_error)
	return v2;
    v2b = rpt_value_booleanize(v2);
    if (v2b->method->type != rpt_value_type_boolean)
    {
	sub_context_ty	*scp;
	string_ty	*s;

	scp = sub_context_new();
	rpt_value_free(v2b);
	sub_var_set_charstar(scp, "Name", v2->method->name);
	rpt_value_free(v2);
	s =
	    subst_intl
	    (
	       	scp,
		i18n("boolean value required for logical or (was given $name)")
	    );
	sub_context_delete(scp);
	v2b = rpt_value_error(0, s);
	str_free(s);
	return v2b;
    }
    rpt_value_free(v2);
    return v2b;
}


static tree_method_ty or_method =
{
    sizeof(tree_diadic_ty),
    "||",
    tree_diadic_destructor,
    tree_diadic_print,
    or_evaluate,
    tree_diadic_useful,
    tree_diadic_constant,
    tree_diadic_optimize,
};


tree_ty *
tree_or_new(tree_ty *left, tree_ty *right)
{
    return tree_diadic_new(&or_method, left, right);
}


static rpt_value_ty *
not_evaluate(tree_ty *tp, string_ty *path, struct stat *st)
{
    tree_monadic_ty *this_thing;
    rpt_value_ty    *v1;
    rpt_value_ty    *v2;
    rpt_value_ty    *v3;

    this_thing = (tree_monadic_ty *)tp;
    v1 = tree_evaluate(this_thing->arg, path, st);
    if (v1->method->type == rpt_value_type_error)
	return v1;
    v2 = rpt_value_booleanize(v1);
    if (v2->method->type != rpt_value_type_boolean)
    {
	sub_context_ty	*scp;
	string_ty	*s;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name", v1->method->name);
	s = subst_intl(scp, i18n("illegal logical not ($name)"));
	sub_context_delete(scp);
	v3 = rpt_value_error(0, s);
	str_free(s);
    }
    else
    {
	v3 = rpt_value_boolean(!rpt_value_boolean_query(v2));
    }
    rpt_value_free(v1);
    rpt_value_free(v2);
    return v3;
}


static tree_method_ty not_method =
{
    sizeof(tree_monadic_ty),
    "!",
    tree_monadic_destructor,
    tree_monadic_print,
    not_evaluate,
    tree_monadic_useful,
    tree_monadic_constant,
    tree_monadic_optimize,
};


tree_ty *
tree_not_new(tree_ty *arg)
{
    return tree_monadic_new(&not_method, arg);
}


static rpt_value_ty *
comma_evaluate(tree_ty *tp, string_ty *path, struct stat *st)
{
    tree_diadic_ty  *this_thing;
    rpt_value_ty    *vp;

    this_thing = (tree_diadic_ty *)tp;

    vp = tree_evaluate(this_thing->left, path, st);
    if (vp->method->type == rpt_value_type_error)
	return vp;
    rpt_value_free(vp);

    return tree_evaluate(this_thing->right, path, st);
}


static tree_method_ty comma_method =
{
    sizeof(tree_diadic_ty),
    ",",
    tree_diadic_destructor,
    tree_diadic_print,
    comma_evaluate,
    tree_diadic_useful,
    tree_diadic_constant,
    tree_diadic_optimize,
};


tree_ty *
tree_comma_new(tree_ty *left, tree_ty *right)
{
    return tree_diadic_new(&comma_method, left, right);
}


typedef struct triadic_ty triadic_ty;
struct triadic_ty
{
    tree_ty         inherited;
    tree_ty	    *arg1;
    tree_ty	    *arg2;
    tree_ty	    *arg3;
};


static void
triadic_destructor(tree_ty *tp)
{
    triadic_ty      *this_thing;

    this_thing = (triadic_ty *)tp;
    tree_delete(this_thing->arg1);
    tree_delete(this_thing->arg2);
    tree_delete(this_thing->arg3);
}


static void
triadic_print(tree_ty *tp)
{
    triadic_ty      *this_thing;

    this_thing = (triadic_ty *)tp;
    printf("( ( ");
    tree_print(this_thing->arg1);
    printf(" ) ? ( ");
    tree_print(this_thing->arg2);
    printf(" ) : ( ");
    tree_print(this_thing->arg3);
    printf(" ) )");
}


static rpt_value_ty *
triadic_evaluate(tree_ty *tp, string_ty *path, struct stat *st)
{
    triadic_ty      *this_thing;
    rpt_value_ty    *v1;
    rpt_value_ty    *v1b;
    int             which;

    this_thing = (triadic_ty *)tp;

    v1 = tree_evaluate(this_thing->arg1, path, st);
    if (v1->method->type == rpt_value_type_error)
	return v1;
    v1b = rpt_value_booleanize(v1);
    if (v1b->method->type != rpt_value_type_boolean)
    {
	sub_context_ty	*scp;
	string_ty	*s;

	scp = sub_context_new();
	rpt_value_free(v1b);
	sub_var_set_charstar(scp, "Name", v1->method->name);
	rpt_value_free(v1);
	s =
	    subst_intl
	    (
	       	scp,
		i18n("boolean value required for if (was given $name)")
	    );
	sub_context_delete(scp);
	v1b = rpt_value_error(0, s);
	str_free(s);
	return v1b;
    }
    rpt_value_free(v1);
    which = rpt_value_boolean_query(v1b);
    rpt_value_free(v1b);

    return tree_evaluate(
        (which ? this_thing->arg2 : this_thing->arg3), path, st);
}


static int
triadic_useful(tree_ty *tp)
{
    triadic_ty      *this_thing;

    this_thing = (triadic_ty *)tp;
    return
    (
	tree_useful(this_thing->arg1)
    ||
	tree_useful(this_thing->arg2)
    ||
	tree_useful(this_thing->arg3)
    );
}


static int
triadic_constant(tree_ty *tp)
{
    triadic_ty      *this_thing;
    rpt_value_ty    *v1;
    rpt_value_ty    *v1b;
    int             which;

    this_thing = (triadic_ty *)tp;

    if (!tree_constant(this_thing->arg1))
	return 0;
    v1 = tree_evaluate_constant(this_thing->arg1);
    if (v1->method->type == rpt_value_type_error)
    {
	rpt_value_free(v1);
	return 0;
    }
    v1b = rpt_value_booleanize(v1);
    rpt_value_free(v1);
    if (v1b->method->type != rpt_value_type_boolean)
    {
	rpt_value_free(v1b);
	return 0;
    }
    which = rpt_value_boolean_query(v1b);
    rpt_value_free(v1b);

    return tree_constant(which ? this_thing->arg2 : this_thing->arg3);
}


static tree_ty *
triadic_optimize(tree_ty *tp)
{
    triadic_ty      *this_thing;
    rpt_value_ty    *v1;
    rpt_value_ty    *v1b;
    int             which;
    tree_ty         *result;

    this_thing = (triadic_ty *)tp;

    if (!tree_constant(this_thing->arg1))
    {
	tree_ty		*arg1;
	tree_ty		*arg2;
	tree_ty		*arg3;

	arg1 = tree_optimize(this_thing->arg1);
	arg2 = tree_optimize(this_thing->arg2);
	arg3 = tree_optimize(this_thing->arg3);
	result = tree_triadic_new(arg1, arg2, arg3);
	tree_delete(arg1);
	tree_delete(arg2);
	tree_delete(arg3);
	return result;
    }
    v1 = tree_evaluate_constant(this_thing->arg1);
    if (v1->method->type == rpt_value_type_error)
    {
	result = tree_constant_new(v1);
	rpt_value_free(v1);
	return result;
    }
    v1b = rpt_value_booleanize(v1);
    if (v1b->method->type != rpt_value_type_boolean)
    {
	sub_context_ty	*scp;
	string_ty	*s;

	scp = sub_context_new();
	rpt_value_free(v1b);
	sub_var_set_charstar(scp, "Name", v1->method->name);
	rpt_value_free(v1);
	s =
	    subst_intl
	    (
	       	scp,
		i18n("boolean value required for if (was given $name)")
	    );
	sub_context_delete(scp);
	v1 = rpt_value_error(0, s);
	str_free(s);
	result = tree_constant_new(v1);
	rpt_value_free(v1);
	return result;
    }
    rpt_value_free(v1);
    which = rpt_value_boolean_query(v1b);
    rpt_value_free(v1b);

    return tree_optimize(which ? this_thing->arg2 : this_thing->arg3);
}


static tree_method_ty triadic_method =
{
    sizeof(tree_diadic_ty),
    "?:",
    triadic_destructor,
    triadic_print,
    triadic_evaluate,
    triadic_useful,
    triadic_constant,
    triadic_optimize,
};


tree_ty *
tree_triadic_new(tree_ty *arg1, tree_ty *arg2, tree_ty *arg3)
{
    tree_ty         *tp;
    triadic_ty      *this_thing;

    tp = tree_new(&triadic_method);
    this_thing = (triadic_ty *)tp;
    this_thing->arg1 = tree_copy(arg1);
    this_thing->arg2 = tree_copy(arg2);
    this_thing->arg3 = tree_copy(arg3);
    return tp;
}
