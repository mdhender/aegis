//
//      aegis - project change supervisor
//      Copyright (C) 2004, 2006, 2008, 2011, 2012 Peter Miller
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

#ifndef AEGET_GET_PROJECT_CHANGE_CAUSE_H
#define AEGET_GET_PROJECT_CHANGE_CAUSE_H

struct project; // forward
struct string_ty; // forward
struct string_list_ty; // forward

void get_project_change_cause(project *pp, string_ty *filename,
    string_list_ty *modifier);

#endif // AEGET_GET_PROJECT_CHANGE_CAUSE_H
// vim: set ts=8 sw=4 et :
