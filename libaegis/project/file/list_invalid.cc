//
//	aegis - project change supervisor
//	Copyright (C) 2006, 2008 Peter Miller
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

#include <common/str_list.h>
#include <common/symtab.h>
#include <libaegis/project.h>


void
project_ty::file_list_invalidate()
{
    for (size_t j = 0; j < SIZEOF(file_list); ++j)
    {
	if (file_list[j])
	{
	    delete file_list[j];
	    file_list[j] = 0;
	}
    }
    for (size_t k = 0; k < SIZEOF(file_by_uuid); ++k)
    {
	if (file_by_uuid[k])
	{
	    delete file_by_uuid[k];
	    file_by_uuid[k] = 0;
	}
    }
}
