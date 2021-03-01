/*
 *	aegis - project change supervisor
 *	Copyright (C) 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate directorys
 */

#include <change.h>
#include <project.h>
#include <project/directory.h>


string_ty *
project_directory_get(project_ty *pp, int resolve)
{
    for (;;)
    {
	change_ty	*cp;
	cstate_ty       *cstate_data;

	if (!pp->parent)
	    return project_baseline_path_get(pp, resolve);
	cp = project_change_get(pp);
	cstate_data = change_cstate_get(cp);
	if (cstate_data->state == cstate_state_being_developed)
	    return project_baseline_path_get(pp, resolve);
	pp = pp->parent;
    }
}
