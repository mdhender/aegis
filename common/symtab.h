//
//	aegis - project change supervisor
//	Copyright (C) 1994, 2002-2004 Peter Miller.
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: interface definition for common/symtab.c
//

#ifndef FMTGEN_SYMTAB_H
#define FMTGEN_SYMTAB_H

#include <str.h>

/** \addtogroup Symtab
  * \brief Symbols table interface
  * \ingroup Common
  * @{
  */

struct symtab_row_ty
{
    string_ty	    *key;
    void	    *data;
    symtab_row_ty   *overflow;
};

struct symtab_ty
{
    symtab_ty	    *chain;
    void	   (*reap)(void *);
    symtab_row_ty   **hash_table;
    str_hash_ty	    hash_modulus;
    str_hash_ty	    hash_mask;
    str_hash_ty	    hash_load;
};

symtab_ty *symtab_alloc(int);
void symtab_free(symtab_ty *);
void *symtab_query(symtab_ty *, string_ty *);
string_ty *symtab_query_fuzzy(symtab_ty *, string_ty *);
void symtab_assign(symtab_ty *, string_ty *, void *);
void symtab_assign_push(symtab_ty *, string_ty *, void *);
void symtab_delete(symtab_ty *, string_ty *);
void symtab_dump(symtab_ty *, char *);
void symtab_walk(symtab_ty *stp, void (*func)(symtab_ty *stp,
    string_ty *key, void *data, void *arg), void *arg);

/** @} */
#endif // FMTGEN_SYMTAB_H
