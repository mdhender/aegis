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

#ifndef AECVSSERVER_DIRECTORY_H
#define AECVSSERVER_DIRECTORY_H

#include <common/str.h>

struct directory_ty
{
    string_ty	*client_side;
    string_ty	*server_side;
};

directory_ty *directory_new(string_ty *, string_ty *);
void directory_constructor(directory_ty *, string_ty *, string_ty *);
void directory_copy_constructor(directory_ty *, const directory_ty *);
void directory_destructor(directory_ty *);
void directory_delete(directory_ty *);

#endif // AECVSSERVER_DIRECTORY_H
