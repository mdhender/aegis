/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1996, 1997, 1999, 2000 Peter Miller;
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
 * MANIFEST: functions to manipulate gstate values
 */

#include <ac/string.h> /* for strerror() */

#include <aer/value/error.h>
#include <aer/value/gstate.h>
#include <aer/value/integer.h>
#include <aer/value/list.h>
#include <aer/value/pstate.h>
#include <aer/value/string.h>
#include <aer/value/struct.h>
#include <change.h>
#include <error.h>
#include <project.h>
#include <str_list.h>
#include <sub.h>
#include <trace.h>


typedef struct rpt_value_gstate_ty rpt_value_gstate_ty;
struct rpt_value_gstate_ty
{
	RPT_VALUE
	string_list_ty	*keys;
};


static void construct _((rpt_value_ty *));

static void
construct(vp)
	rpt_value_ty	*vp;
{
	rpt_value_gstate_ty *this;

	trace(("rpt_value_gstate::construct(vp = %08lX)\n{\n"/*}*/, (long)vp));
	this = (rpt_value_gstate_ty *)vp;
	this->keys = 0;
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
	if (this->keys)
		string_list_delete(this->keys);
	trace((/*{*/"}\n"));
}


static rpt_value_ty *grab_one _((string_ty *));

static rpt_value_ty *
grab_one(project_name)
	string_ty	*project_name;
{
	project_ty	*pp;
	rpt_value_ty	*vp;
	string_ty	*name;
	rpt_value_ty	*value;
	change_ty	*cp;
	string_ty	*dd;
	cstate		cstate_data;
	int		err;

	/*
	 * get details of the project
	 * to put in the structure
	 */
	pp = project_alloc(project_name);
	project_bind_existing(pp);
	vp = rpt_value_struct((struct symtab_ty *)0);

	name = str_from_c("name");
	value = rpt_value_string(project_name_get(pp));
	rpt_value_struct__set(vp, name, value);
	str_free(name);
	rpt_value_free(value);

	err = project_is_readable(pp);
	if (err)
	{
		string_ty	*s;

		name = str_from_c("error");
		s = str_from_c(strerror(err));
		value = rpt_value_string(s);
		str_free(s);
		rpt_value_struct__set(vp, name, value);
		str_free(name);
		rpt_value_free(value);
	}
	else
	{
		/*
		 * The development directory of the project change is
		 * the one which contains the trunk or branch baseline.
		 */
		cp = project_change_get(pp);
		cstate_data = change_cstate_get(cp);
		if (cstate_data->state == cstate_state_being_developed)
		{
			dd = change_development_directory_get(cp, 0);
			name = str_from_c("directory");
			value = rpt_value_string(dd);
			rpt_value_struct__set(vp, name, value);
			str_free(name);
			rpt_value_free(value);
		}

		name = str_from_c("state");
		value = rpt_value_pstate(project_name_get(pp));
		rpt_value_struct__set(vp, name, value);
		str_free(name);
		rpt_value_free(value);
	}

	project_free(pp);

	/*
	 * all done
	 */
	return vp;
}


static void grab _((rpt_value_gstate_ty *));

static void
grab(this)
	rpt_value_gstate_ty *this;
{
	/*
	 * create a structure/array to hold the thing
	 */
	trace(("rpt_value_gstate::grab(this = %08lX)\n{\n"/*}*/, (long)this));
	assert(!this->keys);
	this->keys = string_list_new();

	/*
	 * ask gonzo for the list of project names
	 */
	project_list_get(this->keys);
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
	rpt_value_ty	*rhs2;
	rpt_value_ty	*result;

	trace(("rpt_value_gstate::lookup(this = %08lX)\n{\n"/*}*/, (long)vp));
	this = (rpt_value_gstate_ty *)vp;

	rhs2 = rpt_value_stringize(rhs);
	if (rhs2->method->type == rpt_value_type_string)
	{
		string_ty	*name;

		name = rpt_value_string_query(rhs);
		result = grab_one(name);
	}
	else
	{
		sub_context_ty	*scp;
		string_ty	*s;

		scp = sub_context_new();
		sub_var_set_charstar(scp, "Name1", "project");
		sub_var_set_charstar(scp, "Name2", rhs->method->name);
		s = subst_intl(scp, i18n("illegal lookup ($name1[$name2])"));
		sub_context_delete(scp);
		result = rpt_value_error((struct rpt_pos_ty *)0, s);
		str_free(s);
	}
	rpt_value_free(rhs2);

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
	size_t		j;
	string_ty	*s;
	rpt_value_ty	*ep;

	trace(("rpt_value_gstate::keys(this = %08lX)\n{\n"/*}*/, (long)vp));
	this = (rpt_value_gstate_ty *)vp;
	if (!this->keys)
		grab(this);
	assert(this->keys);

	result = rpt_value_list();
	for (j = 0; j < this->keys->nstrings; ++j)
	{
		s = this->keys->string[j];
		ep = rpt_value_string(s);
		rpt_value_list_append(result, ep);
		rpt_value_free(ep);
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
	rpt_value_gstate_ty *this;
	rpt_value_ty	*result;

	trace(("rpt_value_gstate::count(this = %08lX)\n{\n"/*}*/, (long)vp));
	this = (rpt_value_gstate_ty *)vp;
	if (!this->keys)
		grab(this);
	assert(this->keys);
	result = rpt_value_integer((long)this->keys->nstrings);
	trace(("return %08lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


static char *type_of _((rpt_value_ty *));

static char *
type_of(vp)
	rpt_value_ty	*vp;
{
	return "struct";
}


static rpt_value_method_ty method =
{
	sizeof(rpt_value_gstate_ty),
	"gstate",
	rpt_value_type_structure,
	construct,
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
rpt_value_gstate()
{
	static rpt_value_ty *vp;

	if (!vp)
		vp = rpt_value_alloc(&method);
	return vp;
}
