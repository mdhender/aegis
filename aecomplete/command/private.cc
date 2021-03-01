//
//	aegis - project change supervisor
//	Copyright (C) 2002-2005 Peter Miller;
//	All rights reserved.
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
// MANIFEST: functions to manipulate privates
//

#include <aecomplete/command/private.h>
#include <common/error.h> // for assert
#include <common/mem.h>


command_ty *
command_new(command_vtbl_ty *vptr)
{
    command_ty      *this_thing;

    assert(vptr);
    assert((size_t)vptr->size >= sizeof(command_ty));
    this_thing = (command_ty *)mem_alloc(vptr->size);
    this_thing->vptr = vptr;
    return this_thing;
}


void
command_delete(command_ty *this_thing)
{
    assert(this_thing);
    if (this_thing)
    {
	assert(this_thing->vptr);
	if (this_thing->vptr && this_thing->vptr->destructor)
	    this_thing->vptr->destructor(this_thing);
	this_thing->vptr = 0;
	mem_free(this_thing);
    }
}
