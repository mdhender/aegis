//
//	aegis - project change supervisor
//	Copyright (C) 1994-1996, 1999, 2002-2005 Peter Miller;
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
// MANIFEST: functions to manipulate assign expressions
//

#include <libaegis/aer/expr/assign.h>
#include <libaegis/aer/expr/bit.h>
#include <libaegis/aer/expr/constant.h>
#include <libaegis/aer/expr/mul.h>
#include <libaegis/aer/expr/plus.h>
#include <libaegis/aer/expr/power.h>
#include <libaegis/aer/expr/shift.h>
#include <libaegis/aer/lex.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/real.h>
#include <libaegis/aer/value/ref.h>
#include <common/error.h>
#include <libaegis/sub.h>
#include <common/trace.h>


static rpt_value_ty *
simple_evaluate(rpt_expr_ty *this_thing)
{
    rpt_value_ty	*lv;
    rpt_value_ty	*rv;

    //
    // evaluate the left hand side
    //
    trace(("simple_assign::evaluate()\n{\n"));
    assert(this_thing->nchild == 2);
    lv = rpt_expr_evaluate(this_thing->child[0], 0);
    assert(lv->reference_count >= 1);
    if (lv->method->type == rpt_value_type_error)
    {
	trace(("}\n"));
	return lv;
    }

    //
    // if the left hand side is not a reference type,
    // it is an error
    //
    if (lv->method->type != rpt_value_type_reference)
    {
	sub_context_ty	*scp;
	string_ty	*s;
	rpt_value_ty	*vp;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name", lv->method->name);
	s =
	    subst_intl
	    (
	       	scp,
		i18n("illegal left hand side of assignment (was given $name)")
	    );
	sub_context_delete(scp);
	assert(this_thing->pos);
	vp = rpt_value_error(this_thing->child[0]->pos, s);
	str_free(s);
	rpt_value_free(lv);
	trace(("}\n"));
	return vp;
    }

    //
    // evaluate the right hand side
    //
    rv = rpt_expr_evaluate(this_thing->child[1], 0);
    assert(rv->reference_count >= 1);
    if (rv->method->type == rpt_value_type_error)
    {
	rpt_value_free(lv);
	trace(("}\n"));
	return rv;
    }

    //
    // set the value (will take copy)
    //
    trace(("set reference\n"));
    trace(("(lv=%08lX)->refence_count = %ld;\n",
	(long)lv, lv->reference_count));
    trace(("(rv=%08lX)->refence_count = %ld;\n",
	(long)rv, rv->reference_count));
    rpt_value_reference_set(lv, rv);
    trace(("(lv=%08lX)->refence_count = %ld;\n",
	(long)lv, lv->reference_count));
    trace(("(rv=%08lX)->refence_count = %ld;\n",
	(long)rv, rv->reference_count));

    //
    // cleanup and go home
    //
    trace(("clean up and go home\n"));
    rpt_value_free(lv);
    assert(rv->reference_count >= 2);
    trace(("return %08lX;\n", (long)rv));
    trace(("}\n"));
    return rv;
}


static rpt_expr_method_ty simple_method =
{
    sizeof(rpt_expr_ty),
    "assign",
    0, // construct
    0, // destruct
    simple_evaluate,
    0, // lvalue
};


rpt_expr_ty *
rpt_expr_assign(rpt_expr_ty *a, rpt_expr_ty *b)
{
    rpt_expr_ty	*this_thing;

    this_thing = rpt_expr_alloc(&simple_method);
    if (!rpt_expr_lvalue(a))
	rpt_expr_parse_error(a, i18n("illegal left hand side of assignment"));
    rpt_expr_append(this_thing, a);
    rpt_expr_append(this_thing, b);
    return this_thing;
}


