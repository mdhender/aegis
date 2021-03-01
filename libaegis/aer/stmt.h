/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1996, 2002, 2003 Peter Miller;
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
 * MANIFEST: interface definition for aegis/aer/stmt.c
 */

#ifndef AEGIS_AER_STMT_H
#define AEGIS_AER_STMT_H

#include <ac/stddef.h>
#include <main.h>

enum rpt_stmt_status_ty
{
    rpt_stmt_status_normal,
    rpt_stmt_status_break,
    rpt_stmt_status_continue,
    rpt_stmt_status_return,
    rpt_stmt_status_error
};
typedef enum rpt_stmt_status_ty rpt_stmt_status_ty;

typedef struct rpt_stmt_result_ty rpt_stmt_result_ty;
struct rpt_stmt_result_ty
{
    rpt_stmt_status_ty status;
    struct rpt_value_ty *thrown;
};


struct rpt_stmt_ty;

typedef struct rpt_stmt_method_ty rpt_stmt_method_ty;
struct rpt_stmt_method_ty
{
    size_t          size;
    const char      *name;
    void (*construct)(struct rpt_stmt_ty *);
    void (*destruct)(struct rpt_stmt_ty *);
    void (*run)(struct rpt_stmt_ty *, rpt_stmt_result_ty *);
};

#define RPT_STMT			\
    rpt_stmt_method_ty *method;		\
    long            reference_count;	\
    rpt_stmt_ty     **child;		\
    size_t          nchild;		\
    size_t          nchild_max;

typedef struct rpt_stmt_ty rpt_stmt_ty;
struct rpt_stmt_ty
{
    RPT_STMT
};

rpt_stmt_ty *rpt_stmt_alloc(rpt_stmt_method_ty *);
void rpt_stmt_free(rpt_stmt_ty *);
rpt_stmt_ty *rpt_stmt_copy(rpt_stmt_ty *);

void rpt_stmt_append(rpt_stmt_ty *parent, rpt_stmt_ty *child);
void rpt_stmt_prepend(rpt_stmt_ty *parent, rpt_stmt_ty *child);

#define rpt_stmt_run(s, r) (s)->method->run((s), (r))

#endif /* AEGIS_AER_STMT_H */
