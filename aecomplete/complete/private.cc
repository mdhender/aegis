//
//	aegis - project change supervisor
//	Copyright (C) 2002-2006, 2008 Peter Miller
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

#include <aecomplete/complete/private.h>
#include <common/mem.h>


complete_ty *
complete_new(complete_vtbl_ty *vptr)
{
    complete_ty     *this_thing;

    this_thing = (complete_ty *)mem_alloc(vptr->size);
    this_thing->vptr = vptr;
    return this_thing;
}


void
complete_delete(complete_ty *this_thing)
{
    if (this_thing)
    {
	if (this_thing->vptr && this_thing->vptr->destructor)
	    this_thing->vptr->destructor(this_thing);
	this_thing->vptr = 0;
	mem_free(this_thing);
    }
}
