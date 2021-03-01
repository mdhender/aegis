//
//	aegis - project change supervisor
//	Copyright (C) 2005-2008 Peter Miller
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

#include <libaegis/change.h>
#include <common/nstring.h>


bool
change_diff_required(change::pointer cp)
{
    pconf_ty *pconf_data = change_pconf_get(cp, 0);
    if (!pconf_data)
	return true;
    if (!pconf_data->diff_command)
	return true;
    static nstring exit_0("exit 0");
    return (nstring(pconf_data->diff_command) != exit_0);
}
