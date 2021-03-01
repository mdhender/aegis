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
 * MANIFEST: interface definition for aefind/tree/relative.c
 */

#ifndef AEFIND_TREE_RELATIVE_H
#define AEFIND_TREE_RELATIVE_H

#include <main.h>

struct tree_ty *tree_ge_new _((struct tree_ty *, struct tree_ty *));
struct tree_ty *tree_gt_new _((struct tree_ty *, struct tree_ty *));
struct tree_ty *tree_le_new _((struct tree_ty *, struct tree_ty *));
struct tree_ty *tree_lt_new _((struct tree_ty *, struct tree_ty *));
struct tree_ty *tree_eq_new _((struct tree_ty *, struct tree_ty *));
struct tree_ty *tree_ne_new _((struct tree_ty *, struct tree_ty *));

#endif /* AEFIND_TREE_RELATIVE_H */