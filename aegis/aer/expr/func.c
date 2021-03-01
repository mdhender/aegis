/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994 Peter Miller.
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
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: functions to manipulate function call expressions
 */

#include <aer/expr/func.h>
#include <aer/func.h>
#include <aer/value/error.h>
#include <aer/value/func.h>
#include <error.h>
#include <mem.h>


static rpt_value_ty *evaluate _((rpt_expr_ty *));

static rpt_value_ty *
evaluate(ep)
	rpt_expr_ty	*ep;
{
	size_t		argc;
	rpt_value_ty	**argv;
	size_t		j;
	rpt_value_ty	*vp;
	rpt_func_ty	*fp;

	/*
	 * get the function pointer from the first argument
	 */
	assert(ep->nchild == 2);
	vp = rpt_expr_evaluate(ep->child[0]);
	if (vp->method->type != rpt_value_type_function)
	{
		rpt_value_ty	*vpe;

		vpe =
			rpt_value_error
			(
				"invalid function name (%s)",
				vp->method->name
			);
		rpt_value_free(vp);
		vp = vpe;
		rpt_expr_error(ep, "%S", rpt_value_error_query(vp));
		return vp;
	}
	fp = rpt_value_func_query(vp);
	rpt_value_free(vp);

	/*
	 * get the argument list from the second argument
	 */
	ep = ep->child[1];
	if (!fp->verify(ep))
	{
		vp = rpt_value_error("invalid function arguments");
		rpt_expr_error(ep, "%S", rpt_value_error_query(vp));
		return vp;
	}
	argc = ep->nchild;
	argv = mem_alloc(argc * sizeof(rpt_value_ty *));
	for (j = 0; j < argc; ++j)
		argv[j] = rpt_expr_evaluate(ep->child[j]);

	/*
	 * evaluate the function
	 */
	vp = fp->run(ep, argc, argv);

	/*
	 * free the argument list
	 */
	for (j = 0; j < argc; ++j)
		rpt_value_free(argv[j]);
	mem_free(argv);

	/*
	 * return the function return value
	 */
	return vp;
}


static rpt_expr_method_ty method =
{
	sizeof(rpt_expr_ty),
	"function call",
	0, /* construct */
	0, /* destruct */
	evaluate,
	0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_func(e1, e2)
	rpt_expr_ty	*e1;
	rpt_expr_ty	*e2;
{
	rpt_expr_ty	*this;

	this = rpt_expr_alloc(&method);
	rpt_expr_append(this, e1);
	rpt_expr_append(this, e2);
	return this;
}
