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
 * MANIFEST: interface definition for aegis/aer/func.c
 */

#ifndef AEGIS_AER_FUNC_H
#define AEGIS_AER_FUNC_H

#include <ac/stddef.h>
#include <main.h>

struct rpt_expr_ty;
struct rpt_value_ty;
struct symtab_ty;

typedef struct rpt_func_ty rpt_func_ty;
struct rpt_func_ty
{
	char		*name;
	int		optimizable;
	int (*verify)_((struct rpt_expr_ty *ep));
	struct rpt_value_ty *(*run)_((struct rpt_expr_ty *, size_t argc,
		struct rpt_value_ty **argv));
};

void rpt_func_init _((struct symtab_ty *));

#endif /* AEGIS_AER_FUNC_H */
