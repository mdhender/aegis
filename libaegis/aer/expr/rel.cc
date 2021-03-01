//
//	aegis - project change supervisor
//	Copyright (C) 1994-1996, 1999, 2001-2005 Peter Miller;
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
// MANIFEST: functions to manipulate relative comparison expressions
//

#include <common/ac/string.h>

#include <libaegis/aer/expr/rel.h>
#include <libaegis/aer/value/boolean.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/real.h>
#include <libaegis/aer/value/string.h>
#include <common/error.h>
#include <common/fstrcmp.h>
#include <libaegis/sub.h>
#include <common/trace.h>

#define PAIR(a, b)	((a) * rpt_value_type_MAX + (b))


static rpt_value_ty *
lt_evaluate(rpt_expr_ty *this_thing)
{
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
    trace(("lt::evaluate()\n{\n"));
    assert(this_thing->nchild == 2);
    lv1 = rpt_expr_evaluate(this_thing->child[0], 1);
    if (lv1->method->type == rpt_value_type_error)
    {
	trace(("}\n"));
	return lv1;
    }

    //
    // coerce the left hand side to an arithmetic type
    //	(will not give error if can't, will copy instead)
    //
    lv2 = rpt_value_arithmetic(lv1);

    //
    // evaluate the right hand side
    //
    rv1 = rpt_expr_evaluate(this_thing->child[1], 1);
    if (rv1->method->type == rpt_value_type_error)
    {
	rpt_value_free(lv1);
	rpt_value_free(lv2);
	trace(("}\n"));
	return rv1;
    }

    //
    // coerce the right hand side to an arithmetic type
    //	(will not give error if can't, will copy instead)
    //
    rv2 = rpt_value_arithmetic(rv1);

    //
    // what to do depends on
    // the types of the operands
    //
    switch (PAIR(lv2->method->type, rv2->method->type))
    {
    case PAIR(rpt_value_type_real, rpt_value_type_real):
	vp =
	    rpt_value_boolean
	    (
	       	rpt_value_real_query(lv2) < rpt_value_real_query(rv2)
	    );
	trace(("(%g < %g) --> %d\n", rpt_value_real_query(lv2),
	    rpt_value_real_query(rv2), rpt_value_boolean_query(vp)));
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
	//
	// try to compare as strings
	//
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

	//
	// give up on this one
	//
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name1", lv1->method->name);
	sub_var_set_charstar(scp, "Name2", rv1->method->name);
	s = subst_intl(scp, i18n("illegal comparison ($name1 < $name2)"));
	sub_context_delete(scp);
	vp = rpt_value_error(this_thing->pos, s);
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


static rpt_expr_method_ty lt_method =
{
    sizeof(rpt_expr_ty),
    "less than",
    0, // construct
    0, // destruct
    lt_evaluate,
    0, // lvalue
};


rpt_expr_ty *
rpt_expr_lt(rpt_expr_ty *a, rpt_expr_ty *b)
{
    rpt_expr_ty     *this_thing;

    this_thing = rpt_expr_alloc(&lt_method);
    rpt_expr_append(this_thing, a);
    rpt_expr_append(this_thing, b);
    return this_thing;
}


static rpt_value_ty *
le_evaluate(rpt_expr_ty *this_thing)
{
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
    trace(("le::evaluate()\n{\n"));
    assert(this_thing->nchild == 2);
    lv1 = rpt_expr_evaluate(this_thing->child[0], 1);
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

    //
    // evaluate the right hand side
    //
    rv1 = rpt_expr_evaluate(this_thing->child[1], 1);
    if (rv1->method->type == rpt_value_type_error)
    {
	rpt_value_free(lv1);
	rpt_value_free(lv2);
	trace(("}\n"));
	return rv1;
    }

    //
    // coerce the right hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    rv2 = rpt_value_arithmetic(rv1);

    //
    // what to do depends on
    // the types of the operands
    //
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
	//
	// try to compare as strings
	//
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

	//
	// give up on this one
	//
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name1", lv1->method->name);
	sub_var_set_charstar(scp, "Name2", rv1->method->name);
	s = subst_intl(scp, i18n("illegal comparison ($name1 <= $name2)"));
	sub_context_delete(scp);
	vp = rpt_value_error(this_thing->pos, s);
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


static rpt_expr_method_ty le_method =
{
    sizeof(rpt_expr_ty),
    "less than or equal to",
    0, // construct
    0, // destruct
    le_evaluate,
    0, // lvalue
};


rpt_expr_ty *
rpt_expr_le(rpt_expr_ty *a, rpt_expr_ty *b)
{
    rpt_expr_ty     *this_thing;

    this_thing = rpt_expr_alloc(&le_method);
    rpt_expr_append(this_thing, a);
    rpt_expr_append(this_thing, b);
    return this_thing;
}


static rpt_value_ty *
gt_evaluate(rpt_expr_ty *this_thing)
{
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
    trace(("gt::evaluate()\n{\n"));
    assert(this_thing->nchild == 2);
    lv1 = rpt_expr_evaluate(this_thing->child[0], 1);
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

    //
    // evaluate the right hand side
    //
    rv1 = rpt_expr_evaluate(this_thing->child[1], 1);
    if (rv1->method->type == rpt_value_type_error)
    {
	rpt_value_free(lv1);
	rpt_value_free(lv2);
	trace(("}\n"));
	return rv1;
    }

    //
    // coerce the right hand side to an arithmetic type
    //	(will not give error if can't, will copy instead)
    //
    rv2 = rpt_value_arithmetic(rv1);

    //
    // what to do depends on
    // the types of the operands
    //
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
	//
	// try to compare as strings
	//
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

	//
	// give up on this one
	//
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name1", lv1->method->name);
	sub_var_set_charstar(scp, "Name2", rv1->method->name);
	s = subst_intl(scp, i18n("illegal comparison ($name1 > $name2)"));
	sub_context_delete(scp);
	vp = rpt_value_error(this_thing->pos, s);
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


static rpt_expr_method_ty gt_method =
{
    sizeof(rpt_expr_ty),
    "greater than",
    0, // construct
    0, // destruct
    gt_evaluate,
    0, // lvalue
};


rpt_expr_ty *
rpt_expr_gt(rpt_expr_ty *a, rpt_expr_ty *b)
{
    rpt_expr_ty     *this_thing;

    this_thing = rpt_expr_alloc(&gt_method);
    rpt_expr_append(this_thing, a);
    rpt_expr_append(this_thing, b);
    return this_thing;
}


static rpt_value_ty *
ge_evaluate(rpt_expr_ty *this_thing)
{
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
    trace(("ge::evaluate()\n{\n"));
    assert(this_thing->nchild == 2);
    lv1 = rpt_expr_evaluate(this_thing->child[0], 1);
    if (lv1->method->type == rpt_value_type_error)
    {
	trace(("}\n"));
	return lv1;
    }

    //
    // coerce the left hand side to an arithmetic type
    //	(will not give error if can't, will copy instead)
    //
    lv2 = rpt_value_arithmetic(lv1);

    //
    // evaluate the right hand side
    //
    rv1 = rpt_expr_evaluate(this_thing->child[1], 1);
    if (rv1->method->type == rpt_value_type_error)
    {
	rpt_value_free(lv1);
	rpt_value_free(lv2);
	trace(("}\n"));
	return rv1;
    }

    //
    // coerce the right hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    rv2 = rpt_value_arithmetic(rv1);

    //
    // what to do depends on
    // the types of the operands
    //
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
	//
	// try to compare as strings
	//
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

	//
	// give up on this one
	//
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name1", lv1->method->name);
	sub_var_set_charstar(scp, "Name2", rv1->method->name);
	s = subst_intl(scp, i18n("illegal comparison ($name1 >= $name2)"));
	sub_context_delete(scp);
	vp = rpt_value_error(this_thing->pos, s);
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


static rpt_expr_method_ty ge_method =
{
    sizeof(rpt_expr_ty),
    "greater than or equal to",
    0, // construct
    0, // destruct
    ge_evaluate,
    0, // lvalue
};


rpt_expr_ty *
rpt_expr_ge(rpt_expr_ty *a, rpt_expr_ty *b)
{
    rpt_expr_ty     *this_thing;

    this_thing = rpt_expr_alloc(&ge_method);
    rpt_expr_append(this_thing, a);
    rpt_expr_append(this_thing, b);
    return this_thing;
}


static rpt_value_ty *
eq_evaluate(rpt_expr_ty *this_thing)
{
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
    trace(("eq::evaluate()\n{\n"));
    assert(this_thing->nchild == 2);
    lv1 = rpt_expr_evaluate(this_thing->child[0], 1);
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

    //
    // evaluate the right hand side
    //
    rv1 = rpt_expr_evaluate(this_thing->child[1], 1);
    if (rv1->method->type == rpt_value_type_error)
    {
	rpt_value_free(lv1);
	rpt_value_free(lv2);
	trace(("}\n"));
	return rv1;
    }

    //
    // coerce the right hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    rv2 = rpt_value_arithmetic(rv1);

    //
    // what to do depends on
    // the types of the operands
    //
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
	//
	// try to compare as strings
	//
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

	//
	// give up on this_thing one
	//
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name1", lv1->method->name);
	sub_var_set_charstar(scp, "Name2", rv1->method->name);
	s = subst_intl(scp, i18n("illegal comparison ($name1 == $name2)"));
	sub_context_delete(scp);
	vp = rpt_value_error(this_thing->pos, s);
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


static rpt_expr_method_ty eq_method =
{
    sizeof(rpt_expr_ty),
    "equal to",
    0, // construct
    0, // destruct
    eq_evaluate,
    0, // lvalue
};


rpt_expr_ty *
rpt_expr_eq(rpt_expr_ty *a, rpt_expr_ty *b)
{
    rpt_expr_ty     *this_thing;

    this_thing = rpt_expr_alloc(&eq_method);
    rpt_expr_append(this_thing, a);
    rpt_expr_append(this_thing, b);
    return this_thing;
}


static rpt_value_ty *
ne_evaluate(rpt_expr_ty *this_thing)
{
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
    trace(("ne::evaluate()\n{\n"));
    assert(this_thing->nchild == 2);
    lv1 = rpt_expr_evaluate(this_thing->child[0], 1);
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

    //
    // evaluate the right hand side
    //
    rv1 = rpt_expr_evaluate(this_thing->child[1], 1);
    if (rv1->method->type == rpt_value_type_error)
    {
	rpt_value_free(lv1);
	rpt_value_free(lv2);
	trace(("}\n"));
	return rv1;
    }

    //
    // coerce the right hand side to an arithmetic type
    // (will not give error if can't, will copy instead)
    //
    rv2 = rpt_value_arithmetic(rv1);

    //
    // what to do depends on
    // the types of the operands
    //
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
	//
	// try to compare as strings
	//
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

	//
	// give up on this one
	//
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name1", lv1->method->name);
	sub_var_set_charstar(scp, "Name2", rv1->method->name);
	s = subst_intl(scp, i18n("illegal comparison ($name1 != $name2)"));
	sub_context_delete(scp);
	vp = rpt_value_error(this_thing->pos, s);
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


static rpt_expr_method_ty ne_method =
{
    sizeof(rpt_expr_ty),
    "not equal to",
    0, // construct
    0, // destruct
    ne_evaluate,
    0, // lvalue
};


rpt_expr_ty *
rpt_expr_ne(rpt_expr_ty *a, rpt_expr_ty *b)
{
    rpt_expr_ty     *this_thing;

    this_thing = rpt_expr_alloc(&ne_method);
    rpt_expr_append(this_thing, a);
    rpt_expr_append(this_thing, b);
    return this_thing;
}


static rpt_value_ty *
match_evaluate(rpt_expr_ty *this_thing)
{
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
    trace(("match::evaluate()\n{\n"));
    assert(this_thing->nchild == 2);
    lv1 = rpt_expr_evaluate(this_thing->child[0], 1);
    if (lv1->method->type == rpt_value_type_error)
    {
	trace(("}\n"));
	return lv1;
    }

    //
    // coerce the left hand side to a string
    // (will not give error if can't, will copy instead)
    //
    lv2 = rpt_value_stringize(lv1);

    //
    // evaluate the right hand side
    //
    rv1 = rpt_expr_evaluate(this_thing->child[1], 1);
    if (rv1->method->type == rpt_value_type_error)
    {
	rpt_value_free(lv1);
	rpt_value_free(lv2);
	trace(("}\n"));
	return rv1;
    }

    //
    // coerce the right hand side to a string
    // (will not give error if can't, will copy instead)
    //
    rv2 = rpt_value_stringize(rv1);

    //
    // what to do depends on
    // the types of the operands
    //
    switch (PAIR(lv2->method->type, rv2->method->type))
    {
    case PAIR(rpt_value_type_string, rpt_value_type_string):
	vp =
	    rpt_value_real
	    (
	       	fstrcmp
	       	(
		    rpt_value_string_query(lv2)->str_text,
		    rpt_value_string_query(rv2)->str_text
	       	)
	    );
	break;

    default:
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name1", lv1->method->name);
	sub_var_set_charstar(scp, "Name2", rv1->method->name);
	s = subst_intl(scp, i18n("illegal comparison ($name1 ~~ $name2)"));
	sub_context_delete(scp);
	vp = rpt_value_error(this_thing->pos, s);
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


static rpt_expr_method_ty match_method =
{
    sizeof(rpt_expr_ty),
    "match",
    0, // construct
    0, // destruct
    match_evaluate,
    0, // lvalue
};


rpt_expr_ty *
rpt_expr_match(rpt_expr_ty *a, rpt_expr_ty *b)
{
    rpt_expr_ty     *this_thing;

    this_thing = rpt_expr_alloc(&match_method);
    rpt_expr_append(this_thing, a);
    rpt_expr_append(this_thing, b);
    return this_thing;
}


static rpt_value_ty *
nmatch_evaluate(rpt_expr_ty *this_thing)
{
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
    trace(("nmatch::evaluate()\n{\n"));
    assert(this_thing->nchild == 2);
    lv1 = rpt_expr_evaluate(this_thing->child[0], 1);
    if (lv1->method->type == rpt_value_type_error)
    {
	trace(("}\n"));
	return lv1;
    }

    //
    // coerce the left hand side to a string
    // (will not give error if can't, will copy instead)
    //
    lv2 = rpt_value_arithmetic(lv1);

    //
    // evaluate the right hand side
    //
    rv1 = rpt_expr_evaluate(this_thing->child[1], 1);
    if (rv1->method->type == rpt_value_type_error)
    {
	rpt_value_free(lv1);
	rpt_value_free(lv2);
	trace(("}\n"));
	return rv1;
    }

    //
    // coerce the right hand side to a string
    // (will not give error if can't, will copy instead)
    //
    rv2 = rpt_value_arithmetic(rv1);

    //
    // what to do depends on
    // the types of the operands
    //
    switch (PAIR(lv2->method->type, rv2->method->type))
    {
    case PAIR(rpt_value_type_string, rpt_value_type_string):
	vp =
	    rpt_value_real
	    (
	       	1
	    -
	       	fstrcmp
	       	(
		    rpt_value_string_query(lv2)->str_text,
		    rpt_value_string_query(rv2)->str_text
	       	)
	    );
	break;

    default:
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name1", lv1->method->name);
	sub_var_set_charstar(scp, "Name2", rv1->method->name);
	s = subst_intl(scp, i18n("illegal comparison ($name1 !~ $name2)"));
	sub_context_delete(scp);
	vp = rpt_value_error(this_thing->pos, s);
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


static rpt_expr_method_ty nmatch_method =
{
    sizeof(rpt_expr_ty),
    "does not match",
    0, // construct
    0, // destruct
    nmatch_evaluate,
    0, // lvalue
};


rpt_expr_ty *
rpt_expr_nmatch(rpt_expr_ty *a, rpt_expr_ty *b)
{
    rpt_expr_ty     *this_thing;

    this_thing = rpt_expr_alloc(&nmatch_method);
    rpt_expr_append(this_thing, a);
    rpt_expr_append(this_thing, b);
    return this_thing;
}
