//
//	aegis - project change supervisor
//	Copyright (C) 1994-1996, 1999, 2003, 2004 Peter Miller;
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
// MANIFEST: functions to manipulate list values
//

#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <aer/value/list.h>
#include <aer/value/null.h>
#include <aer/value/real.h>
#include <error.h>
#include <mem.h>
#include <sub.h>


typedef struct rpt_value_list_ty rpt_value_list_ty;
struct rpt_value_list_ty
{
	RPT_VALUE
	size_t		length;
	size_t		max;
	rpt_value_ty	**item;
};


static void
construct(rpt_value_ty *vp)
{
	rpt_value_list_ty *this_thing;

	this_thing = (rpt_value_list_ty *)vp;
	assert(this_thing->method->type == rpt_value_type_list);
	this_thing->length = 0;
	this_thing->max = 0;
	this_thing->item = 0;
}


static void
destruct(rpt_value_ty *vp)
{
	rpt_value_list_ty *this_thing;
	size_t		j;

	this_thing = (rpt_value_list_ty *)vp;
	assert(this_thing->method->type == rpt_value_type_list);
	for (j = 0; j < this_thing->length; ++j)
		rpt_value_free(this_thing->item[j]);
	if (this_thing->item)
		mem_free(this_thing->item);
}


static rpt_value_ty *
lookup(rpt_value_ty *vp, rpt_value_ty *rhs, int lvalue)
{
	sub_context_ty	*scp;
	rpt_value_list_ty *this_thing;
	rpt_value_ty	*rhs2;
	long		idx;
	string_ty	*s;
	rpt_value_ty	*result;

	if (lvalue)
	{
		scp = sub_context_new();
		sub_var_set_charstar(scp, "Name1", vp->method->name);
		sub_var_set_charstar(scp, "Name2", rhs->method->name);
		s =
			subst_intl
			(
				scp,
			i18n("may not assign to a list member ($name1[$name2])")
			);
		sub_context_delete(scp);
		result = rpt_value_error((struct rpt_pos_ty *)0, s);
		str_free(s);
		return result;
	}
	this_thing = (rpt_value_list_ty *)vp;
	assert(this_thing->method->type == rpt_value_type_list);
	rhs2 = rpt_value_arithmetic(rhs);
	switch (rhs2->method->type)
	{
	case rpt_value_type_integer:
		idx = rpt_value_integer_query(rhs2);
		break;

	case rpt_value_type_real:
		idx = (long int)rpt_value_real_query(rhs2);
		break;

	default:
		rpt_value_free(rhs2);
		scp = sub_context_new();
		sub_var_set_charstar(scp, "Name1", vp->method->name);
		sub_var_set_charstar(scp, "Name2", rhs->method->name);
		s = subst_intl(scp, i18n("illegal lookup ($name1[$name2])"));
		sub_context_delete(scp);
		result = rpt_value_error((struct rpt_pos_ty *)0, s);
		str_free(s);
		return result;
	}
	rpt_value_free(rhs2);
	if (idx < 0 || idx >= (long)this_thing->length)
		return rpt_value_nul();
	return rpt_value_copy(this_thing->item[idx]);
}


static rpt_value_ty *
keys(rpt_value_ty *vp)
{
	rpt_value_list_ty *this_thing;
	rpt_value_ty	*result;
	long		j;
	rpt_value_ty	*n;

	this_thing = (rpt_value_list_ty *)vp;
	result = rpt_value_list();
	for (j = 0; j < (long)this_thing->length; ++j)
	{
		n = rpt_value_integer(j);
		rpt_value_list_append(result, n);
		rpt_value_free(n);
	}
	return result;
}


static rpt_value_ty *
count(rpt_value_ty *vp)
{
	rpt_value_list_ty *this_thing;

	this_thing = (rpt_value_list_ty *)vp;
	return rpt_value_integer(this_thing->length);
}


static rpt_value_method_ty method =
{
	sizeof(rpt_value_list_ty),
	"list",
	rpt_value_type_list,
	construct,
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
rpt_value_list()
{
	return rpt_value_alloc(&method);
}


void
rpt_value_list_append(rpt_value_ty *vp, rpt_value_ty *child)
{
	rpt_value_list_ty *this_thing;

	this_thing = (rpt_value_list_ty *)vp;
	assert(this_thing->method->type == rpt_value_type_list);
	if (this_thing->length >= this_thing->max)
	{
		size_t		nbytes;

		this_thing->max = this_thing->max * 2 + 4;
		nbytes = this_thing->max * sizeof(rpt_value_ty *);
		this_thing->item =
                    (rpt_value_ty **)mem_change_size(this_thing->item, nbytes);
	}
	this_thing->item[this_thing->length++] = rpt_value_copy(child);
}


long
rpt_value_list_length(rpt_value_ty *vp)
{
	rpt_value_list_ty *this_thing;

	this_thing = (rpt_value_list_ty *)vp;
	assert(this_thing->method->type == rpt_value_type_list);
	return this_thing->length;
}


rpt_value_ty *
rpt_value_list_nth(rpt_value_ty *vp, long n)
{
	rpt_value_list_ty *this_thing;

	this_thing = (rpt_value_list_ty *)vp;
	assert(this_thing->method->type == rpt_value_type_list);
	assert(n >= 0);
	assert((size_t)n < this_thing->length);
	return this_thing->item[n];
}
