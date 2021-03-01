/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate privates
 */

#include <complete/private.h>
#include <mem.h>


complete_ty *
complete_new(vptr)
    complete_vtbl_ty *vptr;
{
    complete_ty     *this;

    this = mem_alloc(vptr->size);
    this->vptr = vptr;
    return this;
}


void
complete_delete(this)
    complete_ty     *this;
{
    if (this)
    {
	if (this->vptr && this->vptr->destructor)
	    this->vptr->destructor(this);
	this->vptr = 0;
	mem_free(this);
    }
}
