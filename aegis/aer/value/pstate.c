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
 * MANIFEST: functions to manipulate pstate values
 */

#include <ac/string.h>

#include <aer/value/cstate.h>
#include <aer/value/error.h>
#include <aer/value/pstate.h>
#include <aer/value/string.h>
#include <aer/value/struct.h>
#include <error.h>
#include <gonzo.h>
#include <os.h>
#include <project.h>
#include <trace.h>


typedef struct rpt_value_pstate_ty rpt_value_pstate_ty;
struct rpt_value_pstate_ty
{
	RPT_VALUE
	string_ty	*name;
	rpt_value_ty	*value;
};


static void destruct _((rpt_value_ty *));

static void
destruct(vp)
	rpt_value_ty	*vp;
{
	rpt_value_pstate_ty *this;

	trace(("rpt_value_pstate::destruct(vp = %08lX)\n{\n"/*}*/, (long)vp));
	this = (rpt_value_pstate_ty *)vp;
	if (this->value)
		rpt_value_free(this->value);
	str_free(this->name);
	trace((/*{*/"}\n"));
}


static void grab _((rpt_value_pstate_ty *));

static void
grab(this)
	rpt_value_pstate_ty *this;
{
	project_ty	*pp;
	int		err;
	pstate		pstate_data;
	string_ty	*name;
	rpt_value_ty	*value;

	/*
	 * establish that it is a valid project name
	 */
	trace(("rpt_value_pstate::grab(this = %08lX)\n{\n"/*}*/, (long)this));
	assert(!this->value);
	assert(this->name);
	if (!gonzo_project_home_path_from_name(this->name))
	{
		this->value =
			rpt_value_error("project \"%S\" unknown", this->name);
		goto done;
	}

	/*
	 * make sure the project state is readable
	 */
	pp = project_alloc(this->name);
	project_bind_existing(pp);
	os_become_orig();
	err = os_readable(project_pstate_path_get(pp));
	os_become_undo();
	if (err)
	{
		this->value =
			rpt_value_error
			(
				"%S: %s",
				project_pstate_path_get(pp),
				strerror(err)
			);
		goto done;
	}

	/*
	 * create the result value
	 */
	pstate_data = project_pstate_get(pp);
	this->value = pstate_type.convert(&pstate_data);
	assert(this->value);
	assert(this->value->method->type == rpt_value_type_structure);

	/*
	 * add some extra stuff
	 */
	name = str_from_c("name");
	value = rpt_value_string(this->name);
	rpt_value_struct__set(this->value, name, value);
	str_free(name);
	rpt_value_free(value);
	name = str_from_c("directory");
	value = rpt_value_string(project_pstate_path_get(pp));
	rpt_value_struct__set(this->value, name, value);
	str_free(name);
	rpt_value_free(value);

	/*
	 * mangle the "change" list
	 * into a deferred array of cstate values
	 */
	value =
		rpt_value_cstate
		(
			pp,
			pstate_data->change->length,
			pstate_data->change->list
		);
	name = str_from_c("change");
	rpt_value_struct__set(this->value, name, value);
	str_free(name);
	rpt_value_free(value);

	/*
	 * clean up and go home
	 */
	project_free(pp);
	done:
	trace(("this->value = %08lX;\n", (long)this->value));
	trace((/*{*/"}\n"));
}


static rpt_value_ty *lookup _((rpt_value_ty *, rpt_value_ty *, int));

static rpt_value_ty *
lookup(vp, rhs, lval)
	rpt_value_ty	*vp;
	rpt_value_ty	*rhs;
	int		lval;
{
	rpt_value_pstate_ty *this;
	rpt_value_ty	*result;

	trace(("rpt_value_pstate::lookup(this = %08lX)\n{\n"/*}*/, (long)vp));
	this = (rpt_value_pstate_ty *)vp;
	if (!this->value)
		grab(this);
	assert(this->value);
	result = rpt_value_lookup(this->value, rhs, lval);
	trace(("return %08lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


static rpt_value_ty *keys _((rpt_value_ty *));

static rpt_value_ty *
keys(vp)
	rpt_value_ty	*vp;
{
	rpt_value_pstate_ty *this;
	rpt_value_ty	*result;

	trace(("rpt_value_pstate::keys(this = %08lX)\n{\n"/*}*/, (long)vp));
	this = (rpt_value_pstate_ty *)vp;
	if (!this->value)
		grab(this);
	assert(this->value);
	result = rpt_value_keys(this->value);
	trace(("return %08lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


static rpt_value_ty *count _((rpt_value_ty *));

static rpt_value_ty *
count(vp)
	rpt_value_ty	*vp;
{
	rpt_value_pstate_ty *this;
	rpt_value_ty	*result;

	trace(("rpt_value_pstate::count(this = %08lX)\n{\n"/*}*/, (long)vp));
	this = (rpt_value_pstate_ty *)vp;
	if (!this->value)
		grab(this);
	assert(this->value);
	result = rpt_value_count(this->value);
	trace(("return %08lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


static char *type_of _((rpt_value_ty *));

static char *
type_of(vp)
	rpt_value_ty	*vp;
{
	rpt_value_pstate_ty *this;
	char		*result;

	trace(("rpt_value_pstate::type_of(this = %08lX)\n{\n"/*}*/, (long)vp));
	this = (rpt_value_pstate_ty *)vp;
	if (!this->value)
		grab(this);
	assert(this->value);
	result = rpt_value_typeof(this->value);
	trace(("return \"%s\";\n", result));
	trace((/*{*/"}\n"));
	return result;
}


static rpt_value_method_ty method =
{
	sizeof(rpt_value_pstate_ty),
	"pstate",
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
rpt_value_pstate(name)
	string_ty	*name;
{
	rpt_value_pstate_ty *this;

	this = (rpt_value_pstate_ty *)rpt_value_alloc(&method);
	this->name = str_copy(name);
	this->value = 0;
	return (rpt_value_ty *)this;
}
