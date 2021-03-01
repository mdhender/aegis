//
//	aegis - project change supervisor
//	Copyright (C) 1995, 1996, 1999, 2002-2005 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate fstate valuess
//

#include <libaegis/aer/value/fstate.h>
#include <libaegis/change/file.h>
#include <common/error.h>
#include <common/trace.h>


struct rpt_value_fstate_ty
{
    RPT_VALUE
    change_ty       *cp;
    rpt_value_ty    *converted;
};


static void
destruct(rpt_value_ty *vp)
{
    rpt_value_fstate_ty *this_thing;

    trace(("rpt_value_fstate::destruct(vp = %08lX)\n{\n", (long)vp));
    this_thing = (rpt_value_fstate_ty *)vp;
    change_free(this_thing->cp);
    if (this_thing->converted)
	rpt_value_free(this_thing->converted);
    trace(("}\n"));
}


static void
convert(rpt_value_fstate_ty *this_thing)
{
    fstate_ty       *fstate_data;

    assert(!this_thing->converted);
    fstate_data = change_fstate_get(this_thing->cp);
    this_thing->converted = fstate_src_list_type.convert(&fstate_data->src);
}


static rpt_value_ty *
lookup(rpt_value_ty *vp, rpt_value_ty *rhs, int lval)
{
    rpt_value_fstate_ty *this_thing;
    rpt_value_ty    *result;

    trace(("rpt_value_fstate::lookup(this = %08lX)\n{\n", (long)vp));
    this_thing = (rpt_value_fstate_ty *)vp;
    if (!this_thing->converted)
	convert(this_thing);
    result = rpt_value_lookup(this_thing->converted, rhs, lval);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static rpt_value_ty *
keys(rpt_value_ty *vp)
{
    rpt_value_fstate_ty *this_thing;
    rpt_value_ty    *result;

    trace(("rpt_value_fstate::keys(this = %08lX)\n{\n", (long)vp));
    this_thing = (rpt_value_fstate_ty *)vp;
    if (!this_thing->converted)
	convert(this_thing);
    result = rpt_value_keys(this_thing->converted);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static rpt_value_ty *
count(rpt_value_ty *vp)
{
    rpt_value_fstate_ty *this_thing;
    rpt_value_ty    *result;

    trace(("rpt_value_fstate::count(this = %08lX)\n{\n", (long)vp));
    this_thing = (rpt_value_fstate_ty *)vp;
    if (!this_thing->converted)
	    convert(this_thing);
    result = rpt_value_count(this_thing->converted);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static const char *
type_of(rpt_value_ty *vp)
{
    rpt_value_fstate_ty *this_thing;
    const char      *result;

    trace(("rpt_value_fstate::type_of(this = %08lX)\n{\n", (long)vp));
    this_thing = (rpt_value_fstate_ty *)vp;
    if (!this_thing->converted)
	    convert(this_thing);
    result = rpt_value_typeof(this_thing->converted);
    trace(("return \"%s\";\n", result));
    trace(("}\n"));
    return result;
}


static rpt_value_ty *
undefer(rpt_value_ty *vp)
{
    rpt_value_fstate_ty *this_thing;
    rpt_value_ty    *result;

    trace(("rpt_value_fstate::undefer(this = %08lX)\n{\n", (long)vp));
    this_thing = (rpt_value_fstate_ty *)vp;
    if (!this_thing->converted)
	    convert(this_thing);
    result = rpt_value_copy(this_thing->converted);
    trace(("return %08lX\n", (long)result));
    trace(("}\n"));
    return result;
}


static rpt_value_method_ty method =
{
    sizeof(rpt_value_fstate_ty),
    "fstate",
    rpt_value_type_deferred,
    0, // construct
    destruct,
    0, // arithmetic
    0, // stringize
    0, // booleanize
    lookup,
    keys,
    count,
    type_of,
    undefer,
};


rpt_value_ty *
rpt_value_fstate(change_ty *cp)
{
    rpt_value_fstate_ty *this_thing;

    this_thing = (rpt_value_fstate_ty *)rpt_value_alloc(&method);
    this_thing->cp = change_copy(cp);
    this_thing->converted = 0;
    return (rpt_value_ty *)this_thing;
}
