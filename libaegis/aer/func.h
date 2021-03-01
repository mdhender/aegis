//
//	aegis - project change supervisor
//	Copyright (C) 1994, 2002-2005 Peter Miller.
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
// MANIFEST: interface definition for aegis/aer/func.c
//

#ifndef AEGIS_AER_FUNC_H
#define AEGIS_AER_FUNC_H

#include <common/ac/stddef.h>
#include <common/main.h>

struct rpt_expr_ty;
struct rpt_value_ty;
struct symtab_ty;

struct rpt_func_ty
{
    const char      *name;
    int             optimizable;
    int             (*verify)(struct rpt_expr_ty *ep);
    struct rpt_value_ty *(*run)(struct rpt_expr_ty *, size_t argc,
			struct rpt_value_ty **argv);
};

void rpt_func_init(struct symtab_ty *);

#endif // AEGIS_AER_FUNC_H
