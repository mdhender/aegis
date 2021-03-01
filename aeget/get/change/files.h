//
// aegis - project change supervisor
// Copyright (C) 2003, 2005-2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef AEGET_GET_CHANGE_FILES_H
#define AEGET_GET_CHANGE_FILES_H

struct string_ty; // modifier
struct string_list_ty; // modifier

void get_change_files(change::pointer pp, struct string_ty *filename,
    struct string_list_ty *modifier);

#endif // AEGET_GET_CHANGE_FILES_H
// vim: set ts=8 sw=4 et :
