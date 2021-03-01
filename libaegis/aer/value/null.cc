//
// aegis - project change supervisor
// Copyright (C) 1994, 1996, 2003-2008 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/str.h>
#include <libaegis/aer/value/boolean.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/list.h>
#include <libaegis/aer/value/null.h>
#include <libaegis/aer/value/real.h>
#include <libaegis/aer/value/string.h>


rpt_value_null::~rpt_value_null()
{
}


rpt_value_null::rpt_value_null()
{
}


rpt_value::pointer
rpt_value_null::create()
{
    static rpt_value::pointer vp;
    if (!vp)
        vp = pointer(new rpt_value_null());
    return vp;
}


rpt_value::pointer
rpt_value_null::arithmetic_or_null()
    const
{
    return rpt_value_integer::create(0);
}


rpt_value::pointer
rpt_value_null::stringize_or_null()
    const
{
    return rpt_value_string::create("");
}


rpt_value::pointer
rpt_value_null::booleanize_or_null()
    const
{
    return rpt_value_boolean::create(false);
}


rpt_value::pointer
rpt_value_null::integerize_or_null()
    const
{
    return rpt_value_integer::create(0);
}


rpt_value::pointer
rpt_value_null::realize_or_null()
    const
{
    return rpt_value_real::create(0);
}


rpt_value::pointer
rpt_value_null::lookup(const rpt_value::pointer &, bool)
    const
{
    return create();
}


rpt_value::pointer
rpt_value_null::keys()
    const
{
    return rpt_value_list::create();
}


rpt_value::pointer
rpt_value_null::count()
    const
{
    return rpt_value_integer::create(0);
}


const char *
rpt_value_null::name()
    const
{
    return "nul";
}
