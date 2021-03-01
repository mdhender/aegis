/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994-1996, 1999, 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate plus expressions
 */

#include <aer/expr/plus.h>
#include <aer/value/error.h>
#include <aer/value/list.h>
#include <aer/value/integer.h>
#include <aer/value/real.h>
#include <aer/value/string.h>
#include <error.h>
#include <sub.h>
#include <trace.h>

#define PAIR(a, b)	((a) * rpt_value_type_MAX + (b))


static rpt_value_ty *
plus_evaluate(rpt_expr_ty *this)
{
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
    trace(("plus::evaluate()\n{\n"));
    assert(this->nchild == 2);
    lv1 = rpt_expr_evaluate(this->child[0], 1);
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
    rpt_value_free(lv1);

    /*
     * evaluate the right hand side
     */
    rv1 = rpt_expr_evaluate(this->child[1], 1);
    if (rv1->method->type == rpt_value_type_error)
    {
	rpt_value_free(lv2);
	trace(("}\n"));
	return rv1;
    }

    /*
     * coerce the right hand side to an arithmetic type
     * (will not give error if can't, will copy instead)
     */
    rv2 = rpt_value_arithmetic(rv1);
    rpt_value_free(rv1);

    /*
     * the type of the result depends on
     * the types of the operands
     */
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
	vp = rpt_value_error(this->pos, s);
	str_free(s);
	break;
    }
    rpt_value_free(lv2);
    rpt_value_free(rv2);
    trace(("return %08lX;\n", (long)vp));
    trace(("}\n"));
    return vp;
}


static rpt_expr_method_ty plus_method =
{
    sizeof(rpt_expr_ty),
    "plus",
    0, /* construct */
    0, /* destruct */
    plus_evaluate,
    0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_plus(rpt_expr_ty *a, rpt_expr_ty *b)
{
    rpt_expr_ty     *this;

    this = rpt_expr_alloc(&plus_method);
    rpt_expr_append(this, a);
    rpt_expr_append(this, b);
    return this;
}


static rpt_value_ty *
minus_evaluate(rpt_expr_ty *this)
{
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
    trace(("minus::evaluate()\n{\n"));
    assert(this->nchild == 2);
    lv1 = rpt_expr_evaluate(this->child[0], 1);
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
    rpt_value_free(lv1);

    /*
     * evaluate the right hand side
     */
    rv1 = rpt_expr_evaluate(this->child[1], 1);
    if (rv1->method->type == rpt_value_type_error)
    {
	rpt_value_free(lv2);
	trace(("}\n"));
	return rv1;
    }

    /*
     * coerce the right hand side to an arithmetic type
     * (will not give error if can't, will copy instead)
     */
    rv2 = rpt_value_arithmetic(rv1);
    rpt_value_free(rv1);

    /*
     * the type of the result depends on
     * the types of the operands
     */
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
	vp = rpt_value_error(this->pos, s);
	str_free(s);
	break;
    }
    rpt_value_free(lv2);
    rpt_value_free(rv2);
    trace(("return %08lX;\n", (long)vp));
    trace(("}\n"));
    return vp;
}


static rpt_expr_method_ty minus_method =
{
    sizeof(rpt_expr_ty),
    "minus",
    0, /* construct */
    0, /* destruct */
    minus_evaluate,
    0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_minus(rpt_expr_ty *a, rpt_expr_ty *b)
{
    rpt_expr_ty     *this;

    this = rpt_expr_alloc(&minus_method);
    rpt_expr_append(this, a);
    rpt_expr_append(this, b);
    return this;
}


static rpt_value_ty *
join_evaluate(rpt_expr_ty *this)
{
    sub_context_ty  *scp;
    rpt_value_ty    *v1;
    rpt_value_ty    *v2;
    rpt_value_ty    *result;
    string_ty       *s;

    /*
     * evaluate the left hand side
     */
    trace(("join::evaluate()\n{\n"));
    assert(this->nchild == 2);
    v1 = rpt_expr_evaluate(this->child[0], 1);
    if (v1->method->type == rpt_value_type_error)
    {
	trace(("}\n"));
	return v1;
    }
    assert(v1->method->type != rpt_value_type_reference);

    /*
     * evaluate the right hand side
     */
    v2 = rpt_expr_evaluate(this->child[1], 1);
    if (v2->method->type == rpt_value_type_error)
    {
	rpt_value_free(v1);
	trace(("}\n"));
	return v2;
    }
    assert(v2->method->type != rpt_value_type_reference);

    /*
     * you can join almost anything to a list
     */
    if (v1->method->type == rpt_value_type_list)
    {
	long		n;
	long		j;

	result = rpt_value_list();
	n = rpt_value_list_length(v1);
	for (j = 0; j < n; ++j)
	{
	    rpt_value_list_append(result, rpt_value_list_nth(v1, j));
	}

	if (v2->method->type == rpt_value_type_list)
	{
	    n = rpt_value_list_length(v2);
	    for (j = 0; j < n; ++j)
	    {
		rpt_value_list_append(result, rpt_value_list_nth(v2, j));
	    }
	}
	else
	    rpt_value_list_append(result, v2);
    }
    else if (v2->method->type == rpt_value_type_list)
    {
	long		n;
	long		j;

	result = rpt_value_list();
	rpt_value_list_append(result, v1);
	n = rpt_value_list_length(v2);
	for (j = 0; j < n; ++j)
	{
	    rpt_value_list_append(result, rpt_value_list_nth(v2, j));
	}
    }
    else
    {
	rpt_value_ty	*v1s;
	rpt_value_ty	*v2s;

	/*
	 * must be a string join
	 */
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
	    result = rpt_value_error(this->pos, s);
	    str_free(s);
	}
	rpt_value_free(v1s);
	rpt_value_free(v2s);
    }

    /*
     * clean up and go home
     */
    rpt_value_free(v1);
    rpt_value_free(v2);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static rpt_expr_method_ty join_method =
{
    sizeof(rpt_expr_ty),
    "join",
    0, /* construct */
    0, /* destruct */
    join_evaluate,
    0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_join(rpt_expr_ty *a, rpt_expr_ty *b)
{
    rpt_expr_ty     *this;

    this = rpt_expr_alloc(&join_method);
    rpt_expr_append(this, a);
    rpt_expr_append(this, b);
    return this;
}
