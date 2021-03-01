/*
 *	aegis - project change supervisor
 *	Copyright (C) 1997 Peter Miller;
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
 * MANIFEST: interface definition for aefind/tree/diadic.c
 */

#ifndef AEFIND_TREE_DIADIC_H
#define AEFIND_TREE_DIADIC_H

#include <tree/private.h>

typedef struct tree_diadic_ty tree_diadic_ty;
struct tree_diadic_ty
{
	tree_ty		inherited;
	tree_ty		*left;
	tree_ty		*right;
};

tree_ty *tree_diadic_new _((tree_method_ty *, tree_ty *, tree_ty *));
void tree_diadic_destructor _((tree_ty *));
void tree_diadic_print _((tree_ty *));
int tree_diadic_useful _((tree_ty *));
int tree_diadic_constant _((tree_ty *));
tree_ty *tree_diadic_optimize _((tree_ty *));

#endif /* AEFIND_TREE_DIADIC_H */