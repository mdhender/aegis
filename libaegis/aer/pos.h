/*
 *	aegis - project change supervisor
 *	Copyright (C) 1996, 2002, 2003 Peter Miller;
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
 * MANIFEST: interface definition for aegis/aer/pos.c
 */

#ifndef AEGIS_AER_POS_H
#define AEGIS_AER_POS_H

#include <main.h>

struct sub_context_ty; /* existence */

typedef struct rpt_pos_ty rpt_pos_ty;
struct rpt_pos_ty
{
	long		reference_count;
	struct string_ty *file_name;
	long		line_number1;
	long		line_number2;
};

rpt_pos_ty *rpt_pos_alloc(struct string_ty *, long);
rpt_pos_ty *rpt_pos_copy(rpt_pos_ty *);
rpt_pos_ty *rpt_pos_union(rpt_pos_ty *, rpt_pos_ty *);
void rpt_pos_free(rpt_pos_ty *);
void rpt_pos_error(struct sub_context_ty *, rpt_pos_ty *, const char *);

#endif /* AEGIS_AER_POS_H */
