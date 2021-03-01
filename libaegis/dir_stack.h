/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002 Peter Miller;
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
 * MANIFEST: interface definition for libaegis/dir_stack.c
 */

#ifndef LIBAEGIS_DIR_STACK_H
#define LIBAEGIS_DIR_STACK_H

#include <main.h>
#include <str_list.h>

enum dir_stack_walk_message_t
{
	dir_stack_walk_dir_before,
	dir_stack_walk_dir_after,
	dir_stack_walk_file,
	dir_stack_walk_special,
	dir_stack_walk_symlink
};
typedef enum dir_stack_walk_message_t dir_stack_walk_message_t;

struct stat; /* forward */

typedef void (*dir_stack_walk_callback_t)_((void *arg,
	dir_stack_walk_message_t msg, string_ty *relpath, struct stat *statbuf,
	int depth));
void dir_stack_walk _((string_list_ty *, string_ty *, dir_stack_walk_callback_t,
	void *arg));
string_ty *dir_stack_find _((string_list_ty *stack, size_t start_pos,
	string_ty *path, struct stat *statbuf, int *depth));
void dir_stack_stat _((string_list_ty *stack, string_ty *path,
	struct stat *statbuf, int *depth));
string_ty *dir_stack_relative _((string_list_ty *, string_ty *));

#endif /* LIBAEGIS_DIR_STACK_H */
