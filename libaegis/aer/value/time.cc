//
//	aegis - project change supervisor
//	Copyright (C) 1994, 1996, 2003-2008 Peter Miller
//	Copyright (C) 2007 Walter Franzini
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
#include <libaegis/aer/value/boolean.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/real.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/aer/value/time.h>


rpt_value_time::~rpt_value_time()
{
}


rpt_value_time::rpt_value_time(time_t arg) :
    value(arg)
{
}


rpt_value::pointer
rpt_value_time::create(time_t t)
{
    return pointer(new rpt_value_time(t));
}


rpt_value::pointer
rpt_value_time::stringize_or_null()
    const
{
    nstring s(ctime(&value), 24);
    return rpt_value_string::create(s);
}


rpt_value::pointer
rpt_value_time::arithmetic_or_null()
    const
{
    return rpt_value_integer::create(long(value));
}


rpt_value::pointer
rpt_value_time::integerize_or_null()
    const
{
    return rpt_value_integer::create(long(value));
}


rpt_value::pointer
rpt_value_time::realize_or_null()
    const
{
    return rpt_value_real::create(double(value));
}


rpt_value::pointer
rpt_value_time::booleanize_or_null()
    const
{
    return rpt_value_boolean::create(value != 0);
}


const char *
rpt_value_time::name()
    const
{
    return "time";
}
