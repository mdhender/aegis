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

#include <aecomplete/complete/user.h>
#include <aecomplete/complete/user/integrator.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>


static int
yes(project_ty *pp, string_ty *name)
{
    return project_integrator_query(pp, nstring(name));
}


complete_ty *
complete_user_integrator(project_ty *pp)
{
    return complete_user(pp, yes);
}


static int
no(project_ty *pp, string_ty *name)
{
    return !project_integrator_query(pp, nstring(name));
}


complete_ty *
complete_user_integrator_not(project_ty *pp)
{
    return complete_user(pp, no);
}
