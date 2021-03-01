//
// aegis - project change supervisor
// Copyright (C) 1994, 1996, 1998, 1999, 2002-2008, 2012 Peter Miller
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

#include <common/ac/ctype.h>
#include <common/ac/math.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/error.h>
#include <common/sizeof.h>
#include <common/str.h>
#include <common/trace.h>
#include <libaegis/aer/value/boolean.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/real.h>
#include <libaegis/aer/value/string.h>


rpt_value_string::~rpt_value_string()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


rpt_value_string::rpt_value_string(const nstring &arg) :
    value(arg)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


rpt_value::pointer
rpt_value_string::create(const nstring &arg)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer(new rpt_value_string(arg));
}


rpt_value::pointer
rpt_value_string::create(const char *arg)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return create(nstring(arg));
}


rpt_value::pointer
rpt_value_string::create(string_ty *arg)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return create(nstring(arg));
}


static bool
is_integer(const char *s, long &n)
{
    char *ep = 0;
    n = strtol(s, &ep, 0);
    return (ep != s && !*ep);
}


static bool
is_real(const char *s, double &n)
{
    char *ep = 0;
    n = strtod(s, &ep);
    return (ep != s && !*ep);
}


rpt_value::pointer
rpt_value_string::arithmetic_or_null()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    nstring val = value.trim();
    if (val.empty())
    {
        trace(("blank\n"));
        return rpt_value_integer::create(0);
    }
    long ival = 0;
    if (is_integer(val.c_str(), ival))
    {
        trace(("integer %ld\n", ival));
        return rpt_value_integer::create(ival);
    }
    double rval = 0;
    if (is_real(val.c_str(), rval))
    {
        trace(("real %g\n", rval));
        return rpt_value_real::create(rval);
    }
    return pointer();
}


rpt_value::pointer
rpt_value_string::integerize_or_null()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    nstring val = value.trim();
    if (val.empty())
    {
        trace(("blank\n"));
        return rpt_value_integer::create(0);
    }
    long ival = 0;
    if (is_integer(val.c_str(), ival))
    {
        trace(("integer %ld\n", ival));
        return rpt_value_integer::create(ival);
    }
    return pointer();
}


rpt_value::pointer
rpt_value_string::realize_or_null()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    nstring val = value.trim();
    if (val.empty())
    {
        trace(("blank\n"));
        return rpt_value_real::create(0);
    }
    double rval = 0;
    if (is_real(val.c_str(), rval))
    {
        trace(("real %g\n", rval));
        return rpt_value_real::create(rval);
    }
    return pointer();
}


rpt_value::pointer
rpt_value_string::booleanize_or_null()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    //
    // If we can make itan arithmetic type, then we can ask that value
    // to turn itself into a boolean value.
    //
    rpt_value::pointer vp = arithmetic_or_null();
    if (vp)
        return rpt_value::booleanize(vp);

    //
    // Look at a few fixed strings, just in case it looks like one of them.
    //
    struct table_t
    {
        const char *name;
        bool value;
    };
    static const table_t table[] =
    {
        { "false", false },
        { "no", false },
        { "true", true },
        { "yes", true },
    };
    nstring val = value.trim();
    for (const table_t *tp = table; tp < ENDOF(table); ++tp)
    {
        if (0 == strcasecmp(tp->name, val.c_str()))
            return rpt_value_boolean::create(tp->value);
    }

    //
    // Return the NULL pointer if all else fails.
    //
    return pointer();
}


const char *
rpt_value_string::name()
    const
{
    return "string";
}


nstring
rpt_value_string::query()
    const
{
    return value;
}


// vim: set ts=8 sw=4 et :
