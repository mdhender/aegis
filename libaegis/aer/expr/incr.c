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
 * MANIFEST: functions to manipulate increment/decrement expressions
 */

#include <aer/expr/incr.h>
#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <aer/value/real.h>
#include <aer/value/ref.h>
#include <error.h>
#include <sub.h>


static rpt_value_ty *
inc_pre_evaluate(rpt_expr_ty *this)
{
    sub_context_ty  *scp;
    rpt_value_ty    *vp;
    rpt_value_ty    *v2;
    rpt_value_ty    *v2a;
    rpt_value_ty    *result;
    string_ty	    *s;

    assert(this->nchild==1);
    vp = rpt_expr_evaluate(this->child[0], 0);
    if (vp->method->type == rpt_value_type_error)
	return vp;
    if (vp->method->type != rpt_value_type_reference)
    {
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name", vp->method->name);
	rpt_value_free(vp);
	s =
	    subst_intl
	    (
		scp,
	      i18n("modifiable lvalue required for increment (was given $name)")
	    );
	sub_context_delete(scp);
	result = rpt_value_error(this->child[0]->pos, s);
	str_free(s);
	return result;
    }

    v2 = rpt_value_reference_get(vp);
    v2a = rpt_value_arithmetic(v2);
    rpt_value_free(v2);

    switch (v2a->method->type)
    {
    case rpt_value_type_integer:
	result = rpt_value_integer(1 + rpt_value_integer_query(v2a));
	break;

    case rpt_value_type_real:
	result = rpt_value_real(1 + rpt_value_real_query(v2a));
	break;

    case rpt_value_type_error:
	assert(0);
	/* fall through... */

    default:
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name", v2a->method->name);
	s =
	    subst_intl
	    (
		scp,
		i18n("arithmetic type required for increment (was given $name)")
	    );
	sub_context_delete(scp);
	result = rpt_value_error(this->child[0]->pos, s);
	str_free(s);
	return result;
    }
    rpt_value_free(v2a);
    rpt_value_reference_set(vp, result);
    rpt_value_free(vp);
    return result;
}


