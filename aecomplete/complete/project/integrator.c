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
 * MANIFEST: functions to manipulate integrators
 */

#include <complete/private.h>
#include <complete/project/integrator.h>
#include <project.h>
#include <project/history.h>
#include <shell.h>


typedef struct complete_project_integrator_ty complete_project_integrator_ty;
struct complete_project_integrator_ty
{
    complete_ty     inherited;
    project_ty      *pp;
};


static void
destructor(complete_ty *cp)
{
    complete_project_integrator_ty *this_thing;

    this_thing = (complete_project_integrator_ty *)cp;
    project_free(this_thing->pp);
}


static void
perform(complete_ty *cp, shell_ty *sh)
{
    complete_project_integrator_ty *this_thing;
    string_ty       *prefix;
    size_t          j;
    string_ty       *name;

    this_thing = (complete_project_integrator_ty *)cp;
    prefix = shell_prefix_get(sh);
    for (j = 0; ; ++j)
    {
	name = project_integrator_nth(this_thing->pp, j);
	if (!name)
	    break;
	if (str_leading_prefix(name, prefix))
	    shell_emit(sh, name);
    }
}


static complete_vtbl_ty vtbl =
{
    destructor,
    perform,
    sizeof(complete_project_integrator_ty),
    "project integrator",
};


complete_ty *
complete_project_integrator(project_ty *pp)
{
    complete_ty     *result;
    complete_project_integrator_ty *this_thing;

    result = complete_new(&vtbl);
    this_thing = (complete_project_integrator_ty *)result;
    this_thing->pp = pp;
    return result;
}
