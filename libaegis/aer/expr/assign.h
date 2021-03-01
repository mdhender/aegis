//
//	aegis - project change supervisor
//	Copyright (C) 1994, 2002, 2005, 2006 Peter Miller.
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
// MANIFEST: interface definition for aegis/aer/expr/assign.c
//

#ifndef AEGIS_AER_EXPR_ASSIGN_H
#define AEGIS_AER_EXPR_ASSIGN_H

#include <libaegis/aer/expr.h>

rpt_expr_ty *rpt_expr_assign(rpt_expr_ty *, rpt_expr_ty *);
rpt_expr_ty *rpt_expr_assign_power(rpt_expr_ty *, rpt_expr_ty *);
rpt_expr_ty *rpt_expr_assign_mul(rpt_expr_ty *, rpt_expr_ty *);
rpt_expr_ty *rpt_expr_assign_div(rpt_expr_ty *, rpt_expr_ty *);
rpt_expr_ty *rpt_expr_assign_mod(rpt_expr_ty *, rpt_expr_ty *);
rpt_expr_ty *rpt_expr_assign_plus(rpt_expr_ty *, rpt_expr_ty *);
rpt_expr_ty *rpt_expr_assign_minus(rpt_expr_ty *, rpt_expr_ty *);
rpt_expr_ty *rpt_expr_assign_join(rpt_expr_ty *, rpt_expr_ty *);
rpt_expr_ty *rpt_expr_assign_and_bit(rpt_expr_ty *, rpt_expr_ty *);
rpt_expr_ty *rpt_expr_assign_xor_bit(rpt_expr_ty *, rpt_expr_ty *);
rpt_expr_ty *rpt_expr_assign_or_bit(rpt_expr_ty *, rpt_expr_ty *);
rpt_expr_ty *rpt_expr_assign_shift_left(rpt_expr_ty *, rpt_expr_ty *);
rpt_expr_ty *rpt_expr_assign_shift_right(rpt_expr_ty *, rpt_expr_ty *);

#endif // AEGIS_AER_EXPR_ASSIGN_H
