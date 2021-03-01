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
 * MANIFEST: interface definition for common/symtab.c
 */

#ifndef FMTGEN_SYMTAB_H
#define FMTGEN_SYMTAB_H

#include <str.h>

typedef struct symtab_row_ty symtab_row_ty;
struct symtab_row_ty
{
	string_ty	*key;
	void		*data;
	symtab_row_ty	*overflow;
};

typedef struct symtab_ty symtab_ty;
struct symtab_ty
{
	symtab_ty	*chain;
	void		(*reap)_((void *));
	symtab_row_ty	**hash_table;
	str_hash_ty	hash_modulus;
	str_hash_ty	hash_cutover;
	str_hash_ty	hash_cutover_mask;
	str_hash_ty	hash_cutover_split_mask;
	str_hash_ty	hash_split;
	str_hash_ty	hash_load;
};

symtab_ty *symtab_alloc _((int));
void symtab_free _((symtab_ty *));
void *symtab_query _((symtab_ty *, string_ty *));
string_ty *symtab_query_fuzzy _((symtab_ty *, string_ty *));
void symtab_assign _((symtab_ty *, string_ty *, void *));
void symtab_assign_push _((symtab_ty *, string_ty *, void *));
void symtab_delete _((symtab_ty *, string_ty *));
void symtab_dump _((symtab_ty *, char *));
void symtab_walk _((symtab_ty *stp, void (*func)_((symtab_ty *stp,
	string_ty *key, void *data, void *arg)), void *arg));

#endif /* FMTGEN_SYMTAB_H */