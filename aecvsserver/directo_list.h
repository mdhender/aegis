//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#ifndef AECVSSERVER_DIRECTO_LIST_H
#define AECVSSERVER_DIRECTO_LIST_H

#include <aecvsserver/directory.h>

struct directory_list_ty
{
    size_t          length;
    size_t          maximum;
    directory_ty    *item;
};

void directory_list_constructor(directory_list_ty *);
void directory_list_destructor(directory_list_ty *);
void directory_list_rewind(directory_list_ty *);
void directory_list_append(directory_list_ty *, string_ty *, string_ty *);

#endif // AECVSSERVER_DIRECTO_LIST_H
