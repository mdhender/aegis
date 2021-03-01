//
//	aegis - project change supervisor
//	Copyright (C) 2007 Walter Franzini
//	Copyright (C) 2007 Peter Miller
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
// MANIFEST: implementation of the
//      change::project_specific_setenv_performed_get method.
//

#include <libaegis/change.h>

bool
change::project_specific_setenv_performed_get()
    const
{
    return project_specific_setenv_performed;
}
