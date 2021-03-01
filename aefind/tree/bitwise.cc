//
//	aegis - project change supervisor
//	Copyright (C) 1997, 1999, 2002-2004 Peter Miller;
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
// MANIFEST: functions to manipulate bitwise operators
//

#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <error.h>
#include <sub.h>
#include <trace.h>
#include <tree/bitwise.h>
#include <tree/diadic.h>
#include <tree/monadic.h>


static rpt_value_ty *
bitwise_and_evaluate(tree_ty *tp, string_ty *path, struct stat *st)
{
    tree_diadic_ty  *this_thing;
    rpt_value_ty    *v1;
    rpt_value_ty    *v1i;
    rpt_value_ty    *v2;
    rpt_value_ty    *v2i;
    rpt_value_ty    *result;

    this_thing = (tree_diadic_ty *)tp;
    v1 = tree_evaluate(this_thing->left, path, st);
    if (v1->method->type == rpt_value_type_error)
	return v1;
    v1i = rpt_value_integerize(v1);
    if (v1i->method->type != rpt_value_type_integer)
    {
	sub_context_ty	*scp;
	string_ty	*s;

	scp = sub_context_new();
	rpt_value_free(v1i);
	sub_var_set_charstar(scp, "Name", v1->method->name);
	rpt_value_free(v1);
	s =
	    subst_intl
	    (
	       	scp,
		i18n("integer value required for bit and (was given $name)")
	    );
	sub_context_delete(scp);
	result = rpt_value_error(0, s);
	str_free(s);
	return result;
    }
    rpt_value_free(v1);

    v2 = tree_evaluate(this_thing->right, path, st);
    if (v2->method->type == rpt_value_type_error)
    {
	rpt_value_free(v1i);
	return v2;
    }
    v2i = rpt_value_integerize(v2);
    if (v2i->method->type != rpt_value_type_integer)
    {
	sub_context_ty	*scp;
	string_ty	*s;

	rpt_value_free(v1i);
	rpt_value_free(v2i);
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name", v2->method->name);
	s =
	    subst_intl
	    (
	       	scp,
		i18n("integer value required for bit and (was given $name)")
	    );
	sub_context_delete(scp);
	result = rpt_value_error(0, s);
	str_free(s);
	rpt_value_free(v2);
	return result;
    }
    rpt_value_free(v2);

    result =
	rpt_value_integer
	(
    	    rpt_value_integer_query(v1i) & rpt_value_integer_query(v2i)
	);
    rpt_value_free(v1i);
    rpt_value_free(v2i);
    return result;
}


static tree_method_ty bitwise_and_method =
{
    sizeof(tree_diadic_ty),
    "&",
    tree_diadic_destructor,
    tree_diadic_print,
    bitwise_and_evaluate,
    tree_diadic_useful,
    tree_diadic_constant,
    tree_diadic_optimize,
};


tree_ty *
tree_bitwise_and_new(tree_ty *left, tree_ty *right)
{
    return tree_diadic_new(&bitwise_and_method, left, right);
}


static rpt_value_ty *
bitwise_xor_evaluate(tree_ty *tp, string_ty *path, struct stat *st)
{
    tree_diadic_ty  *this_thing;
    rpt_value_ty    *v1;
    rpt_value_ty    *v1i;
    rpt_value_ty    *v2;
    rpt_value_ty    *v2i;
    rpt_value_ty    *result;

    this_thing = (tree_diadic_ty *)tp;
    v1 = tree_evaluate(this_thing->left, path, st);
    if (v1->method->type == rpt_value_type_error)
	return v1;
    v1i = rpt_value_integerize(v1);
    if (v1i->method->type != rpt_value_type_integer)
    {
	sub_context_ty	*scp;
	string_ty	*s;

	scp = sub_context_new();
	rpt_value_free(v1i);
	sub_var_set_charstar(scp, "Name", v1->method->name);
	rpt_value_free(v1);
	s =
	    subst_intl
	    (
	       	scp,
		i18n("integer value required for bit xor (was given $name)")
	    );
	sub_context_delete(scp);
	result = rpt_value_error(0, s);
	str_free(s);
	return result;
    }
    rpt_value_free(v1);

    v2 = tree_evaluate(this_thing->right, path, st);
    if (v2->method->type == rpt_value_type_error)
    {
	rpt_value_free(v1i);
	return v2;
    }
    v2i = rpt_value_integerize(v2);
    if (v2i->method->type != rpt_value_type_integer)
    {
	sub_context_ty	*scp;
	string_ty	*s;

	scp = sub_context_new();
	rpt_value_free(v1i);
	rpt_value_free(v2i);
	sub_var_set_charstar(scp, "Name", v2->method->name);
	rpt_value_free(v2);
	s =
	    subst_intl
	    (
	       	scp,
		i18n("integer value required for bit xor (was given $name)")
	    );
	sub_context_delete(scp);
	result = rpt_value_error(0, s);
	str_free(s);
	return result;
    }
    rpt_value_free(v2);

    result =
	rpt_value_integer
	(
    	    rpt_value_integer_query(v1i) ^ rpt_value_integer_query(v2i)
	);
    rpt_value_free(v1i);
    rpt_value_free(v2i);
    return result;
}


