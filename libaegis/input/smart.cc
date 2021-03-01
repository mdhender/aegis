//
//      aegis - project change supervisor
//      Copyright (C) 2006, 2008, 2012 Peter Miller
//      Copyright (C) 2008, 2009 Walter Franzini
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/stack.h>
#include <common/trace.h>
#include <libaegis/input.h>


input::~input()
{
    trace(("input::~input(this = %p)\n{\n", this));
    assert(valid());
    close();
    trace(("}\n"));
}


input::input() :
    ref(0)
{
    trace(("input::input(this = %p)\n{\n", this));
    assert(valid());
    trace(("}\n"));
}


input::input(const input &arg) :
    ref(arg.ref)
{
    trace(("input::input(this = %p, arg = &%p)\n{\n", this,
        &arg));
    if (ref)
    {
        assert(!variable_is_on_stack((void *)ref));
        ref->reference_count_up();
    }
    assert(valid());
    trace(("}\n"));
}


input::input(input_ty::pointer arg) :
    ref(arg)
{
    trace(("input::input(this = %p, arg = %p)\n{\n", this,
        arg));
    assert(!variable_is_on_stack((void *)ref));
    assert(valid());
    // DO NOT increase the reference count, this input_ty pointer is
    // being given to us to manage.
    trace(("}\n"));
}


void
input::close()
{
    trace(("input::close(this = %p)\n{\n", this));
    assert(valid());
    if (ref)
    {
        assert(!variable_is_on_stack((void *)ref));
        input_ty::pointer tmp = ref;
        ref = 0;
        tmp->reference_count_down();
    }
    trace(("}\n"));
}


input &
input::operator=(const input &arg)
{
    trace(("input::operator=(this = %p)\n{\n", this));
    assert(arg.valid());
    assert(valid());
    if (this != &arg && ref != arg.ref)
    {
        close();
        if (arg.ref)
        {
            ref = arg.ref;
            if (ref)
            {
                assert(!variable_is_on_stack((void *)ref));
                ref->reference_count_up();
            }
        }
    }
    trace(("}\n"));
    return *this;
}


bool
input::valid()
    const
{
    return (ref == 0 || ref->reference_count_valid());
}


// vim: set ts=8 sw=4 et :
