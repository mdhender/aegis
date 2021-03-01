//
//	aegis - project change supervisor
//	Copyright (C) 1994, 1996, 2003-2005 Peter Miller;
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
// MANIFEST: functions to manipulate reference values
//

#include <libaegis/aer/value/ref.h>
#include <common/error.h>
#include <common/trace.h>


struct rpt_value_reference_ty
{
    RPT_VALUE
    rpt_value_ty    *value;
};


static void
destruct(rpt_value_ty *vp)
{
    rpt_value_reference_ty *this_thing;

    trace(("reference::destruct(this = %08lX)\n{\n", (long)vp));
    this_thing = (rpt_value_reference_ty *)vp;
    assert(this_thing->method->type == rpt_value_type_reference);
    assert(this_thing->value->reference_count >= 1);
    rpt_value_free(this_thing->value);
    trace(("}\n"));
}


static rpt_value_ty *
arithmetic(rpt_value_ty *vp)
{
    rpt_value_reference_ty *this_thing;
    rpt_value_ty    *result;

    trace(("reference::destruct(this = %08lX)\n{\n", (long)vp));
    this_thing = (rpt_value_reference_ty *)vp;
    assert(this_thing->method->type == rpt_value_type_reference);
    assert(this_thing->reference_count >= 1);
    assert(this_thing->value->reference_count >= 1);
    assert(this_thing->value->method->type != rpt_value_type_reference);
    result = rpt_value_arithmetic(this_thing->value);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static rpt_value_ty *
stringize(rpt_value_ty *vp)
{
    rpt_value_reference_ty *this_thing;
    rpt_value_ty    *result;

    trace(("reference::stringize(this = %08lX)\n{\n", (long)vp));
    this_thing = (rpt_value_reference_ty *)vp;
    assert(this_thing->method->type == rpt_value_type_reference);
    assert(this_thing->reference_count >= 1);
    assert(this_thing->value->reference_count >= 1);
    assert(this_thing->value->method->type != rpt_value_type_reference);
    result = rpt_value_stringize(this_thing->value);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static rpt_value_ty *
booleanize(rpt_value_ty *vp)
{
    rpt_value_reference_ty *this_thing;
    rpt_value_ty    *result;

    trace(("reference::booleanize(this = %08lX)\n{\n", (long)vp));
    this_thing = (rpt_value_reference_ty *)vp;
    assert(this_thing->method->type == rpt_value_type_reference);
    assert(this_thing->reference_count >= 1);
    assert(this_thing->value->reference_count >= 1);
    assert(this_thing->value->method->type != rpt_value_type_reference);
    result = rpt_value_booleanize(this_thing->value);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static rpt_value_ty *
lookup(rpt_value_ty *vp, rpt_value_ty *rhs, int lvalue)
{
    rpt_value_reference_ty *this_thing;
    rpt_value_ty    *result;

    trace(("reference::lookup(this = %08lX)\n{\n", (long)vp));
    this_thing = (rpt_value_reference_ty *)vp;
    assert(this_thing->method->type == rpt_value_type_reference);
    assert(this_thing->reference_count >= 1);
    assert(this_thing->value->reference_count >= 1);
    assert(this_thing->value->method->type != rpt_value_type_reference);
    result = rpt_value_lookup(this_thing->value, rhs, lvalue);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static rpt_value_ty *
keys(rpt_value_ty *vp)
{
    rpt_value_reference_ty *this_thing;
    rpt_value_ty    *result;

    trace(("reference::keys(this = %08lX)\n{\n", (long)vp));
    this_thing = (rpt_value_reference_ty *)vp;
    assert(this_thing->method->type == rpt_value_type_reference);
    assert(this_thing->reference_count >= 1);
    assert(this_thing->value->reference_count >= 1);
    assert(this_thing->value->method->type != rpt_value_type_reference);
    result = rpt_value_keys(this_thing->value);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static rpt_value_ty *
count(rpt_value_ty *vp)
{
    rpt_value_reference_ty *this_thing;
    rpt_value_ty    *result;

    trace(("reference::count(this = %08lX)\n{\n", (long)vp));
    this_thing = (rpt_value_reference_ty *)vp;
    assert(this_thing->method->type == rpt_value_type_reference);
    assert(this_thing->reference_count >= 1);
    assert(this_thing->value->reference_count >= 1);
    assert(this_thing->value->method->type != rpt_value_type_reference);
    result = rpt_value_count(this_thing->value);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static const char *
type_of(rpt_value_ty *that)
{
    rpt_value_reference_ty *this_thing;

    this_thing = (rpt_value_reference_ty *)that;
    assert(this_thing->reference_count >= 1);
    assert(this_thing->method->type == rpt_value_type_reference);
    assert(this_thing->value->reference_count >= 1);
    assert(this_thing->value->method->type != rpt_value_type_reference);
    return rpt_value_typeof(this_thing->value);
}


static rpt_value_ty *
undefer(rpt_value_ty *that)
{
    rpt_value_reference_ty *this_thing;

    this_thing = (rpt_value_reference_ty *)that;
    assert(this_thing->reference_count >= 1);
    assert(this_thing->method->type == rpt_value_type_reference);
    assert(this_thing->value->reference_count >= 1);
    assert(this_thing->value->method->type != rpt_value_type_reference);
    return rpt_value_copy(this_thing->value);
}


static rpt_value_method_ty method =
{
    sizeof(rpt_value_reference_ty),
    "reference",
    rpt_value_type_reference,
    0, // construct
    destruct,
    arithmetic,
    stringize,
    booleanize,
    lookup,
    keys,
    count,
    type_of,
    undefer,
};


rpt_value_ty *
rpt_value_reference(rpt_value_ty *deeper)
{
    rpt_value_reference_ty *this_thing;

    trace(("rpt_value_reference(deeper = %08lX)\n{\n", (long)deeper));
    assert(deeper->reference_count >= 1);
    assert(deeper->method->type != rpt_value_type_reference);
    this_thing = (rpt_value_reference_ty *)rpt_value_alloc(&method);
    this_thing->value = rpt_value_copy(deeper);
    trace(("return %08lX;\n", (long)this_thing));
    trace(("}\n"));
    return (rpt_value_ty *)this_thing;
}


rpt_value_ty *
rpt_value_reference_get(rpt_value_ty *vp)
{
    rpt_value_reference_ty *this_thing;
    rpt_value_ty    *result;

    trace(("reference::get(this = %08lX)\n{\n", (long)vp));
    this_thing = (rpt_value_reference_ty *)vp;
    assert(this_thing->method == &method);
    assert(this_thing->reference_count >= 1);
    assert(this_thing->value->reference_count >= 1);
    assert(this_thing->value->method->type != rpt_value_type_reference);
    result = rpt_value_copy(this_thing->value);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


void
rpt_value_reference_set(rpt_value_ty *vp, rpt_value_ty *deeper)
{
    rpt_value_reference_ty *this_thing;
    rpt_value_ty    *old;

    trace(("rpt_value_reference_set(vp = %08lX, deeper = %08lX)\n{\n",
	(long)vp, (long)deeper));
    assert(deeper->method->type != rpt_value_type_reference);
    assert(deeper->reference_count >= 1);
    this_thing = (rpt_value_reference_ty *)vp;
    assert(this_thing->method == &method);
    assert(this_thing->reference_count >= 1);
    assert(this_thing->value->reference_count >= 1);
    assert(this_thing->value->method->type != rpt_value_type_reference);
    old = this_thing->value;
    this_thing->value = rpt_value_copy(deeper);
    assert(deeper->reference_count >= 2);
    rpt_value_free(old);
    trace(("}\n"));
}
