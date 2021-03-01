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
#include <aecomplete/command/aend.h>
#include <aecomplete/command/generic.h>
#include <aecomplete/command/private.h>
#include <aecomplete/complete/project/name.h>
#include <aecomplete/complete/user/developer.h>
#include <libaegis/project.h>
#include <libaegis/user.h>


static void
destructor(command_ty *)
{
}


static complete_ty *
completion_get(command_ty *)
{
    string_ty       *project_name;
    complete_ty     *result;
    project_ty      *pp;

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

	case arglex_token_user:
	    switch (arglex())
	    {
	    case arglex_token_string:
	    case arglex_token_string_incomplete:
	    case arglex_token_number_incomplete:
		break;

	    default:
		continue;
	    }
	    break;

	case arglex_token_string:
	case arglex_token_string_incomplete:
	case arglex_token_number_incomplete:
	    break;

	case arglex_token_project:
	    switch (arglex())
	    {
	    case arglex_token_string:
		project_name = str_from_c(arglex_value.alv_string);
		break;

	    case arglex_token_string_incomplete:
	    case arglex_token_number_incomplete:
		return complete_project_name();

	    default:
		continue;
	    }
	    break;

	case arglex_token_wait:
	case arglex_token_wait_not:
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
    // We are going to complete a user name.
    //
    return complete_user_developer_not(pp);
}


static command_vtbl_ty vtbl =
{
    destructor,
    completion_get,
    sizeof(command_ty),
    "aend",
};


command_ty *
command_aend()
{
    return command_new(&vtbl);
}
