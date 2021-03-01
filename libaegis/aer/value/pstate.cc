//
//	aegis - project change supervisor
//	Copyright (C) 1994-1997, 1999, 2002-2006 Peter Miller;
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
// MANIFEST: functions to manipulate pstate values
//

#include <common/ac/string.h>

#include <libaegis/aer/pos.h>
#include <libaegis/aer/value/cstate.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/fstate.h>
#include <libaegis/aer/value/pstate.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/aer/value/struct.h>
#include <libaegis/change.h>
#include <common/error.h>
#include <libaegis/gonzo.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>
#include <common/trace.h>


struct rpt_value_pstate_ty
{
    RPT_VALUE
    string_ty	    *name;
    rpt_value_ty    *value;
};


static void
destruct(rpt_value_ty *vp)
{
    rpt_value_pstate_ty *this_thing;

    trace(("rpt_value_pstate::destruct(vp = %08lX)\n{\n", (long)vp));
    this_thing = (rpt_value_pstate_ty *)vp;
    if (this_thing->value)
	rpt_value_free(this_thing->value);
    str_free(this_thing->name);
    trace(("}\n"));
}


static void
grab(rpt_value_pstate_ty *this_thing)
{
    project_ty	    *pp;
    change_ty	    *cp;
    int		    err;
    cstate_ty       *cstate_data;
    string_ty	    *name;
    rpt_value_ty    *vp1;
    rpt_value_ty    *vp2;
    rpt_value_ty    *vp3;

    //
    // construct the project, assuming it exists
    //
    trace(("rpt_value_pstate::grab(this = %08lX)\n{\n", (long)this_thing));
    assert(!this_thing->value);
    assert(this_thing->name);
    pp = project_alloc(this_thing->name);
    pp->bind_existing();

    //
    // make sure the project state is readable
    //
    err = project_is_readable(pp);
    if (err)
    {
	sub_context_ty	*scp;
	string_ty	*s;

	scp = sub_context_new();
	sub_errno_setx(scp, err);
	sub_var_set_string(scp, "File_Name", pp->pstate_path_get());
	s = subst_intl(scp, "stat $filename: $errno");
	sub_context_delete(scp);
	this_thing->value = rpt_value_error((rpt_pos_ty *)0, s);
	str_free(s);
	goto done;
    }

    //
    // create the result value
    //
    cp = pp->change_get();
    cstate_data = change_cstate_get(cp);
    this_thing->value = cstate_type.convert(&cstate_data);
    assert(this_thing->value);
    assert(this_thing->value->method->type == rpt_value_type_structure);

    //
    // The src field is now kept in the fstate file.
    // It will not be read in until referenced.
    //
    assert(!cstate_data->src);
    name = str_from_c("src");
    vp1 = rpt_value_fstate(cp);
    rpt_value_struct__set(this_thing->value, name, vp1);
    str_free(name);
    rpt_value_free(vp1);

    //
    // add some extra stuff
    //
    name = str_from_c("name");
    vp1 = rpt_value_string(project_name_get(pp));
    rpt_value_struct__set(this_thing->value, name, vp1);
    str_free(name);
    rpt_value_free(vp1);
    if (cstate_data->state == cstate_state_being_developed)
    {
	name = str_from_c("directory");
	vp1 = rpt_value_string(change_development_directory_get(cp, 0));
	rpt_value_struct__set(this_thing->value, name, vp1);
	str_free(name);
	rpt_value_free(vp1);
    }

    if (!pp->is_a_trunk())
    {
	name = str_from_c("parent_name");
	vp1 = rpt_value_string(project_name_get(pp->parent_get()));
	rpt_value_struct__set(this_thing->value, name, vp1);
	str_free(name);
	rpt_value_free(vp1);
    }

    //
    // mangle the "change" list
    // into a deferred array of cstate values
    //
    if (cstate_data->branch && cstate_data->branch->change)
    {
	name = str_from_c("branch");
	vp1 = rpt_value_string(name);
	assert(vp1);
	str_free(name);
	vp2 = rpt_value_lookup(this_thing->value, vp1, 0);
	assert(vp2);
	rpt_value_free(vp1);
	vp3 =
	    rpt_value_cstate
	    (
		pp,
		cstate_data->branch->change->length,
		cstate_data->branch->change->list
	    );
	name = str_from_c("change");
	rpt_value_struct__set(vp2, name, vp3);
	rpt_value_free(vp2);
	str_free(name);
	rpt_value_free(vp3);
    }

    //
    // clean up and go home
    //
    done:
    project_free(pp);
    trace(("this_thing->value = %08lX;\n", (long)this_thing->value));
    trace(("}\n"));
}


static rpt_value_ty *
lookup(rpt_value_ty *vp, rpt_value_ty *rhs, int lval)
{
    rpt_value_pstate_ty *this_thing;
    rpt_value_ty    *result;

    trace(("rpt_value_pstate::lookup(this = %08lX)\n{\n", (long)vp));
    this_thing = (rpt_value_pstate_ty *)vp;
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
    rpt_value_pstate_ty *this_thing;
    rpt_value_ty    *result;

    trace(("rpt_value_pstate::keys(this = %08lX)\n{\n", (long)vp));
    this_thing = (rpt_value_pstate_ty *)vp;
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
    rpt_value_pstate_ty *this_thing;
    rpt_value_ty    *result;

    trace(("rpt_value_pstate::count(this = %08lX)\n{\n", (long)vp));
    this_thing = (rpt_value_pstate_ty *)vp;
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
    rpt_value_pstate_ty *this_thing;
    const char      *result;

    trace(("rpt_value_pstate::type_of(this = %08lX)\n{\n", (long)vp));
    this_thing = (rpt_value_pstate_ty *)vp;
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
    rpt_value_pstate_ty *this_thing;
    rpt_value_ty    *result;

    trace(("rpt_value_pstate::undefer(this = %08lX)\n{\n", (long)vp));
    this_thing = (rpt_value_pstate_ty *)vp;
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
    sizeof(rpt_value_pstate_ty),
    "pstate",
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
rpt_value_pstate(string_ty *name)
{
    rpt_value_pstate_ty *this_thing;

    this_thing = (rpt_value_pstate_ty *)rpt_value_alloc(&method);
    this_thing->name = str_copy(name);
    this_thing->value = 0;
    return (rpt_value_ty *)this_thing;
}
