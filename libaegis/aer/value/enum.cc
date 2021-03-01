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
#include <common/str.h>
#include <libaegis/aer/value/boolean.h>
#include <libaegis/aer/value/enum.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/real.h>
#include <libaegis/aer/value/string.h>


rpt_value_enumeration::~rpt_value_enumeration()
{
}


rpt_value_enumeration::rpt_value_enumeration(int ii, const nstring &ss) :
    ival(ii),
    sval(ss)
{
}


rpt_value::pointer
rpt_value_enumeration::create(int ii, const nstring &ss)
{
    return pointer(new rpt_value_enumeration(ii, ss));
}


rpt_value::pointer
rpt_value_enumeration::stringize_or_null()
    const
{
    return rpt_value_string::create(sval);
}


rpt_value::pointer
rpt_value_enumeration::arithmetic_or_null()
    const
{
    return rpt_value_integer::create(ival);
}


rpt_value::pointer
rpt_value_enumeration::integerize_or_null()
    const
{
    return rpt_value_integer::create(ival);
}


rpt_value::pointer
rpt_value_enumeration::realize_or_null()
    const
{
    return rpt_value_integer::create(ival);
}


rpt_value::pointer
rpt_value_enumeration::booleanize_or_null()
    const
{
    return rpt_value_boolean::create(ival != 0);
}


const char *
rpt_value_enumeration::name()
    const
{
    return "enumeration";
}
