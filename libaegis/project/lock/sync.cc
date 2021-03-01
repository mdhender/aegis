//
//	aegis - project change supervisor
//	Copyright (C) 2006 Peter Miller
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
// MANIFEST: implementation of the project_ty::lock_sync method
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
