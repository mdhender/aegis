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
// MANIFEST: functions to manipulate function call expressions
//

#include <libaegis/aer/expr/func.h>
#include <libaegis/aer/func.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/func.h>
#include <common/error.h>
#include <common/mem.h>
#include <libaegis/sub.h>


static rpt_value_ty *
evaluate(rpt_expr_ty *ep)
{
    size_t	    argc;
    rpt_value_ty    **argv;
    size_t	    j;
    rpt_value_ty    *vp;
    rpt_func_ty     *fp;

    //
    // get the function pointer from the first argument
    //
    assert(ep->nchild == 2);
    vp = rpt_expr_evaluate(ep->child[0], 1);
    if (vp->method->type == rpt_value_type_error)
	return vp;
    if (vp->method->type != rpt_value_type_function)
    {
	sub_context_ty	*scp;
	string_ty	*s;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name", vp->method->name);
	rpt_value_free(vp);
	s = subst_intl(scp, i18n("invalid function name ($name)"));
	sub_context_delete(scp);
	vp = rpt_value_error(ep->child[0]->pos, s);
	str_free(s);
	return vp;
    }
    fp = rpt_value_func_query(vp);
    rpt_value_free(vp);

    //
    // get the argument list from the second argument
    //
    ep = ep->child[1];
    if (!fp->verify(ep))
    {
	sub_context_ty	*scp;
	string_ty	*s;

	scp = sub_context_new();
	s = subst_intl(scp, i18n("invalid function arguments"));
	sub_context_delete(scp);
	vp = rpt_value_error(ep->pos, s);
	str_free(s);
	return vp;
    }
    argc = ep->nchild;
    argv = (rpt_value_ty **)mem_alloc(argc * sizeof(rpt_value_ty *));
    for (j = 0; j < argc; ++j)
    {
	vp = rpt_expr_evaluate(ep->child[j], 0);
	argv[j] = vp;
	if (vp->method->type == rpt_value_type_error)
	{
	    size_t          k;

	    for (k = 0; k < j; ++k)
		rpt_value_free(argv[k]);
	    mem_free(argv);
	    return vp;
	}
    }

    //
    // evaluate the function
    //
    vp = fp->run(ep, argc, argv);

    //
    // free the argument list
    //
    for (j = 0; j < argc; ++j)
	rpt_value_free(argv[j]);
    mem_free(argv);

    //
    // return the function return value
    //
    return vp;
}


static rpt_expr_method_ty method =
{
    sizeof(rpt_expr_ty),
    "function call",
    0, // construct
    0, // destruct
    evaluate,
    0, // lvalue
};


rpt_expr_ty *
rpt_expr_func(rpt_expr_ty *e1, rpt_expr_ty *e2)
{
    rpt_expr_ty     *this_thing;

    this_thing = rpt_expr_alloc(&method);
    rpt_expr_append(this_thing, e1);
    rpt_expr_append(this_thing, e2);
    return this_thing;
}
