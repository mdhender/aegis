//
//      aegis - project change supervisor
//      Copyright (C) 1991-2008, 2011, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#ifndef GONZO_H
#define GONZO_H

#include <libaegis/gstate.fmtgen.h>

class nstring; // forward
class nstring_list; // forward
class project; // forward
class string_list_ty; // forward

void gonzo_gstate_write(void);

void gonzo_library_append(const char *path);

struct string_ty *gonzo_project_home_path_from_name(struct string_ty *);
void gonzo_project_list(struct string_list_ty *result);
void gonzo_project_list_user(const nstring &, nstring_list &result);
void gonzo_project_add(struct project *);
void gonzo_project_delete(struct project *);
void gonzo_gstate_write(void);
void gonzo_gstate_lock_prepare_new(void);

string_ty *gonzo_lockpath_get(void);
nstring gonzo_ustate_path(const nstring &project_name,
    const nstring &user_name);

void gonzo_become(void);
void gonzo_become_undo(void);

void gonzo_report_path(struct string_list_ty *);

void gonzo_alias_add(struct project *, string_ty *);
void gonzo_alias_delete(string_ty *);
void gonzo_alias_list(struct string_list_ty *);
struct string_ty *gonzo_alias_to_actual(string_ty *);
int gonzo_alias_acceptable(string_ty *);

#endif // GONZO_H
// vim: set ts=8 sw=4 et :
