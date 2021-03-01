/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002 Peter Miller;
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
 * MANIFEST: interface definition for aecomplete/command.c
 */

#ifndef AECOMPLETE_COMMAND_H
#define AECOMPLETE_COMMAND_H

#include <str.h>

typedef struct command_ty command_ty;
struct command_ty
{
    struct command_vtbl_ty *vptr;
};

command_ty *command_find(string_ty *);
struct complete_ty *command_completion_get(command_ty *);
const char *command_name(command_ty *);
void command_delete(command_ty *);

#endif /* AECOMPLETE_COMMAND_H */
