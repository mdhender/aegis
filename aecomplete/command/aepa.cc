//
//	aegis - project change supervisor
//	Copyright (C) 2002-2008 Peter Miller
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

#include <libaegis/arglex2.h>
#include <aecomplete/command/aepa.h>
#include <aecomplete/command/generic.h>
#include <aecomplete/command/private.h>
#include <aecomplete/complete/nil.h>
#include <aecomplete/complete/project/name.h>
#include <libaegis/project.h>
#include <libaegis/user.h>
#include <libaegis/zero.h>


static void
destructor(command_ty *)
{
}


static complete_ty *
completion_get(command_ty *)
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
