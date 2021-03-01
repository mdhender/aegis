//
//	aegis - project change supervisor
//	Copyright (C) 1994, 1996, 1999, 2003-2008 Peter Miller
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

#include <common/ac/limits.h>
#include <common/ac/math.h>

#include <common/error.h>
#include <common/trace.h>
#include <libaegis/aer/value/boolean.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/real.h>
#include <libaegis/aer/value/string.h>


rpt_value_integer::~rpt_value_integer()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


rpt_value_integer::rpt_value_integer(long arg) :
    value(arg)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


rpt_value::pointer
rpt_value_integer::create(long arg)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer(new rpt_value_integer(arg));
}


rpt_value::pointer
rpt_value_integer::stringize_or_null()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return rpt_value_string::create(nstring::format("%ld", value));
}


rpt_value::pointer
rpt_value_integer::realize_or_null()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return rpt_value_real::create(value);
}


rpt_value::pointer
rpt_value_integer::booleanize_or_null()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return rpt_value_boolean::create(value != 0);
}


const char *
rpt_value_integer::name()
    const
{
    return "integer";
}


long
rpt_value_integer::query()
    const
{
    return value;
}
