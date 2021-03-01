//
// aegis - project change supervisor
// Copyright (C) 2009 Walter Franzini
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
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef LIBAEGIS_SEARCH_PATH_BASE_GET_H
#define LIBAEGIS_SEARCH_PATH_BASE_GET_H

#include <common/nstring/list.h>
#include <libaegis/user.h>

nstring
search_path_base_get(const nstring_list &, user_ty::pointer);

#endif // LIBAEGIS_SEARCH_PATH_BASE_GET_H
