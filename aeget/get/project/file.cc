//
//	aegis - project change supervisor
//	Copyright (C) 2003-2007 Peter Miller
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
// MANIFEST: functions to manipulate files
//

#include <common/ac/stdio.h> // HACK

#include <libaegis/change.h>
#include <aeget/get/file.h>
#include <aeget/get/project/file.h>
#include <libaegis/project.h>
#include <common/str_list.h>


void
get_project_file(project_ty *pp, string_ty *filename, string_list_ty *modifier)
{
    fprintf(stderr, "%s: %d: get_project_file {\n", __FILE__, __LINE__);
    change::pointer cp = change_bogus(pp);
    get_file(cp, filename, modifier);
    change_free(cp);
    fprintf(stderr, "%s: %d: get_project_file }\n", __FILE__, __LINE__);
}
