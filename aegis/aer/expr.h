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
 * MANIFEST: interface definition for aegis/aer/expr.c
 */

#ifndef AEGIS_AER_EXPR_H
#define AEGIS_AER_EXPR_H

#include <ac/stddef.h>
#include <main.h>

struct rpt_expr_ty;
struct rpt_value_ty;


typedef struct rpt_expr_method_ty rpt_expr_method_ty;
struct rpt_expr_method_ty
{
	size_t		size;
	char		*name;
	void (*construct)_((struct rpt_expr_ty *));
	void (*destruct)_((struct rpt_expr_ty *));
	struct rpt_value_ty *(*evaluate)_((struct rpt_expr_ty *));
	int (*lvalue)_((struct rpt_expr_ty *));
};

#define RPT_EXPR				\
	rpt_expr_method_ty *method;		\
	long		reference_count;	\
	struct rpt_lex_pos_ty *pos;		\
	rpt_expr_ty	**child;		\
	size_t		nchild;			\
	size_t		nchild_max;

typedef struct rpt_expr_ty rpt_expr_ty;
struct rpt_expr_ty
{
	RPT_EXPR
};

rpt_expr_ty *rpt_expr_alloc _((rpt_expr_method_ty *));
void rpt_expr_free _((rpt_expr_ty *));
rpt_expr_ty *rpt_expr_copy _((rpt_expr_ty *));
void rpt_expr_error _((rpt_expr_ty *, char *, ...));
void rpt_expr_append _((rpt_expr_ty *parent, rpt_expr_ty *child));
void rpt_expr_prepend _((rpt_expr_ty *parent, rpt_expr_ty *child));

#define rpt_expr_evaluate(ep) (ep)->method->evaluate(ep)
int rpt_expr_lvalue _((rpt_expr_ty *));

#endif /* AEGIS_AER_EXPR_H */
