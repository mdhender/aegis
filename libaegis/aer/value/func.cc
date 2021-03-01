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
#include <libaegis/aer/func.h>
#include <libaegis/aer/value/func.h>
#include <libaegis/aer/value/string.h>



rpt_value_func::~rpt_value_func()
{
}


rpt_value_func::rpt_value_func(const rpt_func::pointer &arg) :
    value(arg)
{
}


rpt_value::pointer
rpt_value_func::create(const rpt_func::pointer &arg)
{
    return pointer(new rpt_value_func(arg));
}


rpt_value::pointer
rpt_value_func::stringize_or_null()
    const
{
    //
    // stringizing a function pointer
    // returns the name of the function
    //
    return rpt_value_string::create(value->name());
}


const char *
rpt_value_func::name()
    const
{
    return "function";
}


rpt_func::pointer
rpt_value_func::query()
    const
{
    return value;
}
