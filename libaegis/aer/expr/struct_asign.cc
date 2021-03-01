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
// MANIFEST: functions to manipulate struct member assignment expressions
//

#include <aer/expr/struct.h>
#include <aer/expr/struct_asign.h>
#include <aer/value/ref.h>
#include <aer/value/void.h>
#include <symtab.h>


typedef struct rpt_expr_struct_assign_ty rpt_expr_struct_assign_ty;
struct rpt_expr_struct_assign_ty
{
    RPT_EXPR
    string_ty       *name;
    rpt_expr_ty     *value;
};


static void
destruct(rpt_expr_ty *ep)
{
    rpt_expr_struct_assign_ty *this_thing;

    this_thing = (rpt_expr_struct_assign_ty *)ep;
    str_free(this_thing->name);
    rpt_expr_free(this_thing->value);
}


static rpt_value_ty *
evaluate(rpt_expr_ty *ep)
{
    rpt_expr_struct_assign_ty *this_thing;
    rpt_value_ty    *vp;
    rpt_value_ty    *rvp;
    symtab_ty       *stp;

    this_thing = (rpt_expr_struct_assign_ty *)ep;
    vp = rpt_expr_evaluate(this_thing->value, 0);
    if (vp->method->type == rpt_value_type_error)
	return vp;
    rvp = rpt_value_reference(vp);
    rpt_value_free(vp);
    stp = rpt_expr_struct__symtab_query();
    symtab_assign(stp, this_thing->name, rvp);
    return rpt_value_void();
}


static rpt_expr_method_ty method =
{
    sizeof(rpt_expr_struct_assign_ty),
    "struct assign",
    0, // construct
    destruct,
    evaluate,
    0, // lvalue
};


rpt_expr_ty *
rpt_expr_struct_assign(string_ty *name, rpt_expr_ty *ep)
{
    rpt_expr_struct_assign_ty *this_thing;

    this_thing = (rpt_expr_struct_assign_ty *)rpt_expr_alloc(&method);
    this_thing->name = str_copy(name);
    this_thing->value = rpt_expr_copy(ep);
    return (rpt_expr_ty *)this_thing;
}