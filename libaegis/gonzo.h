//
//	aegis - project change supervisor
//	Copyright (C) 1991-2006 Peter Miller;
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
// MANIFEST: interface definition for aegis/gonzo.c
//

#ifndef GONZO_H
#define GONZO_H

#include <common/main.h>
#include <libaegis/gstate.h>

struct string_list_ty; // existence
struct project_ty; // existence

void gonzo_gstate_write(void);

void gonzo_library_append(const char *path);

struct string_ty *gonzo_project_home_path_from_name(struct string_ty *);
void gonzo_project_list(struct string_list_ty *result);
void gonzo_project_list_user(string_ty *, struct string_list_ty *result);
void gonzo_project_add(struct project_ty *);
void gonzo_project_delete(struct project_ty *);
void gonzo_gstate_write(void);
void gonzo_gstate_lock_prepare_new(void);

string_ty *gonzo_lockpath_get(void);
string_ty *gonzo_ustate_path(string_ty *project_name, string_ty *user_name);

void gonzo_become(void);
void gonzo_become_undo(void);

void gonzo_report_path(struct string_list_ty *);

void gonzo_alias_add(struct project_ty *, string_ty *);
void gonzo_alias_delete(string_ty *);
void gonzo_alias_list(struct string_list_ty *);
struct string_ty *gonzo_alias_to_actual(string_ty *);
int gonzo_alias_acceptable(string_ty *);

#endif // GONZO_H
