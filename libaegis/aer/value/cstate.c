/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994-1996, 1999, 2001, 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate cstate values
 */

#include <aer/value/cstate.h>
#include <aer/value/error.h>
#include <aer/value/fstate.h>
#include <aer/value/integer.h>
#include <aer/value/list.h>
#include <aer/value/null.h>
#include <aer/value/pconf.h>
#include <aer/value/string.h>
#include <aer/value/struct.h>
#include <change.h>
#include <error.h>
#include <mem.h>
#include <project.h>
#include <sub.h>
#include <trace.h>


typedef struct rpt_value_cstate_ty rpt_value_cstate_ty;
struct rpt_value_cstate_ty
{
    RPT_VALUE
    project_ty	    *pp;
    long	    length;
    long	    *list;
};


static void
destruct(rpt_value_ty *vp)
{
    rpt_value_cstate_ty *this;

    trace(("rpt_value_cstate::destruct(vp = %08lX)\n{\n"/*}*/, (long)vp));
    this = (rpt_value_cstate_ty *)vp;
    project_free(this->pp);
    mem_free(this->list);
    trace((/*{*/"}\n"));
}


static rpt_value_ty *
lookup(rpt_value_ty *vp, rpt_value_ty *rhs, int lval)
{
    rpt_value_cstate_ty *this;
    rpt_value_ty    *rhs2;
    rpt_value_ty    *result;
    long	    change_number;
    long	    j;
    change_ty	    *cp;
    cstate	    cstate_data;
    string_ty	    *name;
    rpt_value_ty    *vp1;
    rpt_value_ty    *vp2;
    rpt_value_ty    *vp3;

    /*
     * extract the change number
     */
    trace(("rpt_value_cstate::lookup(this = %08lX)\n{\n"/*}*/, (long)vp));
    this = (rpt_value_cstate_ty *)vp;
    rhs2 = rpt_value_integerize(rhs);
    if (rhs2->method->type != rpt_value_type_integer)
    {
	sub_context_ty	*scp;
	string_ty	*s;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name", rhs2->method->name);
	rpt_value_free(rhs2);
	s = subst_intl(scp, i18n("integer index required (was given $name)"));
	sub_context_delete(scp);
	result = rpt_value_error((struct rpt_pos_ty *)0, s);
	trace((/*{*/"}\n"));
	return result;
    }
    change_number = rpt_value_integer_query(rhs2);
    rpt_value_free(rhs2);

    /*
     * see if the change exists
     */
    for (j = 0; j < this->length; ++j)
	if (this->list[j] == change_number)
    	    break;
    if (j >= this->length)
    {
	result = rpt_value_nul();
	trace((/*{*/"}\n"));
	return result;
    }

    /*
     * find the change
     */
    cp = change_alloc(this->pp, magic_zero_encode(change_number));
    change_bind_existing(cp);

    /*
     * create the result value
     */
    cstate_data = change_cstate_get(cp);
    result = cstate_type.convert(&cstate_data);
    assert(result);
    assert(result->method->type == rpt_value_type_structure);

    /*
     * The src field is now kept in the fstate file.
     * It will not be read in until referenced.
     */
    assert(!cstate_data->src);
    name = str_from_c("src");
    vp1 = rpt_value_fstate(cp);
    rpt_value_struct__set(result, name, vp1);
    str_free(name);
    rpt_value_free(vp1);

    /*
     * add some extra stuff
     */
    name = str_from_c("project_name");
    vp1 = rpt_value_string(project_name_get(this->pp));
    rpt_value_struct__set(result, name, vp1);
    str_free(name);
    rpt_value_free(vp1);
    name = str_from_c("change_number");
    vp1 = rpt_value_integer(change_number);
    rpt_value_struct__set(result, name, vp1);
    str_free(name);
    rpt_value_free(vp1);

    /*
     * Add a special "config" field, which is a deferred reference
     * to the project config data.
     */
    name = str_from_c("config");
    vp1 = rpt_value_pconf(cp);
    rpt_value_struct__set(result, name, vp1);
    str_free(name);
    rpt_value_free(vp1);

    /*
     * mangle the "change" list
     * into a deferred array of cstate values
     */
    if (cstate_data->branch && cstate_data->branch->change)
    {
	name = str_from_c("branch");
	vp1 = rpt_value_string(name);
	assert(vp1);
	str_free(name);
	vp2 = rpt_value_lookup(result, vp1, 0);
	assert(vp2);
	rpt_value_free(vp1);
	vp3 =
	    rpt_value_cstate
	    (
	       	this->pp,
	       	cstate_data->branch->change->length,
	       	cstate_data->branch->change->list
	    );
	name = str_from_c("change");
	rpt_value_struct__set(vp2, name, vp3);
	rpt_value_free(vp2);
	str_free(name);
	rpt_value_free(vp3);
    }

    /*
     * clean up and go home
     */
    change_free(cp);
    trace(("return %08lX;\n", (long)result));
    trace((/*{*/"}\n"));
    return result;
}


static rpt_value_ty *
keys(rpt_value_ty *vp)
{
    rpt_value_cstate_ty *this;
    rpt_value_ty    *result;
    long	    j;

    trace(("rpt_value_cstate::keys(this = %08lX)\n{\n"/*}*/, (long)vp));
    this = (rpt_value_cstate_ty *)vp;
    result = rpt_value_list();
    for (j = 0; j < this->length; ++j)
    {
	rpt_value_ty	*elem;

	elem = rpt_value_integer(magic_zero_decode(this->list[j]));
	rpt_value_list_append(result, elem);
	rpt_value_free(elem);
    }
    trace(("return %08lX;\n", (long)result));
    trace((/*{*/"}\n"));
    return result;
}


static rpt_value_ty *
count(rpt_value_ty *vp)
{
    rpt_value_cstate_ty *this;
    rpt_value_ty    *result;

    trace(("rpt_value_cstate::count(this = %08lX)\n{\n"/*}*/, (long)vp));
    this = (rpt_value_cstate_ty *)vp;
    result = rpt_value_integer(this->length);
    trace(("return %08lX;\n", (long)result));
    trace((/*{*/"}\n"));
    return result;
}


static char *
type_of(rpt_value_ty *vp)
{
    char	    *result;

    trace(("rpt_value_cstate::type_of(this = %08lX)\n{\n"/*}*/, (long)vp));
    result = "struct";
    trace(("return \"%s\";\n", result));
    trace((/*{*/"}\n"));
    return result;
}


static rpt_value_method_ty method =
{
    sizeof(rpt_value_cstate_ty),
    "cstate",
    rpt_value_type_structure,
    0, /* construct */
    destruct,
    0, /* arithmetic */
    0, /* stringize */
    0, /* booleanize */
    lookup,
    keys,
    count,
    type_of,
    0, /* undefer */
};


rpt_value_ty *
rpt_value_cstate(project_ty *pp, long length, long *list)
{
    rpt_value_cstate_ty *this;
    long		j;

    this = (rpt_value_cstate_ty *)rpt_value_alloc(&method);
    this->pp = project_copy(pp);
    this->length = length;
    this->list = mem_alloc(length * sizeof(long));
    for (j = 0; j < length; ++j)
	this->list[j] = magic_zero_decode(list[j]);
    return (rpt_value_ty *)this;
}
