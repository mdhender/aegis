//
//	aegis - project change supervisor
//	Copyright (C) 2002-2007 Peter Miller
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
// MANIFEST: functions to manipulate unknowns
//

#include <aecomplete/command/private.h>
#include <aecomplete/command/unknown.h>
#include <aecomplete/complete/nil.h>


static void
destructor(command_ty *)
{
}


static complete_ty *
parse_command_line(command_ty *)
{
    return complete_nil();
}


static command_vtbl_ty vtbl =
{
    destructor,
    parse_command_line,
    sizeof(command_ty),
    "unknown",
};


command_ty *
command_unknown()
{
    return command_new(&vtbl);
}
