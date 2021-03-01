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
	rpt_stmt_status_error
};
typedef enum rpt_stmt_status_ty rpt_stmt_status_ty;



struct rpt_stmt_ty;

typedef struct rpt_stmt_method_ty rpt_stmt_method_ty;
struct rpt_stmt_method_ty
{
	size_t		size;
	char		*name;
	void (*construct)_((struct rpt_stmt_ty *));
	void (*destruct)_((struct rpt_stmt_ty *));
	rpt_stmt_status_ty (*run)_((struct rpt_stmt_ty *));
};

#define RPT_STMT				\
	rpt_stmt_method_ty *method;		\
	long		reference_count;	\
	rpt_stmt_ty	**child;		\
	size_t		nchild;			\
	size_t		nchild_max;

typedef struct rpt_stmt_ty rpt_stmt_ty;
struct rpt_stmt_ty
{
	RPT_STMT
};

rpt_stmt_ty *rpt_stmt_alloc _((rpt_stmt_method_ty *));
void rpt_stmt_free _((rpt_stmt_ty *));
rpt_stmt_ty *rpt_stmt_copy _((rpt_stmt_ty *));

void rpt_stmt_append _((rpt_stmt_ty *parent, rpt_stmt_ty *child));
void rpt_stmt_prepend _((rpt_stmt_ty *parent, rpt_stmt_ty *child));

#define rpt_stmt_run(s) (s)->method->run(s)

#endif /* AEGIS_AER_STMT_H */
