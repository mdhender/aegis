/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001, 2002 Peter Miller;
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
 * MANIFEST: interface definition for aels/list.c
 */

#ifndef AELS_LIST_H
#define AELS_LIST_H

#include <str_list.h>

struct change_ty; /* forward */
struct project_ty; /* forward */

extern int recursive_flag;
extern int long_flag;
extern int show_dot_files;
extern int show_removed_files;
extern int mode_flag;
extern int attr_flag;
extern int user_flag;
extern int group_flag;
extern int size_flag;
extern int when_flag;

void list(string_list_ty *, struct project_ty *, struct change_ty *);

#endif /* AELS_LIST_H */
