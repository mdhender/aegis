/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1993, 2002 Peter Miller.
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
 * MANIFEST: interface definition for aegis/dir.c
 */

#ifndef DIR_H
#define DIR_H

#include <main.h>
#include <str.h>

enum dir_walk_message_ty
{
	dir_walk_dir_before,
	dir_walk_dir_after,
	dir_walk_file,
	dir_walk_special,
	dir_walk_symlink
};
typedef enum dir_walk_message_ty dir_walk_message_ty;

struct stat;

typedef void (*dir_walk_callback_ty)(void *arg, dir_walk_message_ty,
	string_ty *, struct stat *);

void dir_walk(string_ty *, dir_walk_callback_ty, void *arg);

#endif /* DIR_H */
