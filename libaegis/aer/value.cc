//
//	aegis - project change supervisor
//	Copyright (C) 1994-1996, 1999, 2002-2008 Peter Miller
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
#include <common/mem.h>
#include <common/trace.h>
#include <libaegis/aer/expr.h>
#include <libaegis/aer/value.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/sub.h>


rpt_value::rpt_value()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


rpt_value::~rpt_value()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


rpt_value::pointer
rpt_value::lookup(const rpt_value::pointer &rhs, bool)
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    sub_context_ty sc;
    sc.var_set_charstar("Name1", name());
    sc.var_set_charstar("Name2", rhs->name());
    nstring s(sc.subst_intl(i18n("illegal lookup ($name1[$name2])")));
    return rpt_value_error::create(s);
}


rpt_value::pointer
rpt_value::keys()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    sub_context_ty sc;
    sc.var_set_charstar("Name", name());
    nstring s(sc.subst_intl(i18n("illegal keys request ($name)")));
    return rpt_value_error::create(s);
}


rpt_value::pointer
rpt_value::count()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    sub_context_ty sc;
    sc.var_set_charstar("Name", name());
    nstring s(sc.subst_intl(i18n("illegal count request ($name)")));
    return rpt_value_error::create(s);
}


const char *
rpt_value::type_of()
    const
{
    return name();
}


rpt_value::pointer
rpt_value::integerize(const rpt_value::pointer &vp)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    rpt_value::pointer result = vp->integerize_or_null();
    return (result ? result : vp);
}


rpt_value::pointer
rpt_value::integerize_or_null()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer();
}


rpt_value::pointer
rpt_value::undefer(const rpt_value::pointer &vp)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    rpt_value::pointer result = vp->undefer_or_null();
    return (result ? result : vp);
}


rpt_value::pointer
rpt_value::undefer_or_null()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer();
}


rpt_value::pointer
rpt_value::arithmetic(const rpt_value::pointer &vp)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    rpt_value::pointer result = vp->arithmetic_or_null();
    return (result ? result : vp);
}


rpt_value::pointer
rpt_value::arithmetic_or_null()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer();
}


rpt_value::pointer
rpt_value::realize(const rpt_value::pointer &vp)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    rpt_value::pointer result = vp->realize_or_null();
    return (result ? result : vp);
}


rpt_value::pointer
rpt_value::realize_or_null()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer();
}


rpt_value::pointer
rpt_value::stringize(const rpt_value::pointer &vp)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    trace(("vp is a %s\n", vp->name()));
    rpt_value::pointer result = vp->stringize_or_null();
    trace(("result = %p\n", result.get()));
    return (result ? result : vp);
}


rpt_value::pointer
rpt_value::stringize_or_null()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer();
}


rpt_value::pointer
rpt_value::booleanize(const rpt_value::pointer &vp)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    rpt_value::pointer result = vp->booleanize_or_null();
    return (result ? result : vp);
}


rpt_value::pointer
rpt_value::booleanize_or_null()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer();
}


bool
rpt_value::is_an_error()
    const
{
    return false;
}


bool
rpt_value::is_a_struct()
    const
{
    return false;
}
