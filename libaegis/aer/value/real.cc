//
//	aegis - project change supervisor
//	Copyright (C) 1994, 1996, 1997, 2003-2008 Peter Miller
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

#include <common/error.h>
#include <libaegis/aer/value/boolean.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/real.h>
#include <libaegis/aer/value/string.h>


rpt_value_real::~rpt_value_real()
{
}


rpt_value_real::rpt_value_real(double arg) :
    value(arg)
{
}


rpt_value::pointer
rpt_value_real::create(double arg)
{
    return pointer(new rpt_value_real(arg));
}


rpt_value::pointer
rpt_value_real::stringize_or_null()
    const
{
    if (value == 0)
    {
        // avoid IEEE "negative zero" lunacy
        return rpt_value_string::create("0");
    }
    return rpt_value_string::create(nstring::format("%g", value));
}


rpt_value::pointer
rpt_value_real::booleanize_or_null()
    const
{
    return rpt_value_boolean::create(value != 0);
}


rpt_value::pointer
rpt_value_real::integerize_or_null()
    const
{
    if (value < LONG_MIN || value > LONG_MAX)
        return pointer();
    return rpt_value_integer::create(long(value));
}


const char *
rpt_value_real::name()
    const
{
    return "real";
}


double
rpt_value_real::query()
    const
{
    return value;
}
