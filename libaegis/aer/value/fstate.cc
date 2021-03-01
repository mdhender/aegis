//
// aegis - project change supervisor
// Copyright (C) 1995, 1996, 1999, 2002-2008, 2011, 2012 Peter Miller
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

#include <common/ac/assert.h>

#include <common/trace.h>
#include <libaegis/aer/value/fstate.h>
#include <libaegis/change/file.h>


rpt_value_fstate::~rpt_value_fstate()
{
    trace(("rpt_value_fstate::~rpt_value_fstate(this = %p)\n", this));
    if (cp)
        change_free(cp);
}


rpt_value_fstate::rpt_value_fstate(const change::pointer &a_cp) :
    cp(change_copy(a_cp))
{
}


rpt_value::pointer
rpt_value_fstate::create(const change::pointer &a_cp)
{
    return pointer(new rpt_value_fstate(a_cp));
}


void
rpt_value_fstate::convert()
    const
{
    assert(!converted);
    fstate_ty *fstate_data = cp->fstate_get();
    converted = fstate_src_list_type.convert(&fstate_data->src);
}


rpt_value::pointer
rpt_value_fstate::lookup(const rpt_value::pointer &rhs, bool lval)
    const
{
    trace(("rpt_value_fstate::lookup(this = %p)\n", this));
    if (!converted)
        convert();
    return converted->lookup(rhs, lval);
}


rpt_value::pointer
rpt_value_fstate::keys()
    const
{
    trace(("rpt_value_fstate::keys(this = %p)\n{\n", this));
    if (!converted)
        convert();
    return converted->keys();
}


rpt_value::pointer
rpt_value_fstate::count()
    const
{
    trace(("rpt_value_fstate::count(this = %p)\n", this));
    if (!converted)
        convert();
    return converted->count();
}


const char *
rpt_value_fstate::type_of()
    const
{
    trace(("rpt_value_fstate::type_of(this = %p)\n{\n", this));
    if (!converted)
        convert();
    const char *result = converted->type_of();
    trace(("return \"%s\";\n", result));
    trace(("}\n"));
    return result;
}


rpt_value::pointer
rpt_value_fstate::undefer_or_null()
    const
{
    trace(("rpt_value_fstate::undefer(this = %p)\n{\n", this));
    if (!converted)
        convert();
    rpt_value::pointer result = converted;
    trace(("return %p\n", result.get()));
    trace(("}\n"));
    return result;
}


const char *
rpt_value_fstate::name()
    const
{
    return "fstate";
}


bool
rpt_value_fstate::is_a_struct()
    const
{
    if (!converted)
        return true;
    return converted->is_a_struct();
}


// vim: set ts=8 sw=4 et :
