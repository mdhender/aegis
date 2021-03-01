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
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: interface definition for bit.c
 */

#ifndef AEGIS_AER_EXPR_BIT_H
#define AEGIS_AER_EXPR_BIT_H

#include <aer/expr.h>

rpt_expr_ty *rpt_expr_and_bit _((rpt_expr_ty *, rpt_expr_ty *));
rpt_expr_ty *rpt_expr_xor_bit _((rpt_expr_ty *, rpt_expr_ty *));
rpt_expr_ty *rpt_expr_or_bit _((rpt_expr_ty *, rpt_expr_ty *));
rpt_expr_ty *rpt_expr_not_bit _((rpt_expr_ty *));

#endif /* AEGIS_AER_EXPR_BIT_H */