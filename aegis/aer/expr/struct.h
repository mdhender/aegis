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
 * MANIFEST: interface definition for aegis/aer/expr/struct.c
 */

#ifndef AEGIS_AER_EXPR_STRUCT_H
#define AEGIS_AER_EXPR_STRUCT_H

#include <aer/expr.h>

rpt_expr_ty *rpt_expr_struct _((void));

struct symtab_ty *rpt_expr_struct__symtab_query _((void));

#endif /* AEGIS_AER_EXPR_STRUCT_H */
