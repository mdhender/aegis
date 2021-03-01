//
//	aegis - project change supervisor
//	Copyright (C) 2006 Peter Miller;
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
// MANIFEST: implementation of the module class
//

#include <common/error.h> // for assert

#include <aecvsserver/module.h>


module::~module()
{
    assert(valid());
    if (ref)
    {
	ref->reference_count_down();
	ref = 0;
    }
}


module::module(module_ty *arg) :
    ref(arg)
{
    // Do not increase reference count.
    assert(valid());
}


module::module(const module &arg) :
    ref(arg.ref)
{
    if (ref)
	ref->reference_count_up();
    assert(valid());
}


module &
module::operator=(const module &arg)
{
    assert(valid());
    if (this != &arg && ref != arg.ref)
    {
	if (ref)
	{
	    ref->reference_count_down();
	    ref = 0;
	}
	ref = arg.ref;
	if (ref)
	    ref->reference_count_up();
	assert(valid());
    }
    return *this;
}


bool
module::valid()
    const
{
    return (!ref || ref->reference_count_valid());
}
