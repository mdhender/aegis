//
// aegis - project change supervisor
// Copyright (C) 2006, 2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

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


// vim: set ts=8 sw=4 et :