static rpt_value_ty *
bin_eval(rpt_expr_ty *(*op)(rpt_expr_ty *, rpt_expr_ty *), rpt_expr_ty *e)
{
    rpt_value_ty    *ptr;
    rpt_expr_ty     *e1;
    rpt_expr_ty     *e2;
    rpt_expr_ty     *e3;
    rpt_expr_ty     *e4;
    rpt_value_ty    *result;

    //
    // evaluate the left hand side
    //
    trace(("bin_eval()\n{\n"));
    assert(e->nchild == 2);
    ptr = rpt_expr_evaluate(e->child[0], 0);
    assert(ptr->reference_count >= 1);
    if (ptr->method->type == rpt_value_type_error)
    {
	trace(("}\n"));
	return ptr;
    }

    //
    // if the left hand side is not a reference type,
    // it is an error
    //
    if (ptr->method->type != rpt_value_type_reference)
    {
	sub_context_ty	*scp;
	string_ty	*s;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name", ptr->method->name);
	s =
	    subst_intl
	    (
	       	scp,
		i18n("illegal left hand side of assignment (was given $name)")
	    );
	sub_context_delete(scp);
	assert(e->pos);
	result = rpt_value_error(e->pos, s);
	str_free(s);
	goto done;
    }

    //
    // construct
    //	lhs = lhs "op" rhs
    //
    // Note:
    //	e1 and e2 are separate because one is an lvalue, and
    //	the other is an rvalue.  They e1 is altered by
    //	rpt_expr_assign, while e2 is not.
    //
    e1 = rpt_expr_constant(ptr);
    assert(!e1->pos);
    e1->pos = rpt_pos_copy(e->child[0]->pos);
    e2 = rpt_expr_constant(ptr);
    assert(!e2->pos);
    e2->pos = rpt_pos_copy(e->child[0]->pos);

    //
    // use the constant expression to build a simple assignment expression
    // "op" is the appropriate binary expression builder
    //
    e3 = op(e2, e->child[1]);
    rpt_expr_free(e2);
    e4 = rpt_expr_assign(e1, e3);
    rpt_expr_free(e1);
    rpt_expr_free(e3);

    //
    // evaluate the simple assignment
    //	and discard the fake expression tree
    //
    result = rpt_expr_evaluate(e4, 0);
    rpt_expr_free(e4);

    //
    // clean up and go home
    //
    done:
    rpt_value_free(ptr);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static rpt_value_ty *
power_evaluate(rpt_expr_ty *this_thing)
{
    rpt_value_ty    *result;

    trace(("power_assign::evaluate()\n{\n"));
    result = bin_eval(rpt_expr_power, this_thing);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static rpt_expr_method_ty power_method =
{
    sizeof(rpt_expr_ty),
    "power and assign",
    0, // construct
    0, // destruct
    power_evaluate,
    0, // lvalue
};


rpt_expr_ty *
rpt_expr_assign_power(rpt_expr_ty *a, rpt_expr_ty *b)
{
    rpt_expr_ty     *this_thing;

    this_thing = rpt_expr_alloc(&power_method);
    if (!rpt_expr_lvalue(a))
	rpt_expr_parse_error(a, i18n("illegal left hand side of assignment"));
    rpt_expr_append(this_thing, a);
    rpt_expr_append(this_thing, b);
    return this_thing;
}


static rpt_value_ty *
mul_evaluate(rpt_expr_ty *this_thing)
{
    rpt_value_ty    *result;

    trace(("mul_assign::evaluate()\n{\n"));
    result = bin_eval(rpt_expr_mul, this_thing);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static rpt_expr_method_ty mul_method =
{
    sizeof(rpt_expr_ty),
    "mul and assign",
    0, // construct
    0, // destruct
    mul_evaluate,
    0, // lvalue
};


rpt_expr_ty *
rpt_expr_assign_mul(rpt_expr_ty *a, rpt_expr_ty *b)
{
    rpt_expr_ty     *this_thing;

    this_thing = rpt_expr_alloc(&mul_method);
    if (!rpt_expr_lvalue(a))
	rpt_expr_parse_error(a, i18n("illegal left hand side of assignment"));
    rpt_expr_append(this_thing, a);
    rpt_expr_append(this_thing, b);
    return this_thing;
}


static rpt_value_ty *
div_evaluate(rpt_expr_ty *this_thing)
{
    rpt_value_ty    *result;

    trace(("div_assign::evaluate()\n{\n"));
    result = bin_eval(rpt_expr_div, this_thing);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static rpt_expr_method_ty div_method =
{
    sizeof(rpt_expr_ty),
    "div and assign",
    0, // construct
    0, // destruct
    div_evaluate,
    0, // lvalue
};


rpt_expr_ty *
rpt_expr_assign_div(rpt_expr_ty *a, rpt_expr_ty *b)
{
    rpt_expr_ty     *this_thing;

    this_thing = rpt_expr_alloc(&div_method);
    if (!rpt_expr_lvalue(a))
	rpt_expr_parse_error(a, i18n("illegal left hand side of assignment"));
    rpt_expr_append(this_thing, a);
    rpt_expr_append(this_thing, b);
    return this_thing;
}


static rpt_value_ty *
mod_evaluate(rpt_expr_ty *this_thing)
{
    rpt_value_ty    *result;

    trace(("mod_assign::evaluate()\n{\n"));
    result = bin_eval(rpt_expr_mod, this_thing);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static rpt_expr_method_ty mod_method =
{
    sizeof(rpt_expr_ty),
    "mod and assign",
    0, // construct
    0, // destruct
    mod_evaluate,
    0, // lvalue
};


rpt_expr_ty *
rpt_expr_assign_mod(rpt_expr_ty *a, rpt_expr_ty *b)
{
    rpt_expr_ty     *this_thing;

    this_thing = rpt_expr_alloc(&mod_method);
    if (!rpt_expr_lvalue(a))
	rpt_expr_parse_error(a, i18n("illegal left hand side of assignment"));
    rpt_expr_append(this_thing, a);
    rpt_expr_append(this_thing, b);
    return this_thing;
}


static rpt_value_ty *
plus_evaluate(rpt_expr_ty *this_thing)
{
    rpt_value_ty    *result;

    trace(("plus_assign::evaluate()\n{\n"));
    result = bin_eval(rpt_expr_plus, this_thing);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static rpt_expr_method_ty plus_method =
{
    sizeof(rpt_expr_ty),
    "plus and assign",
    0, // construct
    0, // destruct
    plus_evaluate,
    0, // lvalue
};


rpt_expr_ty *
rpt_expr_assign_plus(rpt_expr_ty *a, rpt_expr_ty *b)
{
    rpt_expr_ty     *this_thing;

    this_thing = rpt_expr_alloc(&plus_method);
    if (!rpt_expr_lvalue(a))
	rpt_expr_parse_error(a, i18n("illegal left hand side of assignment"));
    rpt_expr_append(this_thing, a);
    rpt_expr_append(this_thing, b);
    return this_thing;
}


static rpt_value_ty *
minus_evaluate(rpt_expr_ty *this_thing)
{
    rpt_value_ty    *result;

    trace(("minus_assign::evaluate()\n{\n"));
    result = bin_eval(rpt_expr_minus, this_thing);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static rpt_expr_method_ty minus_method =
{
    sizeof(rpt_expr_ty),
    "minus and assign",
    0, // construct
    0, // destruct
    minus_evaluate,
    0, // lvalue
};


rpt_expr_ty *
rpt_expr_assign_minus(rpt_expr_ty *a, rpt_expr_ty *b)
{
    rpt_expr_ty     *this_thing;

    this_thing = rpt_expr_alloc(&minus_method);
    if (!rpt_expr_lvalue(a))
	rpt_expr_parse_error(a, i18n("illegal left hand side of assignment"));
    rpt_expr_append(this_thing, a);
    rpt_expr_append(this_thing, b);
    return this_thing;
}


static rpt_value_ty *
and_bit_evaluate(rpt_expr_ty *this_thing)
{
    rpt_value_ty    *result;

    trace(("and_bit_assign::evaluate()\n{\n"));
    result = bin_eval(rpt_expr_and_bit, this_thing);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static rpt_expr_method_ty and_bit_method =
{
    sizeof(rpt_expr_ty),
    "and_bit and assign",
    0, // construct
    0, // destruct
    and_bit_evaluate,
    0, // lvalue
};


rpt_expr_ty *
rpt_expr_assign_and_bit(rpt_expr_ty *a, rpt_expr_ty *b)
{
    rpt_expr_ty     *this_thing;

    this_thing = rpt_expr_alloc(&and_bit_method);
    if (!rpt_expr_lvalue(a))
	rpt_expr_parse_error(a, i18n("illegal left hand side of assignment"));
    rpt_expr_append(this_thing, a);
    rpt_expr_append(this_thing, b);
    return this_thing;
}


static rpt_value_ty *
xor_bit_evaluate(rpt_expr_ty *this_thing)
{
    rpt_value_ty    *result;

    trace(("xor_bit_assign::evaluate()\n{\n"));
    result = bin_eval(rpt_expr_xor_bit, this_thing);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static rpt_expr_method_ty xor_bit_method =
{
    sizeof(rpt_expr_ty),
    "xor_bit and assign",
    0, // construct
    0, // destruct
    xor_bit_evaluate,
    0, // lvalue
};


rpt_expr_ty *
rpt_expr_assign_xor_bit(rpt_expr_ty *a, rpt_expr_ty *b)
{
    rpt_expr_ty     *this_thing;

    this_thing = rpt_expr_alloc(&xor_bit_method);
    if (!rpt_expr_lvalue(a))
	rpt_expr_parse_error(a, i18n("illegal left hand side of assignment"));
    rpt_expr_append(this_thing, a);
    rpt_expr_append(this_thing, b);
    return this_thing;
}


static rpt_value_ty *
or_bit_evaluate(rpt_expr_ty *this_thing)
{
    rpt_value_ty    *result;

    trace(("or_bit_assign::evaluate()\n{\n"));
    result = bin_eval(rpt_expr_or_bit, this_thing);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static rpt_expr_method_ty or_bit_method =
{
    sizeof(rpt_expr_ty),
    "or_bit and assign",
    0, // construct
    0, // destruct
    or_bit_evaluate,
    0, // lvalue
};


rpt_expr_ty *
rpt_expr_assign_or_bit(rpt_expr_ty *a, rpt_expr_ty *b)
{
    rpt_expr_ty     *this_thing;

    this_thing = rpt_expr_alloc(&or_bit_method);
    if (!rpt_expr_lvalue(a))
	rpt_expr_parse_error(a, i18n("illegal left hand side of assignment"));
    rpt_expr_append(this_thing, a);
    rpt_expr_append(this_thing, b);
    return this_thing;
}


static rpt_value_ty *
shift_left_evaluate(rpt_expr_ty *this_thing)
{
    rpt_value_ty    *result;

    trace(("shift_left_assign::evaluate()\n{\n"));
    result = bin_eval(rpt_expr_shift_left, this_thing);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static rpt_expr_method_ty shift_left_method =
{
    sizeof(rpt_expr_ty),
    "shift_left and assign",
    0, // construct
    0, // destruct
    shift_left_evaluate,
    0, // lvalue
};


rpt_expr_ty *
rpt_expr_assign_shift_left(rpt_expr_ty *a, rpt_expr_ty *b)
{
    rpt_expr_ty     *this_thing;

    this_thing = rpt_expr_alloc(&shift_left_method);
    if (!rpt_expr_lvalue(a))
	rpt_expr_parse_error(a, i18n("illegal left hand side of assignment"));
    rpt_expr_append(this_thing, a);
    rpt_expr_append(this_thing, b);
    return this_thing;
}


static rpt_value_ty *
shift_right_evaluate(rpt_expr_ty *this_thing)
{
    rpt_value_ty    *result;

    trace(("shift_right_assign::evaluate()\n{\n"));
    result = bin_eval(rpt_expr_shift_right, this_thing);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static rpt_expr_method_ty shift_right_method =
{
    sizeof(rpt_expr_ty),
    "shift_right and assign",
    0, // construct
    0, // destruct
    shift_right_evaluate,
    0, // lvalue
};


rpt_expr_ty *
rpt_expr_assign_shift_right(rpt_expr_ty *a, rpt_expr_ty *b)
{
    rpt_expr_ty     *this_thing;

    this_thing = rpt_expr_alloc(&shift_right_method);
    if (!rpt_expr_lvalue(a))
	rpt_expr_parse_error(a, i18n("illegal left hand side of assignment"));
    rpt_expr_append(this_thing, a);
    rpt_expr_append(this_thing, b);
    return this_thing;
}


static rpt_value_ty *
join_evaluate(rpt_expr_ty *this_thing)
{
    rpt_value_ty    *result;

    trace(("join_assign::evaluate()\n{\n"));
    result = bin_eval(rpt_expr_join, this_thing);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static rpt_expr_method_ty join_method =
{
    sizeof(rpt_expr_ty),
    "join and assign",
    0, // construct
    0, // destruct
    join_evaluate,
    0, // lvalue
};


rpt_expr_ty *
rpt_expr_assign_join(rpt_expr_ty *a, rpt_expr_ty *b)
{
    rpt_expr_ty     *this_thing;

    this_thing = rpt_expr_alloc(&join_method);
    if (!rpt_expr_lvalue(a))
	rpt_expr_parse_error(a, i18n("illegal left hand side of assignment"));
    rpt_expr_append(this_thing, a);
    rpt_expr_append(this_thing, b);
    return this_thing;
}
