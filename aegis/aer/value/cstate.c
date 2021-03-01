/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994 Peter Miller.
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
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: functions to manipulate cstate values
 */

#include <aer/value/cstate.h>
#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <aer/value/list.h>
#include <aer/value/nul.h>
#include <aer/value/string.h>
#include <aer/value/struct.h>
#include <change.h>
#include <error.h>
#include <mem.h>
#include <project.h>
#include <trace.h>


typedef struct rpt_value_cstate_ty rpt_value_cstate_ty;
struct rpt_value_cstate_ty
{
	RPT_VALUE
	project_ty	*pp;
	long		length;
	long		*list;
};


static void destruct _((rpt_value_ty *));

static void
destruct(vp)
	rpt_value_ty	*vp;
{
	rpt_value_cstate_ty *this;

	trace(("rpt_value_cstate::destruct(vp = %08lX)\n{\n"/*}*/, (long)vp));
	this = (rpt_value_cstate_ty *)vp;
	project_free(this->pp);
	mem_free(this->list);
	trace((/*{*/"}\n"));
}


static rpt_value_ty *lookup _((rpt_value_ty *, rpt_value_ty *, int));

static rpt_value_ty *
lookup(vp, rhs, lval)
	rpt_value_ty	*vp;
	rpt_value_ty	*rhs;
	int		lval;
{
	rpt_value_cstate_ty *this;
	rpt_value_ty	*rhs2;
	rpt_value_ty	*result;
	long		change_number;
	long		j;
	change_ty	*cp;
	cstate		cstate_data;
	string_ty	*name;
	rpt_value_ty	*value;

	/*
	 * extract the change number
	 */
	trace(("rpt_value_cstate::lookup(this = %08lX)\n{\n"/*}*/, (long)vp));
	this = (rpt_value_cstate_ty *)vp;
	rhs2 = rpt_value_integerize(rhs);
	if (rhs2->method->type != rpt_value_type_integer)
	{
		result =
			rpt_value_error
			(
				"integer index required (was given %s)",
				rhs2->method->name
			);
		rpt_value_free(rhs2);
		goto done;
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
		goto done;
	}

	/*
	 * find the change
	 */
	cp = change_alloc(this->pp, change_number);
	change_bind_existing(cp);

	/*
	 * create the result value
	 */
	cstate_data = change_cstate_get(cp);
	result = cstate_type.convert(&cstate_data);
	assert(result);
	assert(result->method->type == rpt_value_type_structure);

	/*
	 * add some extra stuff
	 */
	name = str_from_c("project_name");
	value = rpt_value_string(project_name_get(this->pp));
	rpt_value_struct__set(result, name, value);
	str_free(name);
	rpt_value_free(value);
	name = str_from_c("change_number");
	value = rpt_value_integer(change_number);
	rpt_value_struct__set(result, name, value);
	str_free(name);
	rpt_value_free(value);

	/*
	 * clean up and go home
	 */
	change_free(cp);
	done:
	trace(("return %08lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


static rpt_value_ty *keys _((rpt_value_ty *));

static rpt_value_ty *
keys(vp)
	rpt_value_ty	*vp;
{
	rpt_value_cstate_ty *this;
	rpt_value_ty	*result;
	long		j;

	trace(("rpt_value_cstate::keys(this = %08lX)\n{\n"/*}*/, (long)vp));
	this = (rpt_value_cstate_ty *)vp;
	result = rpt_value_list();
	for (j = 0; j < this->length; ++j)
	{
		rpt_value_ty	*elem;

		elem = rpt_value_integer(this->list[j]);
		rpt_value_list_append(result, elem);
		rpt_value_free(elem);
	}
	trace(("return %08lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


static rpt_value_ty *count _((rpt_value_ty *));

static rpt_value_ty *
count(vp)
	rpt_value_ty	*vp;
{
	rpt_value_cstate_ty *this;
	rpt_value_ty	*result;

	trace(("rpt_value_cstate::count(this = %08lX)\n{\n"/*}*/, (long)vp));
	this = (rpt_value_cstate_ty *)vp;
	result = rpt_value_integer(this->length);
	trace(("return %08lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


static char *type_of _((rpt_value_ty *));

static char *
type_of(vp)
	rpt_value_ty	*vp;
{
	char		*result;

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
};


rpt_value_ty *
rpt_value_cstate(pp, length, list)
	project_ty	*pp;
	long		length;
	long		*list;
{
	rpt_value_cstate_ty *this;
	long		j;

	this = (rpt_value_cstate_ty *)rpt_value_alloc(&method);
	this->pp = project_copy(pp);
	this->length = length;
	this->list = mem_alloc(length * sizeof(long));
	for (j = 0; j < length; ++j)
		this->list[j] = list[j];
	return (rpt_value_ty *)this;
}
