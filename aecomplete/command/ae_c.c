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
 * MANIFEST: functions to manipulate ae_cs
 */

#include <change.h>
#include <command/ae_c.h>
#include <command/private.h>
#include <complete/change/number.h>
#include <cstate.h>
#include <project.h>
#include <user.h>


static void destructor _((command_ty *));

static void
destructor(this)
    command_ty      *this;
{
}


static complete_ty *completion_get _((command_ty *));

static complete_ty *
completion_get(cmd)
    command_ty      *cmd;
{
    string_ty       *project_name;
    project_ty      *pp;

    /*
     * Work out which project to use.
     */
    project_name = user_default_project();
    pp = project_alloc(project_name);
    project_bind_existing(pp);

    /*
     * We are going to complete a change number.
     *
     * They could want it for just about anything, but we will only
     * suggest active changes, which is more likely to be useful.
     */
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


static command_vtbl_ty vtbl =
{
    destructor,
    completion_get,
    sizeof(command_ty),
    "ae_c",
};


command_ty *
command_ae_c()
{
    return command_new(&vtbl);
}