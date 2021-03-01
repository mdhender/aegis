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
 * MANIFEST: interface definition for aefind/tree/bitwise.c
 */

#ifndef AEFIND_TREE_BITWISE_H
#define AEFIND_TREE_BITWISE_H

#include <main.h>

struct tree_ty *tree_bitwise_and_new(struct tree_ty *, struct tree_ty *);
struct tree_ty *tree_bitwise_or_new(struct tree_ty *, struct tree_ty *);
struct tree_ty *tree_bitwise_xor_new(struct tree_ty *, struct tree_ty *);
struct tree_ty *tree_bitwise_not_new(struct tree_ty *);
struct tree_ty *tree_shift_left_new(struct tree_ty *, struct tree_ty *);
struct tree_ty *tree_shift_right_new(struct tree_ty *, struct tree_ty *);

#endif /* AEFIND_TREE_BITWISE_H */
