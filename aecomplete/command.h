//
//	aegis - project change supervisor
//	Copyright (C) 2002, 2004-2006, 2008 Peter Miller
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

#ifndef AECOMPLETE_COMMAND_H
#define AECOMPLETE_COMMAND_H

#include <common/str.h>

struct command_ty
{
    struct command_vtbl_ty *vptr;
};

command_ty *command_find(string_ty *);
struct complete_ty *command_completion_get(command_ty *);
const char *command_name(command_ty *);
void command_delete(command_ty *);

#endif // AECOMPLETE_COMMAND_H
