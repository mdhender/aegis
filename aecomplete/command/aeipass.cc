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
// MANIFEST: functions to manipulate aeipasss
//

#include <libaegis/arglex2.h>
#include <libaegis/cstate.h>
#include <libaegis/project.h>
#include <libaegis/user.h>
#include <libaegis/zero.h>

#include <aecomplete/command/aeipass.h>
#include <aecomplete/command/generic.h>
#include <aecomplete/command/private.h>
#include <aecomplete/complete/change/number.h>
#include <aecomplete/complete/project/name.h>


static void
destructor(command_ty *)
{
}


static complete_ty *
completion_get(command_ty *)
{
    complete_ty    *result;
    string_ty      *project_name;
    project_ty     *pp;

    arglex2_retable(0);
    arglex();
    project_name = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    result = generic_argument_complete();
	    if (result)
		return result;
	    continue;

	case arglex_token_change:
	    switch (arglex())
	    {
	    default:
		continue;

	    case arglex_token_number:
		break;

	    case arglex_token_number_incomplete:
	    case arglex_token_string_incomplete:
		// incomplete change number
		break;
	    }
	    break;

	case arglex_token_number:
	    break;

	case arglex_token_number_incomplete:
	    // incomplete change number
	    break;

	case arglex_token_project:
	    switch (arglex())
	    {
	    default:
		continue;

	    case arglex_token_string:
		project_name = str_from_c(arglex_value.alv_string);
		break;

	    case arglex_token_string_incomplete:
	    case arglex_token_number_incomplete:
		return complete_project_name();
	    }
	    break;

	case arglex_token_string:
	    project_name = str_from_c(arglex_value.alv_string);
	    break;

	case arglex_token_string_incomplete:
	    if (!project_name || *arglex_value.alv_string)
		return complete_project_name();
	    // incomplete change number
	    break;
	}
	arglex();
    }

    //
    // Work out which project to use.
    //
    if (!project_name)
    {
        nstring n = user_ty::create()->default_project();
	project_name = str_copy(n.get_ref());
    }
    pp = project_alloc(project_name);
    pp->bind_existing();

    //
    // We are going to complete a change number.
    //
    return complete_change_number(pp, 1 << cstate_state_being_integrated);
}


static command_vtbl_ty vtbl =
{
    destructor,
    completion_get,
    sizeof(command_ty),
    "aeipass",
};


command_ty *
command_aeipass()
{
    return command_new(&vtbl);
}
