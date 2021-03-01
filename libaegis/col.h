/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1993, 2001, 2002 Peter Miller.
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
 * MANIFEST: interface definition for aegis/col.c
 */

#ifndef COL_H
#define COL_H

#include <main.h>

struct string_ty; /* forward */

typedef struct col_ty col_ty;
struct col_ty
{
	struct col_vtbl_ty *vptr;
};

col_ty *col_open(struct string_ty *filename);
void col_close(col_ty *p);
struct output_ty *col_create(col_ty *p, int left, int right,
	const char *title);
void col_title(col_ty *, const char *, const char *);
void col_eoln(col_ty *);
void col_need(col_ty *, int);
void col_eject(col_ty *);

#endif /* COL_H */
