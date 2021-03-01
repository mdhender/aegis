/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1996, 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate list expressions
 */

#include <aer/expr/list.h>
#include <aer/value/list.h>
#include <error.h>


static rpt_value_ty *
list_evaluate(rpt_expr_ty *ep)
{
    rpt_value_ty    *vlp;
    rpt_value_ty    *vp;
    size_t	    j;

    vlp = rpt_value_list();
    for (j = 0; j < ep->nchild; ++j)
    {
	vp = rpt_expr_evaluate(ep->child[j], 0);
	if (vp->method->type == rpt_value_type_error)
	{
	    rpt_value_free(vlp);
	    return vp;
	}
	rpt_value_list_append(vlp, vp);
	rpt_value_free(vp);
    }
    return vlp;
}


static rpt_expr_method_ty list_method =
{
    sizeof(rpt_expr_ty),
    "list",
    0, /* construct */
    0, /* destruct */
    list_evaluate,
    0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_list()
{
    return rpt_expr_alloc(&list_method);
}


static rpt_value_ty *
comma_evaluate(rpt_expr_ty *ep)
{
    rpt_value_ty    *vp;

    assert(ep->nchild == 2);
    vp = rpt_expr_evaluate(ep->child[0], 0);
    if (vp->method->type == rpt_value_type_error)
	return vp;
    rpt_value_free(vp);
    return rpt_expr_evaluate(ep->child[1], 0);
}


static rpt_expr_method_ty comma_method =
{
    sizeof(rpt_expr_ty),
    "comma",
    0, /* construct */
    0, /* destruct */
    comma_evaluate,
    0, /* lvalue */
};


rpt_expr_ty *
rpt_expr_comma(rpt_expr_ty *e1, rpt_expr_ty *e2)
{
    rpt_expr_ty     *this;

    this = rpt_expr_alloc(&comma_method);
    rpt_expr_append(this, e1);
    rpt_expr_append(this, e2);
    return this;
}
