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
 * MANIFEST: interface definition for aefind/descend.c
 */

#ifndef AEFIND_DESCEND_H
#define AEFIND_DESCEND_H

#include <main.h>

struct stat; /* existence */
struct string_ty; /* existence */
struct tree_ty; /* existence */

enum descend_message_ty
{
	descend_message_dir_before,
	descend_message_dir_after,
	descend_message_file
};
typedef enum descend_message_ty descend_message_ty;

typedef void (*descend_callback_ty)_((void *arg, descend_message_ty,
	struct string_ty *, struct stat *));

void descend _((struct string_ty *, int, descend_callback_ty, void *));

struct string_ty *stat_stack _((struct string_ty *, struct stat *));

#endif /* AEFIND_DESCEND_H */