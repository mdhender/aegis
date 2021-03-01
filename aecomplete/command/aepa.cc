//
//	aegis - project change supervisor
//	Copyright (C) 2002-2004 Peter Miller;
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
// MANIFEST: functions to manipulate aepas
//

#include <arglex2.h>
#include <command/aepa.h>
#include <command/generic.h>
#include <command/private.h>
#include <complete/nil.h>
#include <complete/project/name.h>
#include <project.h>
#include <user.h>
#include <zero.h>


static void
destructor(command_ty *this_thing)
{
}


static complete_ty *
completion_get(command_ty *cmd)
{
    complete_ty    *result;

    arglex2_retable(0);
    arglex();
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    //
	    // In particular, this handles the -file option.
	    //
	    result = generic_argument_complete();
	    if (result)
		return result;
	    continue;

	case arglex_token_project:
	    switch (arglex())
	    {
	    default:
		continue;

	    case arglex_token_string:
		break;

	    case arglex_token_string_incomplete:
	    case arglex_token_number_incomplete:
		return complete_project_name();
	    }
	    break;
	}
	arglex();
    }

    //
    // Sorry, can't help you.
    //
    return complete_nil();
}


static command_vtbl_ty vtbl =
{
    destructor,
    completion_get,
    sizeof(command_ty),
    "aepa",
};


command_ty *
command_aepa()
{
    return command_new(&vtbl);
}
