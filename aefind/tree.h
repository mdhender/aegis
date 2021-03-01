//
//	aegis - project change supervisor
//	Copyright (C) 1997, 2002, 2004, 2005 Peter Miller;
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
// MANIFEST: interface definition for aefind/tree.c
//

#ifndef AEFIND_TREE_H
#define AEFIND_TREE_H

#include <main.h>

struct string_ty; // existence
struct stat; // existence

struct tree_ty
{
    struct tree_method_ty *method;
    long	    reference_count;
};

tree_ty *tree_copy(tree_ty *);
void tree_delete(tree_ty *);
void tree_print(tree_ty *);
struct rpt_value_ty *tree_evaluate(tree_ty *, struct string_ty *,
    struct string_ty *, struct string_ty *, struct stat *);
int tree_useful(tree_ty *);
int tree_constant(tree_ty *);
struct rpt_value_ty *tree_evaluate_constant(tree_ty *);
tree_ty *tree_optimize(tree_ty *);
tree_ty *tree_optimize_constant(tree_ty *);

#endif // AEFIND_TREE_H
