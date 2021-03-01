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

#include <common/error.h>
#include <libaegis/lock.h>
#include <libaegis/project.h>


void
project_ty::lock_sync()
{
    if (parent)
	this_is_a_bug();
    long n = ::lock_magic();
    if (lock_magic == n)
	return;
    lock_magic = n;

    if (pstate_data && !is_a_new_file)
    {
	pstate_type.free(pstate_data);
	pstate_data = 0;
    }
    file_list_invalidate();
}