static tree_method_ty bitwise_xor_method =
{
    sizeof(tree_diadic_ty),
    "^",
    tree_diadic_destructor,
    tree_diadic_print,
    bitwise_xor_evaluate,
    tree_diadic_useful,
    tree_diadic_constant,
    tree_diadic_optimize,
};


tree_ty *
tree_bitwise_xor_new(tree_ty *left, tree_ty *right)
{
    return tree_diadic_new(&bitwise_xor_method, left, right);
}


static rpt_value_ty *
bitwise_or_evaluate(tree_ty *tp, string_ty *path, struct stat *st)
{
    tree_diadic_ty  *this_thing;
    rpt_value_ty    *v1;
    rpt_value_ty    *v1i;
    rpt_value_ty    *v2;
    rpt_value_ty    *v2i;
    rpt_value_ty    *result;

    this_thing = (tree_diadic_ty *)tp;
    v1 = tree_evaluate(this_thing->left, path, st);
    if (v1->method->type == rpt_value_type_error)
	return v1;
    v1i = rpt_value_integerize(v1);
    if (v1i->method->type != rpt_value_type_integer)
    {
	sub_context_ty	*scp;
	string_ty	*s;

	scp = sub_context_new();
	rpt_value_free(v1i);
	sub_var_set_charstar(scp, "Name", v1->method->name);
	rpt_value_free(v1);
	s =
	    subst_intl
	    (
	       	scp,
		i18n("integer value required for bit or (was given $name)")
	    );
	sub_context_delete(scp);
	result = rpt_value_error(0, s);
	str_free(s);
	return result;
    }
    rpt_value_free(v1);

    v2 = tree_evaluate(this_thing->right, path, st);
    if (v2->method->type == rpt_value_type_error)
    {
	rpt_value_free(v1i);
	return v2;
    }
    v2i = rpt_value_integerize(v2);
    if (v2i->method->type != rpt_value_type_integer)
    {
	sub_context_ty	*scp;
	string_ty	*s;

	scp = sub_context_new();
	rpt_value_free(v1i);
	rpt_value_free(v2i);
	sub_var_set_charstar(scp, "Name", v2->method->name);
	rpt_value_free(v2);
	s =
	    subst_intl
	    (
	       	scp,
		i18n("integer value required for bit or (was given $name)")
	    );
	sub_context_delete(scp);
	result = rpt_value_error(0, s);
	str_free(s);
	return result;
    }
    rpt_value_free(v2);

    result =
	rpt_value_integer
	(
    	    rpt_value_integer_query(v1i) | rpt_value_integer_query(v2i)
	);
    rpt_value_free(v1i);
    rpt_value_free(v2i);
    return result;
}


static tree_method_ty bitwise_or_method =
{
    sizeof(tree_diadic_ty),
    "|",
    tree_diadic_destructor,
    tree_diadic_print,
    bitwise_or_evaluate,
    tree_diadic_useful,
    tree_diadic_constant,
    tree_diadic_optimize,
};


tree_ty *
tree_bitwise_or_new(tree_ty *left, tree_ty *right)
{
    return tree_diadic_new(&bitwise_or_method, left, right);
}


static rpt_value_ty *
bitwise_not_evaluate(tree_ty *tp, string_ty *path, struct stat *st)
{
    tree_monadic_ty *this_thing;
    rpt_value_ty    *v1;
    rpt_value_ty    *v2;
    rpt_value_ty    *vp;

    //
    // evaluate the argument
    //
    trace(("not::evaluate()\n{\n"));
    this_thing = (tree_monadic_ty *)tp;
    v1 = tree_evaluate(this_thing->arg, path, st);
    if (v1->method->type == rpt_value_type_error)
    {
	trace(("}\n"));
	return v1;
    }

    //
    // coerce the argument to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    v2 = rpt_value_integerize(v1);
    rpt_value_free(v1);

    //
    // the type of the result depends on
    // the types of the argument
    //
    if (v2->method->type == rpt_value_type_integer)
	vp = rpt_value_integer(~rpt_value_integer_query(v2));
    else
    {
	sub_context_ty	*scp;
	string_ty	*s;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name", v2->method->name);
	s = subst_intl(scp, i18n("illegal bit not ($name)"));
	sub_context_delete(scp);
	vp = rpt_value_error(0, s);
	str_free(s);
    }
    rpt_value_free(v2);
    trace(("return %08lX;\n", (long)vp));
    trace(("}\n"));
    return vp;
}


