//
//	aegis - project change supervisor
//	Copyright (C) 1997, 2002, 2005, 2006 Peter Miller;
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
// MANIFEST: interface definition for aefind/function/stat.c
//

#ifndef AEFIND_FUNCTION_STAT_H
#define AEFIND_FUNCTION_STAT_H

#include <common/main.h>

struct tree_list_ty; // existence

struct tree_ty *function_atime(struct tree_list_ty *);
struct tree_ty *function_ctime(struct tree_list_ty *);
struct tree_ty *function_gid(struct tree_list_ty *);
struct tree_ty *function_ino(struct tree_list_ty *);
struct tree_ty *function_mode(struct tree_list_ty *);
struct tree_ty *function_mtime(struct tree_list_ty *);
struct tree_ty *function_nlink(struct tree_list_ty *);
struct tree_ty *function_size(struct tree_list_ty *);
struct tree_ty *function_uid(struct tree_list_ty *);
struct tree_ty *function_type(struct tree_list_ty *);

#endif // AEFIND_FUNCTION_STAT_H
