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
 * MANIFEST: functions to manipulate integrators
 */

#include <complete/user.h>
#include <complete/user/integrator.h>
#include <project.h>
#include <project/history.h>


static int yes _((project_ty *, string_ty *));

static int
yes(pp, name)
    project_ty      *pp;
    string_ty       *name;
{
    return project_integrator_query(pp, name);
}


complete_ty *
complete_user_integrator(pp)
    project_ty      *pp;
{
    return complete_user(pp, yes);
}


static int no _((project_ty *, string_ty *));

static int
no(pp, name)
    project_ty      *pp;
    string_ty       *name;
{
    return !project_integrator_query(pp, name);
}


complete_ty *
complete_user_integrator_not(pp)
    project_ty      *pp;
{
    return complete_user(pp, no);
}