static tree_method_ty bitwise_not_method =
{
    sizeof(tree_monadic_ty),
    "~",
    tree_monadic_destructor,
    tree_monadic_print,
    bitwise_not_evaluate,
    tree_monadic_useful,
    tree_monadic_constant,
    tree_monadic_optimize,
};


tree_ty *
tree_bitwise_not_new(tree_ty *arg)
{
    return tree_monadic_new(&bitwise_not_method, arg);
}


static rpt_value_ty *
shift_left_evaluate(tree_ty *tp, string_ty *path, struct stat *st)
{
    tree_diadic_ty  *this_thing;
    sub_context_ty  *scp;
    rpt_value_ty    *v1;
    rpt_value_ty    *v1i;
    unsigned long   v1n;
    rpt_value_ty    *v2;
    rpt_value_ty    *v2i;
    long	    v2n;
    rpt_value_ty    *result;

    this_thing = (tree_diadic_ty *)tp;
    v1 = tree_evaluate(this_thing->left, path, st);
    if (v1->method->type == rpt_value_type_error)
	return v1;
    v1i = rpt_value_integerize(v1);
    rpt_value_free(v1);

    v2 = tree_evaluate(this_thing->right, path, st);
    if (v2->method->type == rpt_value_type_error)
	return v2;
    v2i = rpt_value_integerize(v2);
    rpt_value_free(v2);

    if
    (
	v1i->method->type != rpt_value_type_integer
    ||
	v2i->method->type != rpt_value_type_integer
    )
    {
	string_ty	*s;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name1", v1i->method->name);
	sub_var_set_charstar(scp, "Name2", v2i->method->name);
	s = subst_intl(scp, i18n("illegal shift ($name1 << $name2)"));
	sub_context_delete(scp);
	result = rpt_value_error(0, s);
	str_free(s);
	return result;
    }

    v1n = rpt_value_integer_query(v1i);
    v2n = rpt_value_integer_query(v2i);
    rpt_value_free(v1i);
    rpt_value_free(v2i);
    return rpt_value_integer(v1n << v2n);
}


static tree_method_ty shift_left_method =
{
    sizeof(tree_diadic_ty),
    "<<",
    tree_diadic_destructor,
    tree_diadic_print,
    shift_left_evaluate,
    tree_diadic_useful,
    tree_diadic_constant,
    tree_diadic_optimize,
};


tree_ty *
tree_shift_left_new(tree_ty *left, tree_ty *right)
{
    return tree_diadic_new(&shift_left_method, left, right);
}


static rpt_value_ty *
shift_right_evaluate(tree_ty *tp, string_ty *path, struct stat *st)
{
    tree_diadic_ty  *this_thing;
    sub_context_ty  *scp;
    rpt_value_ty    *v1;
    rpt_value_ty    *v1i;
    unsigned long   v1n;
    rpt_value_ty    *v2;
    rpt_value_ty    *v2i;
    long	    v2n;
    rpt_value_ty    *result;

    this_thing = (tree_diadic_ty *)tp;
    v1 = tree_evaluate(this_thing->left, path, st);
    if (v1->method->type == rpt_value_type_error)
	return v1;
    v1i = rpt_value_integerize(v1);
    rpt_value_free(v1);

    v2 = tree_evaluate(this_thing->right, path, st);
    if (v2->method->type == rpt_value_type_error)
	return v2;
    v2i = rpt_value_integerize(v2);
    rpt_value_free(v2);

    if
    (
	v1i->method->type != rpt_value_type_integer
    ||
	v2i->method->type != rpt_value_type_integer
    )
    {
	string_ty	*s;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name1", v1i->method->name);
	sub_var_set_charstar(scp, "Name2", v2i->method->name);
	s = subst_intl(scp, i18n("illegal shift ($name1 >> $name2)"));
	sub_context_delete(scp);
	result = rpt_value_error(0, s);
	str_free(s);
	return result;
    }

    v1n = rpt_value_integer_query(v1i);
    v2n = rpt_value_integer_query(v2i);
    rpt_value_free(v1i);
    rpt_value_free(v2i);
    return rpt_value_integer(v1n >> v2n);
}


static tree_method_ty shift_right_method =
{
    sizeof(tree_diadic_ty),
    ">>",
    tree_diadic_destructor,
    tree_diadic_print,
    shift_right_evaluate,
    tree_diadic_useful,
    tree_diadic_constant,
    tree_diadic_optimize,
};


tree_ty *
tree_shift_right_new(tree_ty *left, tree_ty *right)
{
    return tree_diadic_new(&shift_right_method, left, right);
}
