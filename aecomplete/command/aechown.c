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
 * MANIFEST: functions to manipulate aechowns
 */

#include <arglex2.h>
#include <command/aechown.h>
#include <command/generic.h>
#include <command/private.h>
#include <complete/change/file_union.h>
#include <complete/change/number.h>
#include <complete/project/developer.h>
#include <complete/project/name.h>
#include <cstate.h>
#include <project.h>
#include <user.h>
#include <zero.h>


static void destructor _((command_ty *));

static void
destructor(this)
    command_ty     *this;
{
}


static complete_ty *completion_get _((command_ty *));

static complete_ty *
completion_get(cmd)
    command_ty     *cmd;
{
    complete_ty    *result;
    string_ty      *project_name;
    project_ty     *pp;
    int            incomplete_developer_name;
    int            incomplete_change_number;

    arglex2_retable(0);
    arglex();
    project_name = 0;
    incomplete_developer_name = 0;
    incomplete_change_number = 0;
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
		incomplete_change_number = 1;
		break;
	    }
	    break;

	case arglex_token_number:
	    break;

	case arglex_token_number_incomplete:
	    incomplete_change_number = 1;
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

	case arglex_token_user:
	    switch (arglex())
	    {
	    default:
		continue;

	    case arglex_token_string:
		break;

	    case arglex_token_string_incomplete:
	    case arglex_token_number_incomplete:
		incomplete_developer_name = 1;
		break;
	    }

	case arglex_token_string:
	    /* whole developer name, ignore */
	    break;

	case arglex_token_string_incomplete:
	    incomplete_developer_name = 1;
	    break;

	}
	arglex();
    }

    /*
     * Work out which project to use.
     */
    if (!project_name)
	project_name = user_default_project();
    pp = project_alloc(project_name);
    project_bind_existing(pp);

    /*
     * If we need to complete a change number, we have the project now.
     */
    if (incomplete_change_number)
	return complete_change_number(pp, 1 << cstate_state_being_developed);

    /*
     * We are going to complete a developer name.
     */
    return complete_project_developer(pp);
}


static command_vtbl_ty vtbl =
{
    destructor,
    completion_get,
    sizeof(command_ty),
    "aechown",
};


command_ty *
command_aechown()
{
    return command_new(&vtbl);
}
