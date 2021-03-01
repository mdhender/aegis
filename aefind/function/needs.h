/*
 *	aegis - project change supervisor
 *	Copyright (C) 1997, 2002, 2003 Peter Miller;
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
 * MANIFEST: interface definition for aefind/function/needs.c
 */

#ifndef AEFIND_FUNCTION_NEEDS_H
#define AEFIND_FUNCTION_NEEDS_H

#include <main.h>

struct tree_list_ty; /* existence */

void function_needs_one(const char *, struct tree_list_ty *);
void function_needs_two(const char *, struct tree_list_ty *);

#endif /* AEFIND_FUNCTION_NEEDS_H */
