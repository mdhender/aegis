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
// MANIFEST: functions to manipulate administratos
//

#include <aecomplete/complete/user.h>
#include <aecomplete/complete/user/administrato.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>


static int
yes(project_ty *pp, string_ty *name)
{
    return project_administrator_query(pp, name);
}


complete_ty *
complete_user_administrator(project_ty *pp)
{
    return complete_user(pp, yes);
}


static int
no(project_ty *pp, string_ty *name)
{
    return !project_administrator_query(pp, name);
}


complete_ty *
complete_user_administrator_not(project_ty *pp)
{
    return complete_user(pp, no);
}
