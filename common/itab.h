/*
 *	aegis - project change supervisor
 *	Copyright (C) 1998 Peter Miller;
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
 * MANIFEST: interface definition for common/itab.c
 */

#ifndef COMMON_ITAB_H
#define COMMON_ITAB_H

#include <main.h>

typedef long itab_key_ty;

typedef struct itab_row_ty itab_row_ty;
struct itab_row_ty
{
	itab_key_ty	key;
	void		*data;
	itab_row_ty	*overflow;
};

typedef struct itab_ty itab_ty;
struct itab_ty
{
	void		(*reap)_((void *));
	itab_row_ty	**hash_table;
	itab_key_ty	hash_modulus;
	itab_key_ty	hash_cutover;
	itab_key_ty	hash_cutover_mask;
	itab_key_ty	hash_cutover_split_mask;
	itab_key_ty	hash_split;
	itab_key_ty	load;
};

itab_ty *itab_alloc _((int));
void itab_free _((itab_ty *));
void *itab_query _((itab_ty *, itab_key_ty));
void itab_assign _((itab_ty *, itab_key_ty, void *));
void itab_delete _((itab_ty *, itab_key_ty));
void itab_walk _((itab_ty *, void (*)(itab_ty *, itab_key_ty, void *, void *),
	void *));

#endif /* COMMON_ITAB_H */