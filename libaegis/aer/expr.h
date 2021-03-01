/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994-1996, 2002 Peter Miller;
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
    size_t	    size;
    char	    *name;
    void	    (*construct)(struct rpt_expr_ty *);
    void	    (*destruct)(struct rpt_expr_ty *);
    struct rpt_value_ty *(*evaluate)(struct rpt_expr_ty *);
    int	    (*lvalue)(struct rpt_expr_ty *);
};

#define RPT_EXPR				\
    rpt_expr_method_ty *method;			\
    long	    reference_count;		\
    struct rpt_pos_ty *pos;			\
    rpt_expr_ty     **child;			\
    size_t	    nchild;			\
    size_t	    nchild_max;

typedef struct rpt_expr_ty rpt_expr_ty;
struct rpt_expr_ty
{
    RPT_EXPR
};

rpt_expr_ty *rpt_expr_alloc(rpt_expr_method_ty *);
void rpt_expr_free(rpt_expr_ty *);
rpt_expr_ty *rpt_expr_copy(rpt_expr_ty *);
void rpt_expr_parse_error(rpt_expr_ty *, char *);
void rpt_expr_append(rpt_expr_ty *parent, rpt_expr_ty *child);
void rpt_expr_prepend(rpt_expr_ty *parent, rpt_expr_ty *child);

struct rpt_value_ty *rpt_expr_evaluate(rpt_expr_ty *ep, int resolve_deferred);
int rpt_expr_lvalue(rpt_expr_ty *);

#endif /* AEGIS_AER_EXPR_H */
