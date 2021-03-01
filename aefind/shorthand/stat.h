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
 * MANIFEST: interface definition for aefind/shorthand/stat.c
 */

#ifndef AEFIND_SHORTHAND_STAT_H
#define AEFIND_SHORTHAND_STAT_H

#include <main.h>

struct string_ty; /* existence */

struct tree_ty *shorthand_atime _((struct tree_ty *(*)(struct tree_ty *,
	struct tree_ty *), int, int));
struct tree_ty *shorthand_ctime _((struct tree_ty *(*)(struct tree_ty *,
	struct tree_ty *), int, int));
struct tree_ty *shorthand_gid _((struct tree_ty *(*)(struct tree_ty *,
	struct tree_ty *), int));
struct tree_ty *shorthand_ino _((struct tree_ty *(*)(struct tree_ty *,
	struct tree_ty *), int));
struct tree_ty *shorthand_mode _((int));
struct tree_ty *shorthand_mtime _((struct tree_ty *(*)(struct tree_ty *,
	struct tree_ty *), int, int));
struct tree_ty *shorthand_newer _((struct string_ty *));
struct tree_ty *shorthand_nlink _((struct tree_ty *(*)(struct tree_ty *,
	struct tree_ty *), int));
struct tree_ty *shorthand_size _((struct tree_ty *(*)(struct tree_ty *,
	struct tree_ty *), int));
struct tree_ty *shorthand_uid _((struct tree_ty *(*)(struct tree_ty *,
	struct tree_ty *), int));
struct tree_ty *shorthand_type _((struct string_ty *));

#endif /* AEFIND_SHORTHAND_STAT_H */
