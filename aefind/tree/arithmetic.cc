//
//	aegis - project change supervisor
//	Copyright (C) 1997, 1999, 2002-2005 Peter Miller;
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
// MANIFEST: functions to manipulate arithmetics
//

#include <ac/math.h>

#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <aer/value/real.h>
#include <aer/value/string.h>
#include <error.h> // for assert
#include <str.h>
#include <sub.h>
#include <trace.h>
#include <tree/arithmetic.h>
#include <tree/diadic.h>
#include <tree/monadic.h>


#define PAIR(a, b)	((a) * rpt_value_type_MAX + (b))


static rpt_value_ty *
mul_evaluate(tree_ty *tp, string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st)
{
    tree_diadic_ty  *this_thing;
    sub_context_ty  *scp;
    rpt_value_ty    *v1;
    rpt_value_ty    *v1a;
    rpt_value_ty    *v2;
    rpt_value_ty    *v2a;
    rpt_value_ty    *vp;
    string_ty       *s;

    //
    // evaluate the left hand side
    //
    trace(("mul::evaluate()\n{\n"));
    this_thing = (tree_diadic_ty *)tp;
    v1 = tree_evaluate(this_thing->left, path_unres, path, path_res, st);
    if (v1->method->type == rpt_value_type_error)
    {
	trace(("}\n"));
	return v1;
    }

    //
    // coerce the left hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    v1a = rpt_value_arithmetic(v1);
    rpt_value_free(v1);

    //
    // evaluate the right hand side
    //
    v2 = tree_evaluate(this_thing->right, path_unres, path, path_res, st);
    if (v2->method->type == rpt_value_type_error)
    {
	rpt_value_free(v1a);
	trace(("}\n"));
	return v2;
    }

    //
    // coerce the right hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    v2a = rpt_value_arithmetic(v2);
    rpt_value_free(v2);

    //
    // the type of the result depends on
    // the types of the operands
    //
    switch (PAIR(v1a->method->type, v2a->method->type))
    {
    case PAIR(rpt_value_type_real, rpt_value_type_real):
	vp =
	    rpt_value_real
	    (
	       	rpt_value_real_query(v1a) * rpt_value_real_query(v2a)
	    );
	trace(("vp = %g real\n", rpt_value_real_query(vp)));
	break;

    case PAIR(rpt_value_type_real, rpt_value_type_integer):
	vp =
	    rpt_value_real
	    (
	       	rpt_value_real_query(v1a) * rpt_value_integer_query(v2a)
	    );
	trace(("vp = %g real\n", rpt_value_real_query(vp)));
	break;

    case PAIR(rpt_value_type_integer, rpt_value_type_real):
	vp =
	    rpt_value_real
	    (
	       	rpt_value_integer_query(v1a) * rpt_value_real_query(v2a)
	    );
	trace(("vp = %g real\n", rpt_value_real_query(vp)));
	break;

    case PAIR(rpt_value_type_integer, rpt_value_type_integer):
	vp =
	    rpt_value_integer
	    (
	       	rpt_value_integer_query(v1a) * rpt_value_integer_query(v2a)
	    );
	trace(("vp = %ld integer\n", rpt_value_integer_query(vp)));
	break;

    default:
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name1", v1a->method->name);
	sub_var_set_charstar(scp, "Name2", v2a->method->name);
	s = subst_intl(scp, i18n("illegal multiplication ($name1 * $name2)"));
	sub_context_delete(scp);
	vp = rpt_value_error(0, s);
	str_free(s);
	break;
    }
    rpt_value_free(v1a);
    rpt_value_free(v2a);
    trace(("return %08lX;\n", (long)vp));
    trace(("}\n"));
    return vp;
}


static tree_method_ty mul_method =
{
    sizeof(tree_diadic_ty),
    "*",
    tree_diadic_destructor,
    tree_diadic_print,
    mul_evaluate,
    tree_diadic_useful,
    tree_diadic_constant,
    tree_diadic_optimize,
};


tree_ty *
tree_mul_new(tree_ty *left, tree_ty *right)
{
    return tree_diadic_new(&mul_method, left, right);
}


