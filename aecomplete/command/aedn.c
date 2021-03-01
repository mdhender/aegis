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
 * MANIFEST: functions to manipulate aedns
 */

#include <arglex2.h>
#include <change.h>
#include <command/aedn.h>
#include <command/generic.h>
#include <command/private.h>
#include <complete/change/number.h>
#include <complete/nil.h>
#include <complete/project/delta.h>
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
    string_ty       *project_name;
    complete_ty     *result;
    int             incomplete_change_number;
    int             incomplete_delta_number;
    project_ty      *pp;

    arglex2_retable(0);
    arglex();
    project_name = 0;
    incomplete_change_number = 0;
    incomplete_delta_number = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    result = generic_argument_complete();
	    if (result)
		return result;
	    continue;

	case arglex_token_string:
	    /* what to name it, ignore */
	    break;

	case arglex_token_string_incomplete:
	    /* incomplete name */
	    break;

	case arglex_token_delta:
	    switch (arglex())
	    {
	    default:
		continue;

	    case arglex_token_number:
		break;

	    case arglex_token_number_incomplete:
	    case arglex_token_string_incomplete:
		incomplete_delta_number = 1;
		break;
	    }
	    break;

	case arglex_token_number:
	    /* delta number, ignore */
	    break;

	case arglex_token_number_incomplete:
	    incomplete_delta_number = 1;
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

	case arglex_token_delta_date:
	    switch (arglex())
	    {
	    default:
		continue;

	    case arglex_token_string:
	    case arglex_token_number:
		break;

	    case arglex_token_string_incomplete:
	    case arglex_token_number_incomplete:
		/* maybe "complete_project_delta_date" */
		return complete_nil();
		break;
	    }
	    break;

	case arglex_token_delta_from_change:
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
	return complete_change_number(pp, 1 << cstate_state_completed);

    /*
     * If we need to complete a delta number, we have the project now,
     * and the branch to use.
     */
    if (incomplete_delta_number)
	return complete_project_delta(pp);

    /*
     * Sorry, can't help you with making up the name.
     */
    return complete_nil();
}


static command_vtbl_ty vtbl =
{
    destructor,
    completion_get,
    sizeof(command_ty),
    "aedn",
};


command_ty *
command_aedn()
{
    return command_new(&vtbl);
}
