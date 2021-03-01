//
//      aegis - project change supervisor
//      Copyright (C) 1994, 1996, 2003-2008 Peter Miller
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

#include <common/error.h>
#include <common/trace.h>
#include <libaegis/aer/value/boolean.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/real.h>
#include <libaegis/aer/value/string.h>


rpt_value_boolean::~rpt_value_boolean()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


rpt_value_boolean::rpt_value_boolean(bool a_value) :
    value(a_value)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


rpt_value::pointer
rpt_value_boolean::create(bool a_value)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer(new rpt_value_boolean(a_value));
}


rpt_value::pointer
rpt_value_boolean::arithmetic_or_null()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return rpt_value_integer::create(value);
}


rpt_value::pointer
rpt_value_boolean::integerize_or_null()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return rpt_value_integer::create(value);
}


rpt_value::pointer
rpt_value_boolean::realize_or_null()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return rpt_value_real::create(value);
}


rpt_value::pointer
rpt_value_boolean::stringize_or_null()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return rpt_value_string::create(value ? "true" : "false");
}


bool
rpt_value_boolean::query()
    const
{
    return value;
}


const char *
rpt_value_boolean::name()
    const
{
    return "boolean";
}
