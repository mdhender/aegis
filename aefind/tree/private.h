/*
 *	aegis - project change supervisor
 *	Copyright (C) 1997, 2002 Peter Miller;
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
 * MANIFEST: interface definition for aefind/tree/private.c
 */

#ifndef AEFIND_TREE_PRIVATE_H
#define AEFIND_TREE_PRIVATE_H

#include <tree.h>

typedef struct tree_method_ty tree_method_ty;
struct tree_method_ty
{
    int             size;
    char            *name;
    void	    (*destructor)(tree_ty *);
    void	    (*print)(tree_ty *);
    struct rpt_value_ty *(*evaluate)(tree_ty *, struct string_ty *,
		    struct stat *);
    int		    (*useful)(tree_ty *);
    int		    (*constant)(tree_ty *);
    tree_ty	    *(*optimize)(tree_ty *);
};

tree_ty *tree_new(tree_method_ty *);

#endif /* AEFIND_TREE_PRIVATE_H */
