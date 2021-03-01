/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002, 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate aeifails
 */

#include <arglex2.h>
#include <command/aeifail.h>
#include <command/generic.h>
#include <command/private.h>
#include <complete/change/number.h>
#include <complete/filename.h>
#include <complete/nil.h>
#include <complete/project/name.h>
#include <cstate.h>
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
    string_ty      *project_name;
    project_ty     *pp;
    int            incomplete_filename;
    int            incomplete_change_number;

    arglex2_retable(0);
    arglex();
    project_name = 0;
    incomplete_filename = 0;
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

	case arglex_token_string:
	    /*
	     * Deprecated: strings lying around are file names.
	     * The preferred -file option will be handled by generic.
	     */
	    break;

	case arglex_token_string_incomplete:
	    return complete_filename(0);

	case arglex_token_reason:
	    switch (arglex())
	    {
	    default:
		continue;

	    case arglex_token_string:
		break;

	    case arglex_token_string_incomplete:
	    case arglex_token_number_incomplete:
		/* Sorry, can't help you. */
		return complete_nil();
	    }
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
     * We are going to complete a change number.
     */
    return complete_change_number(pp, 1 << cstate_state_being_integrated);
}


static command_vtbl_ty vtbl =
{
    destructor,
    completion_get,
    sizeof(command_ty),
    "aeifail",
};


command_ty *
command_aeifail()
{
    return command_new(&vtbl);
}
