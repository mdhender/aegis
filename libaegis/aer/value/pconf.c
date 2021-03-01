/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001-2003 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate pconfs
 */

#include <aer/value/pconf.h>
#include <change.h>
#include <error.h> /* for assert */
#include <pconf.h>
#include <trace.h>


typedef struct rpt_value_pconf_ty rpt_value_pconf_ty;
struct rpt_value_pconf_ty
{
    RPT_VALUE
    change_ty       *cp;
    rpt_value_ty    *value;
};


static void
destruct(rpt_value_ty *vp)
{
    rpt_value_pconf_ty *this_thing;

    trace(("rpt_value_pconf::destruct(vp = %08lX)\n{\n", (long)vp));
    this_thing = (rpt_value_pconf_ty *)vp;
    if (this_thing->value)
	rpt_value_free(this_thing->value);
    change_free(this_thing->cp);
    trace(("}\n"));
}


static void
grab(rpt_value_pconf_ty *this_thing)
{
    pconf           pconf_data;

    /*
     * construct the project config file's value, assuming it exists
     */
    trace(("rpt_value_pconf::grab(this = %08lX)\n{\n", (long)this_thing));
    assert(!this_thing->value);
    assert(this_thing->cp);
    pconf_data = change_pconf_get(this_thing->cp, 0);

    /*
     * create the result value
     */
    this_thing->value = pconf_type.convert(&pconf_data);
    assert(this_thing->value);
    assert(this_thing->value->method->type == rpt_value_type_structure);

    /*
     * clean up and go home
     */
    trace(("this_thing->value = %08lX;\n", (long)this_thing->value));
    trace(("}\n"));
}


static rpt_value_ty *
lookup(rpt_value_ty *vp, rpt_value_ty *rhs, int lval)
{
    rpt_value_pconf_ty *this_thing;
    rpt_value_ty    *result;

    trace(("rpt_value_pconf::lookup(this = %08lX)\n{\n", (long)vp));
    this_thing = (rpt_value_pconf_ty *)vp;
    if (!this_thing->value)
	grab(this_thing);
    assert(this_thing->value);
    if (this_thing->value->method->type == rpt_value_type_error)
	result = rpt_value_copy(this_thing->value);
    else
	result = rpt_value_lookup(this_thing->value, rhs, lval);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static rpt_value_ty *
keys(rpt_value_ty *vp)
{
    rpt_value_pconf_ty *this_thing;
    rpt_value_ty    *result;

    trace(("rpt_value_pconf::keys(this = %08lX)\n{\n", (long)vp));
    this_thing = (rpt_value_pconf_ty *)vp;
    if (!this_thing->value)
	grab(this_thing);
    assert(this_thing->value);
    if (this_thing->value->method->type == rpt_value_type_error)
	result = rpt_value_copy(this_thing->value);
    else
	result = rpt_value_keys(this_thing->value);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static rpt_value_ty *
count(rpt_value_ty *vp)
{
    rpt_value_pconf_ty *this_thing;
    rpt_value_ty    *result;

    trace(("rpt_value_pconf::count(this = %08lX)\n{\n", (long)vp));
    this_thing = (rpt_value_pconf_ty *)vp;
    if (!this_thing->value)
	grab(this_thing);
    assert(this_thing->value);
    if (this_thing->value->method->type == rpt_value_type_error)
	result = rpt_value_copy(this_thing->value);
    else
	result = rpt_value_count(this_thing->value);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static const char *
type_of(rpt_value_ty *vp)
{
    rpt_value_pconf_ty *this_thing;
    const char      *result;

    trace(("rpt_value_pconf::type_of(this = %08lX)\n{\n", (long)vp));
    this_thing = (rpt_value_pconf_ty *)vp;
    if (!this_thing->value)
	grab(this_thing);
    assert(this_thing->value);
    result = rpt_value_typeof(this_thing->value);
    trace(("return \"%s\";\n", result));
    trace(("}\n"));
    return result;
}


static rpt_value_ty *
undefer(rpt_value_ty *vp)
{
    rpt_value_pconf_ty *this_thing;
    rpt_value_ty	*result;

    trace(("rpt_value_pconf::undefer(this = %08lX)\n{\n", (long)vp));
    this_thing = (rpt_value_pconf_ty *)vp;
    if (!this_thing->value)
	grab(this_thing);
    assert(this_thing->value);
    result = rpt_value_copy(this_thing->value);
    trace(("return %08lX\n", (long)result));
    trace(("}\n"));
    return result;
}


static rpt_value_method_ty method =
{
    sizeof(rpt_value_pconf_ty),
    "pconf",
    rpt_value_type_deferred,
    0, /* construct */
    destruct,
    0, /* arithmetic */
    0, /* stringize */
    0, /* booleanize */
    lookup,
    keys,
    count,
    type_of,
    undefer,
};


rpt_value_ty *
rpt_value_pconf(change_ty *cp)
{
    rpt_value_pconf_ty *this_thing;

    this_thing = (rpt_value_pconf_ty *)rpt_value_alloc(&method);
    this_thing->cp = change_copy(cp);
    this_thing->value = 0;
    return (rpt_value_ty *)this_thing;
}
