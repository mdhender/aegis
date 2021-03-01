//
//	aegis - project change supervisor
//	Copyright (C) 2000, 2002-2007 Peter Miller
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
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//
// MANIFEST: functions to manipulate gets
//

#include <libaegis/change.h>
#include <libaegis/pconf.h>
#include <libaegis/project.h>


pconf_ty *
project_pconf_get(project_ty *pp)
{
    change::pointer cp;

    cp = pp->change_get();
    return change_pconf_get(cp, 0);
}
