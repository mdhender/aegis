/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1995, 1996, 1997, 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate pstate values
 */

#include <ac/string.h>

#include <aer/value/cstate.h>
#include <aer/value/error.h>
#include <aer/value/fstate.h>
#include <aer/value/pstate.h>
#include <aer/value/string.h>
#include <aer/value/struct.h>
#include <change.h>
#include <error.h>
#include <gonzo.h>
#include <os.h>
#include <project.h>
#include <sub.h>
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
	change_ty	*cp;
	int		err;
	cstate		cstate_data;
	string_ty	*name;
	rpt_value_ty	*vp1;
	rpt_value_ty	*vp2;
	rpt_value_ty	*vp3;

	/*
	 * construct the project, assuming it exists
	 */
	trace(("rpt_value_pstate::grab(this = %08lX)\n{\n"/*}*/, (long)this));
	assert(!this->value);
	assert(this->name);
	pp = project_alloc(this->name);
	project_bind_existing(pp);

	/*
	 * make sure the project state is readable
	 */
	err = project_is_readable(pp);
	if (err)
	{
		sub_context_ty	*scp;
		string_ty	*s;

		scp = sub_context_new();
		sub_errno_setx(scp, err);
		sub_var_set_string(scp, "File_Name", project_pstate_path_get(pp));
		s = subst_intl(scp, "stat $filename: $errno");
		sub_context_delete(scp);
		this->value = rpt_value_error((void *)0, s);
		str_free(s);
		goto done;
	}

	/*
	 * create the result value
	 */
	cp = project_change_get(pp);
	cstate_data = change_cstate_get(cp);
	this->value = cstate_type.convert(&cstate_data);
	assert(this->value);
	assert(this->value->method->type == rpt_value_type_structure);

	/*
	 * The src field is now kept in the fstate file.
	 * It will not be read in until referenced.
	 */
	assert(!cstate_data->src);
	name = str_from_c("src");
	vp1 = rpt_value_fstate(cp);
	rpt_value_struct__set(this->value, name, vp1);
	str_free(name);
	rpt_value_free(vp1);

	/*
	 * add some extra stuff
	 */
	name = str_from_c("name");
	vp1 = rpt_value_string(this->name);
	rpt_value_struct__set(this->value, name, vp1);
	str_free(name);
	rpt_value_free(vp1);
	if (cstate_data->state == cstate_state_being_developed)
	{
		name = str_from_c("directory");
		vp1 = rpt_value_string(change_development_directory_get(cp, 0));
		rpt_value_struct__set(this->value, name, vp1);
		str_free(name);
		rpt_value_free(vp1);
	}

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
		vp2 = rpt_value_lookup(this->value, vp1, 0);
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

	/*
	 * clean up and go home
	 */
	done:
	project_free(pp);
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
	if (this->value->method->type == rpt_value_type_error)
		result = rpt_value_copy(this->value);
	else
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
	if (this->value->method->type == rpt_value_type_error)
		result = rpt_value_copy(this->value);
	else
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
	if (this->value->method->type == rpt_value_type_error)
		result = rpt_value_copy(this->value);
	else
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


static rpt_value_ty *undefer _((rpt_value_ty *));

static rpt_value_ty *
undefer(vp)
	rpt_value_ty	*vp;
{
	rpt_value_pstate_ty *this;
	rpt_value_ty	*result;

	trace(("rpt_value_pstate::undefer(this = %08lX)\n{\n"/*}*/, (long)vp));
	this = (rpt_value_pstate_ty *)vp;
	if (!this->value)
		grab(this);
	assert(this->value);
	result = rpt_value_copy(this->value);
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
	undefer,
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
