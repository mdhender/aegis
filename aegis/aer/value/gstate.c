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
 * MANIFEST: functions to manipulate gstate values
 */

#include <aer/value/gstate.h>
#include <aer/value/pstate.h>
#include <aer/value/string.h>
#include <aer/value/struct.h>
#include <error.h>
#include <gonzo.h>
#include <project.h>
#include <trace.h>
#include <word.h>


typedef struct rpt_value_gstate_ty rpt_value_gstate_ty;
struct rpt_value_gstate_ty
{
	RPT_VALUE
	rpt_value_ty	*value;
};


static void construct _((rpt_value_ty *));

static void
construct(vp)
	rpt_value_ty	*vp;
{
	rpt_value_gstate_ty *this;

	trace(("rpt_value_gstate::construct(vp = %08lX)\n{\n"/*}*/, (long)vp));
	this = (rpt_value_gstate_ty *)vp;
	this->value = 0;
	trace((/*{*/"}\n"));
}


static void destruct _((rpt_value_ty *));

static void
destruct(vp)
	rpt_value_ty	*vp;
{
	rpt_value_gstate_ty *this;

	trace(("rpt_value_gstate::destruct(vp = %08lX)\n{\n"/*}*/, (long)vp));
	this = (rpt_value_gstate_ty *)vp;
	if (this->value)
		rpt_value_free(this->value);
	trace((/*{*/"}\n"));
}


static void grab _((rpt_value_gstate_ty *));

static void
grab(this)
	rpt_value_gstate_ty *this;
{
	wlist		project_name_list;
	size_t		j;

	/*
	 * create a structure/array to hold the thing
	 */
	trace(("rpt_value_gstate::grab(this = %08lX)\n{\n"/*}*/, (long)this));
	assert(!this->value);
	this->value = rpt_value_struct((struct symtab_ty *)0);

	/*
	 * ask gonzo for the list of project names
	 */
	gonzo_project_list(&project_name_list);

	/*
	 * create a structure for each project
	 */
	for (j = 0; j < project_name_list.wl_nwords; ++j)
	{
		project_ty	*pp;
		rpt_value_ty	*vp;
		string_ty	*name;
		rpt_value_ty	*value;

		/*
		 * get details of the project
		 * to put in the structure
		 */
		pp = project_alloc(project_name_list.wl_word[j]);
		project_bind_existing(pp);
		vp = rpt_value_struct((struct symtab_ty *)0);

		name = str_from_c("name");
		value = rpt_value_string(project_name_list.wl_word[j]);
		rpt_value_struct__set(vp, name, value);
		str_free(name);
		rpt_value_free(value);

		name = str_from_c("directory");
		value = rpt_value_string(project_home_path_get(pp));
		rpt_value_struct__set(vp, name, value);
		str_free(name);
		rpt_value_free(value);

		name = str_from_c("state");
		value = rpt_value_pstate(project_name_list.wl_word[j]);
		rpt_value_struct__set(vp, name, value);
		str_free(name);
		rpt_value_free(value);
		project_free(pp);
		
		/*
		 * add this structure to the array
		 */
		name = project_name_list.wl_word[j];
		rpt_value_struct__set(this->value, name, vp);
		rpt_value_free(vp);
	}
	wl_free(&project_name_list);
	trace((/*{*/"}\n"));
}


static rpt_value_ty *lookup _((rpt_value_ty *, rpt_value_ty *, int));

static rpt_value_ty *
lookup(vp, rhs, lval)
	rpt_value_ty	*vp;
	rpt_value_ty	*rhs;
	int		lval;
{
	rpt_value_gstate_ty *this;
	rpt_value_ty	*result;

	trace(("rpt_value_gstate::lookup(this = %08lX)\n{\n"/*}*/, (long)vp));
	this = (rpt_value_gstate_ty *)vp;
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
	rpt_value_gstate_ty *this;
	rpt_value_ty	*result;

	trace(("rpt_value_gstate::keys(this = %08lX)\n{\n"/*}*/, (long)vp));
	this = (rpt_value_gstate_ty *)vp;
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
	rpt_value_gstate_ty *this;
	rpt_value_ty	*result;

	trace(("rpt_value_gstate::count(this = %08lX)\n{\n"/*}*/, (long)vp));
	this = (rpt_value_gstate_ty *)vp;
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
	rpt_value_gstate_ty *this;
	char		*result;

	trace(("rpt_value_gstate::type_of(this = %08lX)\n{\n"/*}*/, (long)vp));
	this = (rpt_value_gstate_ty *)vp;
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
	sizeof(rpt_value_gstate_ty),
	"gstate",
	rpt_value_type_deferred,
	construct,
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
rpt_value_gstate()
{
	static rpt_value_ty *vp;

	if (!vp)
		vp = rpt_value_alloc(&method);
	return vp;
}
