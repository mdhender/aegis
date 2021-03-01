//
//	aegis - project change supervisor
//	Copyright (C) 1994, 1996, 2002-2004 Peter Miller;
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
// MANIFEST: functions to manipulate struct expressions
//

#include <aer/expr/struct.h>
#include <aer/value/struct.h>
#include <error.h>
#include <mem.h>
#include <symtab.h>

static size_t   ntab;
static size_t   ntab_max;
static symtab_ty **tab;


static void
rpt_expr_struct__symtab_push(symtab_ty *stp)
{
    if (ntab >= ntab_max)
    {
	size_t	nbytes;

	ntab_max = ntab_max * 2 + 4;
	nbytes = ntab_max * sizeof(symtab_ty *);
	tab = (symtab_ty **)mem_change_size(tab, nbytes);
    }
    tab[ntab++] = stp;
}


static void
rpt_expr_struct__symtab_pop(void)
{
    assert(ntab);
    --ntab;
}


symtab_ty *
rpt_expr_struct__symtab_query(void)
{
    assert(ntab);
    return tab[ntab - 1];
}


static void
reap(void *p)
{
    rpt_value_ty    *vp;

    vp = (rpt_value_ty *)p;
    rpt_value_free(vp);
}


static rpt_value_ty *
evaluate(rpt_expr_ty *ep)
{
    size_t	    j;
    rpt_value_ty    *result;
    rpt_value_ty    *vp;

    symtab_ty *stp = new symtab_ty(ep->nchild);
    stp->set_reap(reap);
    rpt_expr_struct__symtab_push(stp);
    result = 0;
    for (j = 0; j < ep->nchild; ++j)
    {
	vp = rpt_expr_evaluate(ep->child[j], 0);
	if (vp->method->type == rpt_value_type_error)
	{
	    result = vp;
	    delete stp;
	    break;
	}
	rpt_value_free(vp);
    }
    rpt_expr_struct__symtab_pop();
    if (!result)
	result = rpt_value_struct(stp);
    return result;
}


static rpt_expr_method_ty method =
{
    sizeof(rpt_expr_ty),
    "struct",
    0, // construct
    0, // destruct
    evaluate,
    0, // lvalue
};


rpt_expr_ty *
rpt_expr_struct(void)
{
    return rpt_expr_alloc(&method);
}
