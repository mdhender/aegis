//
//	aegis - project change supervisor
//	Copyright (C) 1994-1996, 1999, 2002-2005 Peter Miller;
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
// MANIFEST: functions to manipulate struct values
//

#include <libaegis/aer/pos.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/list.h>
#include <libaegis/aer/value/null.h>
#include <libaegis/aer/value/ref.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/aer/value/struct.h>
#include <common/error.h>
#include <libaegis/sub.h>
#include <common/symtab.h>
#include <common/trace.h>


struct rpt_value_struct_ty
{
    RPT_VALUE
    symtab_ty       *value;
};


static void
reap(void *p)
{
    rpt_value_ty    *vp;

    trace(("reap(%08lX);\n", (long)p));
    vp = (rpt_value_ty *)p;
    rpt_value_free(vp);
}


#ifdef DEBUG

static void
func(const symtab_ty *stp, string_ty *key, void *data, void *arg)
{
    trace(("key=\"%s\", data=%08lX\n", key->str_text, (long)data));
}

#endif


static void
destruct(rpt_value_ty *vp)
{
    rpt_value_struct_ty *this_thing;

    trace(("value_struct::destruct(this_thing = %08lX)\n{\n", (long)vp));
    this_thing = (rpt_value_struct_ty *)vp;
    assert(this_thing->method->type == rpt_value_type_structure);
#ifdef DEBUG
    this_thing->value->walk(func, (void *)0);
#endif
    delete this_thing->value;
    this_thing->value = 0;
    trace(("}\n"));
}


static rpt_value_ty *
lookup(rpt_value_ty *vp, rpt_value_ty *rhs, int lvalue)
{
    rpt_value_struct_ty *this_thing;
    rpt_value_ty    *rhs2;
    rpt_value_ty    *data;
    rpt_value_ty    *result = 0;

    trace(("value_struct::lookup(lhs = %08lX, rhs = %08lX, lvalue = %d)\n{\n",
	(long)vp, (long)rhs, lvalue));
    this_thing = (rpt_value_struct_ty *)vp;
    assert(this_thing->reference_count >= 1);
    assert(this_thing->method->type == rpt_value_type_structure);
    rhs2 = rpt_value_stringize(rhs);
    if (rhs2->method->type != rpt_value_type_string)
    {
	sub_context_ty	*scp;
	string_ty	*s;

	scp = sub_context_new();
	rpt_value_free(rhs2);
	trace(("}\n"));
	sub_var_set_charstar(scp, "Name1", vp->method->name);
	sub_var_set_charstar(scp, "Name2", rhs->method->name);
	s = subst_intl(scp, i18n("illegal lookup ($name1[$name2])"));
	sub_context_delete(scp);
	result = (rpt_value_ty *)rpt_value_error((rpt_pos_ty *)0, s);
	str_free(s);
	return result;
    }
    trace(("find the datum\n"));
    data =
	(rpt_value_ty *)this_thing->value->query(rpt_value_string_query(rhs2));
    trace(("data = %08lX;\n", (long)data));
    if (!data)
    {
	result = rpt_value_nul();
	if (lvalue)
	{
	    trace(("create new element\n"));
	    data = rpt_value_reference(result);
	    // reference takes a copy
	    rpt_value_free(result);
	    this_thing->value->assign(rpt_value_string_query(rhs2), data);
	    result = rpt_value_copy(data);
	}
	else
	{
	    //
	    // Note: a nul value will always have ref count
	    // >= 2 (see the assert below) because there is
	    // only ever one allocated.  It is OK for the
	    // nul returned from this function to be
	    // rpt_value_free()ed later, because it is not
	    // anywhere attached.
	    //
	    trace(("element not found, and none created\n"));
	}
    }
    else
    {
	if (data->method->type == rpt_value_type_reference && !lvalue)
	{
	    //
	    // this returns a copy of the referenced value
	    //
	    trace(("resolve the reference\n"));
	    result = rpt_value_reference_get(data);
	}
	else
	{
	    trace(("copy the data\n"));
	    result = rpt_value_copy(data);
	}
    }
    assert(rhs2->reference_count >= 1);
    rpt_value_free(rhs2);
    assert(result->reference_count >= 2);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static void
keys_callback(const symtab_ty *stp, string_ty *key, void *data, void *arg)
{
    rpt_value_ty    *vlp;
    rpt_value_ty    *s;

    vlp = (rpt_value_ty *)arg;
    s = rpt_value_string(key);
    rpt_value_list_append(vlp, s);
    rpt_value_free(s);
}


static rpt_value_ty *
keys(rpt_value_ty *vp)
{
    rpt_value_struct_ty *this_thing;
    rpt_value_ty    *result;

    this_thing = (rpt_value_struct_ty *)vp;
    assert(this_thing->method->type == rpt_value_type_structure);
    result = rpt_value_list();
    this_thing->value->walk(keys_callback, result);
    return result;
}


static rpt_value_ty *
count(rpt_value_ty *vp)
{
    rpt_value_struct_ty *this_thing;

    this_thing = (rpt_value_struct_ty *)vp;
    assert(this_thing->method->type == rpt_value_type_structure);
    return rpt_value_integer(this_thing->value->size());
}


static rpt_value_method_ty method =
{
    sizeof(rpt_value_struct_ty),
    "struct",
    rpt_value_type_structure,
    0, // construct
    destruct,
    0, // arithmetic
    0, // stringize
    0, // booleanize
    lookup,
    keys,
    count,
    0, // type_of
    0, // undefer
};


rpt_value_ty *
rpt_value_struct(symtab_ty *stp)
{
    rpt_value_struct_ty *this_thing;

    this_thing = (rpt_value_struct_ty *)rpt_value_alloc(&method);
    if (!stp)
	stp = new symtab_ty(5);
    stp->set_reap(reap);
    this_thing->value = stp;
    return (rpt_value_ty *)this_thing;
}


symtab_ty *
rpt_value_struct_query(rpt_value_ty *vp)
{
    rpt_value_struct_ty *this_thing;

    this_thing = (rpt_value_struct_ty *)vp;
    assert(this_thing->method == &method);
    return this_thing->value;
}


rpt_value_ty *
rpt_value_struct_lookup(rpt_value_ty *vp, string_ty *name)
{
    rpt_value_struct_ty *this_thing;

    this_thing = (rpt_value_struct_ty *)vp;
    assert(this_thing->method == &method);
    return (rpt_value_ty *)this_thing->value->query(name);
}


void
rpt_value_struct__set(rpt_value_ty *vp, string_ty *name, rpt_value_ty *value)
{
    rpt_value_struct_ty *this_thing;

    trace(("rpt_value_struct__set(this_thing = %08lX, name = \"%s\", "
	"value = %08lX)\n{\n", (long)vp, name->str_text, (long)value));
    this_thing = (rpt_value_struct_ty *)vp;
    assert(this_thing->method == &method);
    if (value->method->type == rpt_value_type_reference)
	value = rpt_value_copy(value);
    else
	value = rpt_value_reference(value);
    this_thing->value->assign(name, value);
    trace(("}\n"));
}