static rpt_expr_method_ty inc_pre_method =
{
    sizeof(rpt_expr_ty),
    "++e",
    0, /* construct */
    0, /* destruct */
    inc_pre_evaluate,
    0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_inc_pre(rpt_expr_ty *e)
{
    rpt_expr_ty	    *this;

    this = rpt_expr_alloc(&inc_pre_method);
    rpt_expr_append(this, e);
    if (!rpt_expr_lvalue(e))
    {
	rpt_expr_parse_error
	(
	    e,
	    i18n("modifiable lvalue required for increment")
	);
    }
    return this;
}


static rpt_value_ty *
dec_pre_evaluate(rpt_expr_ty *this)
{
    sub_context_ty  *scp;
    rpt_value_ty    *vp;
    rpt_value_ty    *v2;
    rpt_value_ty    *v2a;
    rpt_value_ty    *result;
    string_ty	    *s;

    assert(this->nchild==1);
    vp = rpt_expr_evaluate(this->child[0], 0);
    if (vp->method->type == rpt_value_type_error)
	return vp;
    if (vp->method->type != rpt_value_type_reference)
    {
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name", vp->method->name);
	rpt_value_free(vp);
	s =
	    subst_intl
	    (
		scp,
	      i18n("modifiable lvalue required for decrement (was given $name)")
	    );
	sub_context_delete(scp);
	result = rpt_value_error(this->child[0]->pos, s);
	str_free(s);
	return result;
    }

    v2 = rpt_value_reference_get(vp);
    v2a = rpt_value_arithmetic(v2);
    rpt_value_free(v2);

    switch (v2a->method->type)
    {
    case rpt_value_type_integer:
	result = rpt_value_integer(rpt_value_integer_query(v2a) - 1);
	break;

    case rpt_value_type_real:
	result = rpt_value_real(rpt_value_real_query(v2a) - 1);
	break;

    case rpt_value_type_error:
	assert(0);
	/* fall through... */

    default:
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name", v2a->method->name);
	s =
    	    subst_intl
	    (
		scp,
		i18n("arithmetic type required for decrement (was given $name)")
	    );
	sub_context_delete(scp);
	result = rpt_value_error(this->child[0]->pos, s);
	str_free(s);
	return result;
    }
    rpt_value_free(v2a);
    rpt_value_reference_set(vp, result);
    rpt_value_free(vp);
    return result;
}


static rpt_expr_method_ty dec_pre_method =
{
    sizeof(rpt_expr_ty),
    "--e",
    0, /* construct */
    0, /* destruct */
    dec_pre_evaluate,
    0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_dec_pre(rpt_expr_ty *e)
{
    rpt_expr_ty	    *this;

    this = rpt_expr_alloc(&dec_pre_method);
    rpt_expr_append(this, e);
    if (!rpt_expr_lvalue(e))
    {
	rpt_expr_parse_error
	(
	    e,
	    i18n("modifiable lvalue required for decrement")
	);
    }
    return this;
}


static rpt_value_ty *
inc_post_evaluate(rpt_expr_ty *this)
{
    sub_context_ty  *scp;
    rpt_value_ty    *vp;
    rpt_value_ty    *v2;
    rpt_value_ty    *v2a;
    rpt_value_ty    *result;
    string_ty	    *s;

    assert(this->nchild==1);
    vp = rpt_expr_evaluate(this->child[0], 0);
    if (vp->method->type == rpt_value_type_error)
	return vp;
    if (vp->method->type != rpt_value_type_reference)
    {
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name", vp->method->name);
	rpt_value_free(vp);
	s =
	    subst_intl
	    (
		scp,
	      i18n("modifiable lvalue required for increment (was given $name)")
	    );
	sub_context_delete(scp);
	result = rpt_value_error(this->child[0]->pos, s);
	str_free(s);
	return result;
    }

    v2 = rpt_value_reference_get(vp);
    v2a = rpt_value_arithmetic(v2);
    rpt_value_free(v2);

    switch (v2a->method->type)
    {
    case rpt_value_type_integer:
	result = rpt_value_integer(1 + rpt_value_integer_query(v2a));
	break;

    case rpt_value_type_real:
	result = rpt_value_real(1 + rpt_value_real_query(v2a));
	break;

    case rpt_value_type_error:
	assert(0);
	/* fall through... */

    default:
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name", v2a->method->name);
	rpt_value_free(v2a);
	s =
	    subst_intl
	    (
		scp,
		i18n("arithmetic type required for increment (was given $name)")
	    );
	sub_context_delete(scp);
	result = rpt_value_error(this->child[0]->pos, s);
	str_free(s);
	return result;
    }
    rpt_value_reference_set(vp, result);
    rpt_value_free(vp);
    rpt_value_free(result);
    return v2a;
}


static rpt_expr_method_ty inc_post_method =
{
    sizeof(rpt_expr_ty),
    "e++",
    0, /* construct */
    0, /* destruct */
    inc_post_evaluate,
    0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_inc_post(rpt_expr_ty *e)
{
    rpt_expr_ty	    *this;

    this = rpt_expr_alloc(&inc_post_method);
    rpt_expr_append(this, e);
    if (!rpt_expr_lvalue(e))
    {
	rpt_expr_parse_error
	(
	    e,
	    i18n("modifiable lvalue required for increment")
	);
    }
    return this;
}


static rpt_value_ty *
dec_post_evaluate(rpt_expr_ty *this)
{
    sub_context_ty  *scp;
    rpt_value_ty    *vp;
    rpt_value_ty    *v2;
    rpt_value_ty    *v2a;
    rpt_value_ty    *result;
    string_ty	    *s;

    assert(this->nchild==1);
    vp = rpt_expr_evaluate(this->child[0], 0);
    if (vp->method->type == rpt_value_type_error)
	return vp;
    if (vp->method->type != rpt_value_type_reference)
    {
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name", vp->method->name);
	rpt_value_free(vp);
	s =
	    subst_intl
	    (
		scp,
	      i18n("modifiable lvalue required for decrement (was given $name)")
	    );
	sub_context_delete(scp);
	result = rpt_value_error(this->child[0]->pos, s);
	str_free(s);
	return result;
    }

    v2 = rpt_value_reference_get(vp);
    v2a = rpt_value_arithmetic(v2);
    rpt_value_free(v2);

    switch (v2a->method->type)
    {
    case rpt_value_type_integer:
	result = rpt_value_integer(rpt_value_integer_query(v2a) - 1);
	break;

    case rpt_value_type_real:
	result = rpt_value_real(rpt_value_real_query(v2a) - 1);
	break;

    case rpt_value_type_error:
	assert(0);
	/* fall through... */

    default:
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name", v2a->method->name);
	rpt_value_free(v2a);
	s =
	    subst_intl
	    (
		scp,
		i18n("arithmetic type required for decrement (was given $name)")
	    );
	sub_context_delete(scp);
	result = rpt_value_error(this->child[0]->pos, s);
	str_free(s);
	return result;
    }
    rpt_value_reference_set(vp, result);
    rpt_value_free(vp);
    rpt_value_free(result);
    return v2a;
}


static rpt_expr_method_ty dec_post_method =
{
    sizeof(rpt_expr_ty),
    "e--",
    0, /* construct */
    0, /* destruct */
    dec_post_evaluate,
    0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_dec_post(rpt_expr_ty *e)
{
    rpt_expr_ty	    *this;

    this = rpt_expr_alloc(&dec_post_method);
    rpt_expr_append(this, e);
    if (!rpt_expr_lvalue(e))
    {
	rpt_expr_parse_error
	(
	    e,
	    i18n("modifiable lvalue required for decrement")
	);
    }
    return this;
}