static rpt_value_ty *
divide_evaluate(tree_ty *tp, string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st)
{
    tree_diadic_ty  *this_thing;
    sub_context_ty  *scp;
    rpt_value_ty    *v1;
    rpt_value_ty    *v1a;
    rpt_value_ty    *v2;
    rpt_value_ty    *v2a;
    double	    den;
    long	    lden;
    rpt_value_ty    *result;
    string_ty       *s;

    //
    // evaluate the left hand side
    //
    trace(("divide::evaluate()\n{\n"));
    this_thing = (tree_diadic_ty *)tp;
    v1 = tree_evaluate(this_thing->left, path_unres, path, path_res, st);
    if (v1->method->type == rpt_value_type_error)
    {
	trace(("}\n"));
	return v1;
    }

    //
    // coerce the left hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    v1a = rpt_value_arithmetic(v1);
    rpt_value_free(v1);

    //
    // evaluate the right hand side
    //
    v2 = tree_evaluate(this_thing->right, path_unres, path, path_res, st);
    if (v2->method->type == rpt_value_type_error)
    {
	rpt_value_free(v1a);
	trace(("}\n"));
	return v2;
    }

    //
    // coerce the right hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    v2a = rpt_value_arithmetic(v2);
    rpt_value_free(v2);

    //
    // the type of the result depends on
    // the types of the operands
    //
    switch (PAIR(v1a->method->type, v2a->method->type))
    {
    case PAIR(rpt_value_type_real, rpt_value_type_real):
	den = rpt_value_real_query(v2a);
	if (den == 0)
	{
	    div_by_zero:
	    scp = sub_context_new();
	    s = subst_intl(scp, i18n("division by zero"));
	    sub_context_delete(scp);
	    result = rpt_value_error(0, s);
	    str_free(s);
	    break;
	}
	result = rpt_value_real(rpt_value_real_query(v1a) / den);
	trace(("result = %g real\n", rpt_value_real_query(result)));
	break;

    case PAIR(rpt_value_type_real, rpt_value_type_integer):
	lden = rpt_value_integer_query(v2a);
	if (lden == 0)
	    goto div_by_zero;
	result = rpt_value_real(rpt_value_real_query(v1a) / lden);
	trace(("result = %g real\n", rpt_value_real_query(result)));
	break;

    case PAIR(rpt_value_type_integer, rpt_value_type_real):
	den = rpt_value_real_query(v2a);
	if (den == 0)
	    goto div_by_zero;
	result = rpt_value_real(rpt_value_integer_query(v1a) / den);
	trace(("result = %g real\n", rpt_value_real_query(result)));
	break;

    case PAIR(rpt_value_type_integer, rpt_value_type_integer):
	lden = rpt_value_integer_query(v2a);
	if (lden == 0)
	    goto div_by_zero;
	result = rpt_value_integer(rpt_value_integer_query(v1a) / lden);
	trace(("result = %ld integer\n",
		rpt_value_integer_query(result)));
	break;

    default:
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name1", v1a->method->name);
	sub_var_set_charstar(scp, "Name2", v2a->method->name);
	s = subst_intl(scp, i18n("illegal division ($name1 / $name2)"));
	sub_context_delete(scp);
	result = rpt_value_error(0, s);
	str_free(s);
	break;
    }
    rpt_value_free(v1a);
    rpt_value_free(v2a);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static tree_method_ty divide_method =
{
    sizeof(tree_diadic_ty),
    "/",
    tree_diadic_destructor,
    tree_diadic_print,
    divide_evaluate,
    tree_diadic_useful,
    tree_diadic_constant,
    tree_diadic_optimize,
};


tree_ty *
tree_divide_new(tree_ty *left, tree_ty *right)
{
    return tree_diadic_new(&divide_method, left, right);
}


static rpt_value_ty *
mod_evaluate(tree_ty *tp, string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st)
{
    tree_diadic_ty  *this_thing;
    sub_context_ty  *scp;
    rpt_value_ty    *v1;
    rpt_value_ty    *v1a;
    rpt_value_ty    *v2;
    rpt_value_ty    *v2a;
    double	    den;
    long	    lden;
    rpt_value_ty    *result;
    string_ty       *s;

    //
    // evaluate the left hand side
    //
    trace(("mod::evaluate()\n{\n"));
    this_thing = (tree_diadic_ty *)tp;
    v1 = tree_evaluate(this_thing->left, path_unres, path, path_res, st);
    if (v1->method->type == rpt_value_type_error)
    {
	trace(("}\n"));
	return v1;
    }

    //
    // coerce the left hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    v1a = rpt_value_arithmetic(v1);
    rpt_value_free(v1);

    //
    // evaluate the right hand side
    //
    v2 = tree_evaluate(this_thing->right, path_unres, path, path_res, st);
    if (v2->method->type == rpt_value_type_error)
    {
	rpt_value_free(v1a);
	trace(("}\n"));
	return v2;
    }

    //
    // coerce the right hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    v2a = rpt_value_arithmetic(v2);
    rpt_value_free(v2);

    //
    // the type of the result depends on
    // the types of the operands
    //
    switch (PAIR(v1a->method->type, v2a->method->type))
    {
    case PAIR(rpt_value_type_real, rpt_value_type_real):
	den = rpt_value_real_query(v2a);
	if (den == 0)
	{
	    mod_by_zero:
	    scp = sub_context_new();
	    s = subst_intl(scp, i18n("modulo by zero"));
	    sub_context_delete(scp);
	    result = rpt_value_error(0, s);
	    str_free(s);
	    break;
	}
	result = rpt_value_real(fmod(rpt_value_real_query(v1a), den));
	trace(("result = %g real\n", rpt_value_real_query(result)));
	break;

    case PAIR(rpt_value_type_real, rpt_value_type_integer):
	den = rpt_value_integer_query(v2a);
	if (den == 0)
	    goto mod_by_zero;
	result = rpt_value_real(fmod(rpt_value_real_query(v1a), den));
	trace(("result = %g real\n", rpt_value_real_query(result)));
	break;

    case PAIR(rpt_value_type_integer, rpt_value_type_real):
	den = rpt_value_real_query(v2a);
	if (den == 0)
	    goto mod_by_zero;
	result =
	    rpt_value_real(fmod((double)rpt_value_integer_query(v1a), den));
	trace(("result = %g real\n", rpt_value_real_query(result)));
	break;

    case PAIR(rpt_value_type_integer, rpt_value_type_integer):
	lden = rpt_value_integer_query(v2a);
	if (lden == 0)
	    goto mod_by_zero;
	result = rpt_value_integer(rpt_value_integer_query(v1a) % lden);
	trace(("result = %ld integer\n", rpt_value_integer_query(result)));
	break;

    default:
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name1", v1a->method->name);
	sub_var_set_charstar(scp, "Name2", v2a->method->name);
	// xgettext:no-c-format
	s = subst_intl(scp, i18n("illegal modulo ($name1 % $name2)"));
	sub_context_delete(scp);
	result = rpt_value_error(0, s);
	str_free(s);
	break;
    }
    rpt_value_free(v1a);
    rpt_value_free(v2a);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static tree_method_ty mod_method =
{
    sizeof(tree_diadic_ty),
    "%",
    tree_diadic_destructor,
    tree_diadic_print,
    mod_evaluate,
    tree_diadic_useful,
    tree_diadic_constant,
    tree_diadic_optimize,
};


tree_ty *
tree_mod_new(tree_ty *left, tree_ty *right)
{
    return tree_diadic_new(&mod_method, left, right);
}


static rpt_value_ty *
neg_evaluate(tree_ty *tp, string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st)
{
    tree_monadic_ty *this_thing;
    sub_context_ty  *scp;
    rpt_value_ty    *v1;
    rpt_value_ty    *v2;
    rpt_value_ty    *vp;
    string_ty       *s;

    //
    // evaluate the argument
    //
    trace(("neg::evaluate()\n{\n"));
    this_thing = (tree_monadic_ty *)tp;
    v1 = tree_evaluate(this_thing->arg, path_unres, path, path_res, st);
    if (v1->method->type == rpt_value_type_error)
    {
	trace(("}\n"));
	return v1;
    }

    //
    // coerce the argument to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    v2 = rpt_value_arithmetic(v1);
    rpt_value_free(v1);

    //
    // the type of the result depends on
    // the types of the argument
    //
    switch (v2->method->type)
    {
    case rpt_value_type_real:
	vp = rpt_value_real(-rpt_value_real_query(v2));
	trace(("vp = %g real\n", rpt_value_real_query(vp)));
	break;

    case rpt_value_type_integer:
	vp = rpt_value_integer(-rpt_value_integer_query(v2));
	trace(("vp = %ld integer\n", rpt_value_integer_query(vp)));
	break;

    default:
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name", v2->method->name);
	s = subst_intl(scp, i18n("illegal negative ($name)"));
	sub_context_delete(scp);
	vp = rpt_value_error(0, s);
	str_free(s);
	break;
    }
    rpt_value_free(v2);
    trace(("return %08lX;\n", (long)vp));
    trace(("}\n"));
    return vp;
}


static tree_method_ty neg_method =
{
    sizeof(tree_monadic_ty),
    "-",
    tree_monadic_destructor,
    tree_monadic_print,
    neg_evaluate,
    tree_monadic_useful,
    tree_monadic_constant,
    tree_monadic_optimize,
};


tree_ty *
tree_neg_new(tree_ty *arg)
{
    return tree_monadic_new(&neg_method, arg);
}


static rpt_value_ty *
pos_evaluate(tree_ty *tp, string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st)
{
    tree_monadic_ty *this_thing;
    sub_context_ty  *scp;
    rpt_value_ty    *v1;
    rpt_value_ty    *v2;
    rpt_value_ty    *vp;
    string_ty       *s;

    //
    // evaluate the argument
    //
    trace(("pos::evaluate()\n{\n"));
    this_thing = (tree_monadic_ty *)tp;
    v1 = tree_evaluate(this_thing->arg, path_unres, path, path_res, st);
    if (v1->method->type == rpt_value_type_error)
    {
	trace(("}\n"));
	return v1;
    }

    //
    // coerce the argument to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    v2 = rpt_value_arithmetic(v1);
    rpt_value_free(v1);

    //
    // the type of the result depends on
    // the types of the argument
    //
    switch (v2->method->type)
    {
    case rpt_value_type_real:
    case rpt_value_type_integer:
	vp = rpt_value_copy(v2);
	break;

    default:
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name", v2->method->name);
	s = subst_intl(scp, i18n("illegal positive ($name)"));
	sub_context_delete(scp);
	vp = rpt_value_error(0, s);
	str_free(s);
	break;
    }
    rpt_value_free(v2);
    trace(("return %08lX;\n", (long)vp));
    trace(("}\n"));
    return vp;
}


static tree_method_ty pos_method =
{
    sizeof(tree_monadic_ty),
    "+",
    tree_monadic_destructor,
    tree_monadic_print,
    pos_evaluate,
    tree_monadic_useful,
    tree_monadic_constant,
    tree_monadic_optimize,
};


tree_ty *
tree_pos_new(tree_ty *arg)
{
    return tree_monadic_new(&pos_method, arg);
}


static rpt_value_ty *
plus_evaluate(tree_ty *tp, string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st)
{
    tree_diadic_ty  *this_thing;
    sub_context_ty  *scp;
    rpt_value_ty    *lv1;
    rpt_value_ty    *lv2;
    rpt_value_ty    *rv1;
    rpt_value_ty    *rv2;
    rpt_value_ty    *vp;
    string_ty       *s;

    //
    // evaluate the left hand side
    //
    trace(("plus::evaluate()\n{\n"));
    this_thing = (tree_diadic_ty *)tp;
    lv1 = tree_evaluate(this_thing->left, path_unres, path, path_res, st);
    if (lv1->method->type == rpt_value_type_error)
    {
	trace(("}\n"));
	return lv1;
    }

    //
    // coerce the left hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    lv2 = rpt_value_arithmetic(lv1);
    rpt_value_free(lv1);

    //
    // evaluate the right hand side
    //
    rv1 = tree_evaluate(this_thing->right, path_unres, path, path_res, st);
    if (rv1->method->type == rpt_value_type_error)
    {
	rpt_value_free(lv2);
	trace(("}\n"));
	return rv1;
    }

    //
    // coerce the right hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    rv2 = rpt_value_arithmetic(rv1);
    rpt_value_free(rv1);

    //
    // the type of the result depends on
    // the types of the operands
    //
    switch (PAIR(lv2->method->type, rv2->method->type))
    {
    case PAIR(rpt_value_type_real, rpt_value_type_real):
	vp =
	    rpt_value_real
	    (
	       	rpt_value_real_query(lv2) + rpt_value_real_query(rv2)
	    );
	trace(("vp = %g real\n", rpt_value_real_query(vp)));
	break;

    case PAIR(rpt_value_type_real, rpt_value_type_integer):
	vp =
	    rpt_value_real
	    (
	       	rpt_value_real_query(lv2) + rpt_value_integer_query(rv2)
	    );
	trace(("vp = %g real\n", rpt_value_real_query(vp)));
	break;

    case PAIR(rpt_value_type_integer, rpt_value_type_real):
	vp =
	    rpt_value_real
	    (
	       	rpt_value_integer_query(lv2) + rpt_value_real_query(rv2)
	    );
	trace(("vp = %g real\n", rpt_value_real_query(vp)));
	break;

    case PAIR(rpt_value_type_integer, rpt_value_type_integer):
	vp =
	    rpt_value_integer
	    (
	       	rpt_value_integer_query(lv2) + rpt_value_integer_query(rv2)
	    );
	trace(("vp = %ld integer\n", rpt_value_integer_query(vp)));
	break;

    default:
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name1", lv2->method->name);
	sub_var_set_charstar(scp, "Name2", rv2->method->name);
	s = subst_intl(scp, i18n("illegal addition ($name1 + $name2)"));
	sub_context_delete(scp);
	vp = rpt_value_error(0, s);
	str_free(s);
	break;
    }
    rpt_value_free(lv2);
    rpt_value_free(rv2);
    trace(("return %08lX;\n", (long)vp));
    trace(("}\n"));
    return vp;
}


static tree_method_ty plus_method =
{
    sizeof(tree_diadic_ty),
    "+",
    tree_diadic_destructor,
    tree_diadic_print,
    plus_evaluate,
    tree_diadic_useful,
    tree_diadic_constant,
    tree_diadic_optimize,
};


tree_ty *
tree_plus_new(tree_ty *left, tree_ty *right)
{
    return tree_diadic_new(&plus_method, left, right);
}


static rpt_value_ty *
subtract_evaluate(tree_ty *tp, string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st)
{
    tree_diadic_ty  *this_thing;
    sub_context_ty  *scp;
    rpt_value_ty    *lv1;
    rpt_value_ty    *lv2;
    rpt_value_ty    *rv1;
    rpt_value_ty    *rv2;
    rpt_value_ty    *vp;
    string_ty       *s;

    //
    // evaluate the left hand side
    //
    trace(("subtract::evaluate()\n{\n"));
    this_thing = (tree_diadic_ty *)tp;
    lv1 = tree_evaluate(this_thing->left, path_unres, path, path_res, st);
    if (lv1->method->type == rpt_value_type_error)
    {
	trace(("}\n"));
	return lv1;
    }

    //
    // coerce the left hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    lv2 = rpt_value_arithmetic(lv1);
    rpt_value_free(lv1);

    //
    // evaluate the right hand side
    //
    rv1 = tree_evaluate(this_thing->right, path_unres, path, path_res, st);
    if (rv1->method->type == rpt_value_type_error)
    {
	rpt_value_free(lv2);
	trace(("}\n"));
	return rv1;
    }

    //
    // coerce the right hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    rv2 = rpt_value_arithmetic(rv1);
    rpt_value_free(rv1);

    //
    // the type of the result depends on
    // the types of the operands
    //
    switch (PAIR(lv2->method->type, rv2->method->type))
    {
    case PAIR(rpt_value_type_real, rpt_value_type_real):
	vp =
	    rpt_value_real
	    (
	       	rpt_value_real_query(lv2) - rpt_value_real_query(rv2)
	    );
	trace(("vp = %g real\n", rpt_value_real_query(vp)));
	break;

    case PAIR(rpt_value_type_real, rpt_value_type_integer):
	vp =
	    rpt_value_real
	    (
	       	rpt_value_real_query(lv2) - rpt_value_integer_query(rv2)
	    );
	trace(("vp = %g real\n", rpt_value_real_query(vp)));
	break;

    case PAIR(rpt_value_type_integer, rpt_value_type_real):
	vp =
	    rpt_value_real
	    (
	       	rpt_value_integer_query(lv2) - rpt_value_real_query(rv2)
	    );
	trace(("vp = %g real\n", rpt_value_real_query(vp)));
	break;

    case PAIR(rpt_value_type_integer, rpt_value_type_integer):
	vp =
	    rpt_value_integer
	    (
	       	rpt_value_integer_query(lv2) - rpt_value_integer_query(rv2)
	    );
	trace(("vp = %ld integer\n", rpt_value_integer_query(vp)));
	break;

    default:
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name1", lv2->method->name);
	sub_var_set_charstar(scp, "Name2", rv2->method->name);
	s = subst_intl(scp, i18n("illegal subtraction ($name1 - $name2)"));
	sub_context_delete(scp);
	vp = rpt_value_error(0, s);
	str_free(s);
	break;
    }
    rpt_value_free(lv2);
    rpt_value_free(rv2);
    trace(("return %08lX;\n", (long)vp));
    trace(("}\n"));
    return vp;
}


static tree_method_ty subtract_method =
{
    sizeof(tree_diadic_ty),
    "-",
    tree_diadic_destructor,
    tree_diadic_print,
    subtract_evaluate,
    tree_diadic_useful,
    tree_diadic_constant,
    tree_diadic_optimize,
};


tree_ty *
tree_subtract_new(tree_ty *left, tree_ty *right)
{
    return tree_diadic_new(&subtract_method, left, right);
}


static rpt_value_ty *
join_evaluate(tree_ty *tp, string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st)
{
    tree_diadic_ty  *this_thing;
    sub_context_ty  *scp;
    rpt_value_ty    *v1;
    rpt_value_ty    *v2;
    rpt_value_ty    *result;
    string_ty       *s;
    rpt_value_ty    *v1s;
    rpt_value_ty    *v2s;

    //
    // evaluate the left hand side
    //
    trace(("join::evaluate()\n{\n"));
    this_thing = (tree_diadic_ty *)tp;
    v1 = tree_evaluate(this_thing->left, path_unres, path, path_res, st);
    if (v1->method->type == rpt_value_type_error)
    {
	trace(("}\n"));
	return v1;
    }
    assert(v1->method->type != rpt_value_type_reference);

    //
    // evaluate the right hand side
    //
    v2 = tree_evaluate(this_thing->right, path_unres, path, path_res, st);
    if (v2->method->type == rpt_value_type_error)
    {
	rpt_value_free(v1);
	trace(("}\n"));
	return v2;
    }
    assert(v2->method->type != rpt_value_type_reference);

    //
    // must be a string join
    //
    v1s = rpt_value_stringize(v1);
    v2s = rpt_value_stringize(v2);
    if
    (
	v1s->method->type == rpt_value_type_string
    &&
	v2s->method->type == rpt_value_type_string
    )
    {
	s =
	    str_catenate
	    (
	       	rpt_value_string_query(v1s),
	       	rpt_value_string_query(v2s)
	    );
	result = rpt_value_string(s);
	str_free(s);
    }
    else
    {
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name1", v1s->method->name);
	sub_var_set_charstar(scp, "Name2", v2s->method->name);
	s = subst_intl(scp, i18n("illegal join ($name1 ## $name2)"));
	sub_context_delete(scp);
	result = rpt_value_error(0, s);
	str_free(s);
    }
    rpt_value_free(v1s);
    rpt_value_free(v2s);

    //
    // clean up and go home
    //
    rpt_value_free(v1);
    rpt_value_free(v2);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static tree_method_ty join_method =
{
    sizeof(tree_diadic_ty),
    ":",
    tree_diadic_destructor,
    tree_diadic_print,
    join_evaluate,
    tree_diadic_useful,
    tree_diadic_constant,
    tree_diadic_optimize,
};


tree_ty *
tree_join_new(tree_ty *left, tree_ty *right)
{
    return tree_diadic_new(&join_method, left, right);
}
