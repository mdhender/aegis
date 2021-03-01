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
 * MANIFEST: interface definition for aegis/aer/lex.c
 */

#ifndef AEGIS_AER_LEX_H
#define AEGIS_AER_LEX_H

#include <main.h>

struct string_ty;

typedef struct rpt_lex_pos_ty rpt_lex_pos_ty;
struct rpt_lex_pos_ty
{
	long		reference_count;
	struct string_ty *file_name;
	long		line_number;
};

void rpt_lex_open _((struct string_ty *));
void rpt_lex_close _((void));

int report_lex _((void));
void report_error _((char *, ...));

rpt_lex_pos_ty *rpt_lex_pos_get _((void));
void rpt_lex_pos_free _((rpt_lex_pos_ty *));
void rpt_lex_error _((rpt_lex_pos_ty *, char *, ...));
rpt_lex_pos_ty *rpt_lex_pos_union _((rpt_lex_pos_ty *, rpt_lex_pos_ty *));
rpt_lex_pos_ty *rpt_lex_pos_copy _((rpt_lex_pos_ty *));

#endif /* AEGIS_AER_LEX_H */
