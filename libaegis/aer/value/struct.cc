//
//      aegis - project change supervisor
//      Copyright (C) 1994-1996, 1999, 2002-2008, 2012 Peter Miller
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

#include <common/ac/assert.h>

#include <common/mem.h>
#include <common/symtab.h>
#include <common/trace.h>
#include <libaegis/aer/pos.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/list.h>
#include <libaegis/aer/value/null.h>
#include <libaegis/aer/value/ref.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/aer/value/struct.h>
#include <libaegis/sub.h>


rpt_value_struct::~rpt_value_struct()
{
    trace(("rpt_value_struct::~rpt_value_struct(this = %p)\n", this));
}


rpt_value_struct::rpt_value_struct()
{
    trace(("rpt_value_struct::rpt_value_struct(this = %p)\n", this));
}


rpt_value::pointer
rpt_value_struct::create()
{
    return pointer(new rpt_value_struct());
}


rpt_value::pointer
rpt_value_struct::lookup(const rpt_value::pointer &rhs, bool lvalue)
    const
{
    trace(("value_struct::lookup(this = %p, rhs = %p, lvalue = %d)\n",
        this, rhs.get(), lvalue));
    rpt_value::pointer rhs2 = rpt_value::stringize(rhs);
    rpt_value_string *rhs2sp = dynamic_cast<rpt_value_string *>(rhs2.get());
    if (!rhs2sp)
    {
        sub_context_ty sc;
        sc.var_set_charstar("Name1", name());
        sc.var_set_charstar("Name2", rhs->name());
        nstring s(sc.subst_intl(i18n("illegal lookup ($name1[$name2])")));
        return rpt_value_error::create(s);
    }
    nstring key(rhs2sp->query());
    trace(("find the \"%s\" datum\n", key.c_str()));
    rpt_value::pointer data = members.get(key);
    trace(("data = %p;\n", data.get()));
    if (!data)
    {
        rpt_value::pointer result = rpt_value_null::create();
        trace(("result = %p\n", result.get()));
        if (lvalue)
        {
            trace(("create new element\n"));
            result = rpt_value_reference::create(result);
            members.assign(key, result);
        }
        return result;
    }

    rpt_value_reference *rvrp = dynamic_cast<rpt_value_reference *>(data.get());
    assert(rvrp);
    if (rvrp && !lvalue)
    {
        //
        // this returns a copy of the referenced value
        //
        trace(("resolve the reference\n"));
        return rvrp->get();
    }

    return data;
}


rpt_value::pointer
rpt_value_struct::lookup(const char *key)
    const
{
    return lookup(nstring(key));
}


rpt_value::pointer
rpt_value_struct::lookup(string_ty *key)
    const
{
    return lookup(nstring(key));
}


rpt_value::pointer
rpt_value_struct::lookup(const nstring &key)
    const
{
    return lookup(rpt_value_string::create(key), false);
}


rpt_value::pointer
rpt_value_struct::keys()
    const
{
    nstring_list k;
    members.keys(k);
    rpt_value_list *p = new rpt_value_list();
    for (size_t j = 0; j < k.size(); ++j)
        p->append(rpt_value_string::create(k[j]));
    return pointer(p);
}


rpt_value::pointer
rpt_value_struct::count()
    const
{
    return rpt_value_integer::create(members.size());
}


const char *
rpt_value_struct::name()
    const
{
    return "struct";
}


bool
rpt_value_struct::is_a_struct()
    const
{
    return true;
}


void
rpt_value_struct::assign(const nstring &key, const rpt_value::pointer &value)
{
    trace(("rpt_value_struct::set(this = %p, key = \"%s\", "
        "value = %p %s)\n", this, key.c_str(), value.get(),
        value->name()));
    if (dynamic_cast<const rpt_value_reference *>(value.get()))
        members.assign(key, value);
    else
        members.assign(key, rpt_value_reference::create(value));
}


// vim: set ts=8 sw=4 et :
