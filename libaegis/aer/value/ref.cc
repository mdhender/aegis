//
//	aegis - project change supervisor
//	Copyright (C) 1994, 1996, 2003-2008 Peter Miller
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
#include <common/trace.h>
#include <libaegis/aer/value/ref.h>


rpt_value_reference::~rpt_value_reference()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


rpt_value_reference::rpt_value_reference(const rpt_value::pointer &a_value) :
    value(a_value)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


rpt_value::pointer
rpt_value_reference::arithmetic_or_null()
    const
{
    return arithmetic(value);
}


rpt_value::pointer
rpt_value_reference::stringize_or_null()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return stringize(value);
}


rpt_value::pointer
rpt_value_reference::booleanize_or_null()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return booleanize(value);
}


rpt_value::pointer
rpt_value_reference::lookup(const rpt_value::pointer &rhs, bool lvalue)
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return value->lookup(rhs, lvalue);
}


rpt_value::pointer
rpt_value_reference::keys()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return value->keys();
}


rpt_value::pointer
rpt_value_reference::count()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return value->count();
}


const char *
rpt_value_reference::type_of()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return value->type_of();
}


rpt_value::pointer
rpt_value_reference::integerize_or_null()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return rpt_value::integerize(value);
}


rpt_value::pointer
rpt_value_reference::realize_or_null()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return rpt_value::realize(value);
}


const char *
rpt_value_reference::name()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return "reference";
}


rpt_value::pointer
rpt_value_reference::create(const rpt_value::pointer &deeper)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer(new rpt_value_reference(deeper));
}


rpt_value::pointer
rpt_value_reference::get()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return value;
}


void
rpt_value_reference::set(const rpt_value::pointer &rhs)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    value = rhs;
}
