//
//	aegis - project change supervisor
//	Copyright (C) 2001-2008 Peter Miller
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

#include <common/error.h> // for assert
#include <common/trace.h>
#include <libaegis/aer/value/pconf.h>
#include <libaegis/aer/value/struct.h>
#include <libaegis/change.h>
#include <libaegis/pconf.h>


rpt_value_pconf::~rpt_value_pconf()
{
    change_free(cp);
}


rpt_value_pconf::rpt_value_pconf(const change::pointer &a_cp) :
    cp(change_copy(a_cp))
{
}


rpt_value::pointer
rpt_value_pconf::create(const change::pointer &a_cp)
{
    return pointer(new rpt_value_pconf(a_cp));
}


void
rpt_value_pconf::grab()
    const
{
    //
    // construct the project config file's value, assuming it exists
    //
    trace(("rpt_value_pconf::grab(this = %08lX)\n{\n", (long)this));
    assert(!value);
    assert(cp);
    pconf_ty *pconf_data = change_pconf_get(cp, 0);

    //
    // create the result value
    //
    value = pconf_type.convert(&pconf_data);
    assert(value);
    assert(dynamic_cast<rpt_value_struct *>(value.get()));

    trace(("value = %08lX;\n", (long)value.get()));
    trace(("}\n"));
}


rpt_value::pointer
rpt_value_pconf::lookup(const rpt_value::pointer &rhs, bool lval)
    const
{
    trace(("rpt_value_pconf::lookup(this = %08lX)\n", (long)this));
    if (!value)
	grab();
    assert(value);
    if (value->is_an_error())
        return value;

    return value->lookup(rhs, lval);
}


rpt_value::pointer
rpt_value_pconf::keys()
    const
{
    trace(("rpt_value_pconf::keys(this = %08lX)\n", (long)this));
    if (!value)
	grab();
    assert(value);
    if (value->is_an_error())
        return value;

    return value->keys();
}


rpt_value::pointer
rpt_value_pconf::count()
    const
{
    trace(("rpt_value_pconf::count(this = %08lX)\n", (long)this));
    if (!value)
	grab();
    assert(value);
    if (value->is_an_error())
	return value;

    return value->count();
}


const char *
rpt_value_pconf::type_of()
    const
{
    trace(("rpt_value_pconf::type_of(this = %08lX)\n", (long)this));
    if (!value)
	grab();
    assert(value);
    return value->type_of();
}


rpt_value::pointer
rpt_value_pconf::undefer_or_null()
    const
{
    trace(("rpt_value_pconf::undefer(this = %08lX)\n", (long)this));
    if (!value)
	grab();
    assert(value);
    return value;
}


const char *
rpt_value_pconf::name()
    const
{
    return "pconf";
}


bool
rpt_value_pconf::is_a_struct()
    const
{
    if (!value)
        return true;
    return value->is_a_struct();
}
