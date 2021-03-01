//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
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
#include <aeget/get/change/file_invento.h>
#include <aeget/get/project/file_invento.h>


void
get_project_file_inventory(project_ty *pp, string_ty *filename,
    string_list_ty *modifier)
{
    change::pointer cp = change_bogus(pp);
    get_change_file_inventory(cp, filename, modifier);
    change_free(cp);
}
