//
//	aegis - project change supervisor
//	Copyright (C) 2002-2005 Peter Miller;
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
// MANIFEST: functions to manipulate aecds
//

#include <arglex2.h>
#include <change.h>
#include <command/aecd.h>
#include <command/generic.h>
#include <command/private.h>
#include <complete/change/file_union.h>
#include <complete/change/number.h>
#include <complete/project/name.h>
#include <project.h>
#include <user.h>
#include <zero.h>


static void
destructor(command_ty *this_thing)
{
}


static void
do_nothing(void)
{
}


static complete_ty *
completion_get(command_ty *cmd)
{
    complete_ty    *result;
    string_ty      *project_name;
    long           change_number;
    project_ty     *pp;
    int            incomplete_change_number;
    user_ty        *up;
    change_ty      *cp;
    int            baserel;
    int            baseline;

    arglex2_retable(0);
    arglex();
    project_name = 0;
    change_number = 0;
    incomplete_change_number = 0;
    baseline = 0;
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
		if (arglex_value.alv_number == 0)
		    change_number = MAGIC_ZERO;
		else if (arglex_value.alv_number > 0)
		    change_number = arglex_value.alv_number;
		break;

	    case arglex_token_number_incomplete:
	    case arglex_token_string_incomplete:
		incomplete_change_number = 1;
		break;
	    }
	    break;

	case arglex_token_number:
	    if (arglex_value.alv_number == 0)
		change_number = MAGIC_ZERO;
	    else if (arglex_value.alv_number > 0)
		change_number = arglex_value.alv_number;
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

	case arglex_token_directory:
	case arglex_token_file:
	    // noise word, ignore
	    break;

	case arglex_token_string:
	    // whole file name, ignore
	    break;

	case arglex_token_string_incomplete:
	    // incomplete file name
	    break;

	case arglex_token_base_relative:
	case arglex_token_current_relative:
	    user_relative_filename_preference_argument(do_nothing);
	    break;

	case arglex_token_baseline:
	    baseline = 1;
	    break;
	}
	arglex();
    }

    //
    // Work out which project to use.
    //
    if (!project_name)
	project_name = user_default_project();
    pp = project_alloc(project_name);
    project_bind_existing(pp);

    //
    // If we need to complete a change number, we have the project now.
    //
    if (incomplete_change_number)
    {
	return
	    complete_change_number
	    (
		pp,
		~(
		    (1 << cstate_state_awaiting_development)
		|
		    (1 << cstate_state_completed)
		)
	    );
    }

    //
    // locate user data
    //
    up = user_executing(pp);

    //
    // locate change data
    //
    if (baseline)
	cp = project_change_get(pp);
    else
    {
	if (!change_number)
	    change_number = user_default_change(up);
	cp = change_alloc(pp, change_number);
	change_bind_existing(cp);
    }

    //
    // Figure out whether we are using base relative file names, or
    // current directory relative file names.
    //
    baserel =
	(
    	    user_relative_filename_preference
    	    (
		up,
		uconf_relative_filename_preference_base
    	    )
	==
    	    uconf_relative_filename_preference_base
	);

    //
    // We are going to complete a directory name.
    //
    return complete_change_file_union(cp, baserel, 1);
}


static command_vtbl_ty vtbl =
{
    destructor,
    completion_get,
    sizeof(command_ty),
    "aecd",
};


command_ty *
command_aecd()
{
    return command_new(&vtbl);
}
