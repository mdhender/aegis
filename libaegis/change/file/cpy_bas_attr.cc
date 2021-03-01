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

#include <libaegis/change/file.h>


void
change_file_copy_basic_attributes(fstate_src_ty *to, fstate_src_ty *from)
{
    //
    // Make sure this stays in sync with the code in
    // aegis/aeip.c
    //
    to->usage = from->usage;

    if (to->attribute)
    {
	attributes_list_type.free(to->attribute);
	to->attribute = 0;
    }
    if (from->attribute)
	to->attribute = attributes_list_copy(from->attribute);

    if (to->uuid)
    {
	str_free(to->uuid);
	to->uuid = 0;
    }
    if (from->uuid)
	to->uuid = str_copy(from->uuid);
    to->executable = from->executable;
}
