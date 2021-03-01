/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1995, 1996, 1997 Peter Miller;
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
 * MANIFEST: functions to manipulate group values
 */

#include <ac/stdio.h>
#include <ac/grp.h>

#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <aer/value/list.h>
#include <aer/value/group.h>
#include <aer/value/string.h>
#include <aer/value/struct.h>
#include <error.h>
#include <sub.h>
#include <trace.h>


static rpt_value_ty *gr_to_struct _((struct group *));

static rpt_value_ty *
gr_to_struct(gr)
	struct group	*gr;
{
	rpt_value_ty	*result;
	string_ty	*name;
	string_ty	*s;
	rpt_value_ty	*value;
	long		j;

	trace(("gr_to_struct()\n{\n"/*}*/));
	result = rpt_value_struct((struct symtab_ty *)0);

	trace(("gr_name\n"));
	name = str_from_c("gr_name");
	s = str_from_c(gr->gr_name);
	value = rpt_value_string(s);
	str_free(s);
	rpt_value_struct__set(result, name, value);
	str_free(name);
	assert(value->reference_count == 2);
	rpt_value_free(value);

	trace(("gr_gid\n"));
	name = str_from_c("gr_gid");
	value = rpt_value_integer((long)gr->gr_gid);
	rpt_value_struct__set(result, name, value);
	str_free(name);
	assert(value->reference_count == 2);
	rpt_value_free(value);

	trace(("gr_mem\n"));
	name = str_from_c("gr_mem");
	value = rpt_value_list();
	rpt_value_struct__set(result, name, value);
	str_free(name);
	assert(value->reference_count == 2);
	for (j = 0; gr->gr_mem[j]; ++j)
	{
		rpt_value_ty	*vp;

		name = str_from_c(gr->gr_mem[j]);
		vp = rpt_value_string(name);
		str_free(name);
		rpt_value_list_append(value, vp);
		assert(vp->reference_count == 2);
		rpt_value_free(vp);
	}
	assert(value->reference_count == 2);
	rpt_value_free(value);

	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


static rpt_value_ty *lookup _((rpt_value_ty *, rpt_value_ty *, int));

static rpt_value_ty *
lookup(lhs, rhs, lvalue)
	rpt_value_ty	*lhs;
	rpt_value_ty	*rhs;
	int		lvalue;
{
	rpt_value_ty	*rhs2;
	rpt_value_ty	*result;
	struct group	*gr;

	trace(("value_group::lookup()\n{\n"/*}*/));
	rhs2 = rpt_value_arithmetic(rhs);
	if (rhs2->method->type == rpt_value_type_integer)
	{
		int		gid;

		gid = rpt_value_integer_query(rhs2);
		rpt_value_free(rhs2);
		gr = getgrgid(gid);
		if (gr)
			result = gr_to_struct(gr);
		else
		{
			sub_context_ty	*scp;
			string_ty	*s;

			scp = sub_context_new();
			sub_var_set(scp, "Number", "%d", gid);
			s = subst_intl(scp, i18n("gid $number unknown"));
			sub_context_delete(scp);
			result = rpt_value_error((struct rpt_pos_ty *)0, s);
			str_free(s);
		}
	}
	else
	{
		rpt_value_free(rhs2);
		rhs2 = rpt_value_stringize(rhs);
		if (rhs2->method->type == rpt_value_type_string)
		{
			string_ty	*name;

			name = rpt_value_string_query(rhs2);
			gr = getgrnam(name->str_text);
			if (gr)
				result = gr_to_struct(gr);
			else
			{
				sub_context_ty	*scp;
				string_ty	*s;

				scp = sub_context_new();
				sub_var_set(scp, "Name", "%S", name);
				s = subst_intl(scp, i18n("group \"$name\" unknown"));
				sub_context_delete(scp);
				result = rpt_value_error((struct rpt_pos_ty *)0, s);
				str_free(s);
			}
		}
		else
		{
			sub_context_ty	*scp;
			string_ty	*s;

			scp = sub_context_new();
			sub_var_set(scp, "Name1", "group");
			sub_var_set(scp, "Name2", "%s", rhs->method->name);
			s = subst_intl(scp, i18n("illegal lookup ($name1[$name2])"));
			sub_context_delete(scp);
			result = rpt_value_error((struct rpt_pos_ty *)0, s);
			str_free(s);
		}
		rpt_value_free(rhs2);
	}
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


static rpt_value_ty *keys _((rpt_value_ty *));

static rpt_value_ty *
keys(vp)
	rpt_value_ty	*vp;
{
	rpt_value_ty	*result;
	struct group	*gr;
	string_ty	*s;
	rpt_value_ty	*ep;

	result = rpt_value_list();
	setgrent();
	for (;;)
	{
		gr = getgrent();
		if (!gr)
			break;
		
		s = str_from_c(gr->gr_name);
		ep = rpt_value_string(s);
		str_free(s);
		rpt_value_list_append(result, ep);
		rpt_value_free(ep);
	}
	return result;
}


static rpt_value_ty *count _((rpt_value_ty *));

static rpt_value_ty *
count(vp)
	rpt_value_ty	*vp;
{
	struct group	*gr;
	long		n;

	setgrent();
	n = 0;
	for (;;)
	{
		gr = getgrent();
		if (!gr)
			break;
		++n;
	}
	return rpt_value_integer(n);
}


static char *type_of _((rpt_value_ty *));

static char *
type_of(this)
	rpt_value_ty	*this;
{
	return "struct";
}


static rpt_value_method_ty method =
{
	sizeof(rpt_value_ty),
	"group",
	rpt_value_type_structure,
	0, /* construct */
	0, /* destruct */
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
rpt_value_group()
{
	static rpt_value_ty *vp;

	if (!vp)
		vp = rpt_value_alloc(&method);
	return rpt_value_copy(vp);
}
