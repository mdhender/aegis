/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993 Peter Miller.
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
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: interface definition for aegis/gonzo.c
 */

#ifndef GONZO_H
#define GONZO_H

#include <main.h>
#include <gstate.h>

struct wlist;
struct project_ty;

void gonzo_gstate_write _((void));

void gonzo_library_append _((char *path));

struct string_ty *gonzo_project_home_path_from_name _((struct string_ty *));
void gonzo_project_list _((struct wlist *result));
void gonzo_project_list_user _((string_ty *, struct wlist *result));
void gonzo_project_add _((struct project_ty *));
void gonzo_project_delete _((struct project_ty *));
void gonzo_gstate_write _((void));
void gonzo_gstate_lock_prepare_new _((void));

string_ty *gonzo_lockpath_get _((void));
string_ty *gonzo_ustate_path _((string_ty *project_name, string_ty *user_name));

void gonzo_become _((void));
void gonzo_become_undo _((void));

#endif /* GONZO_H */
