/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1995, 1996 Peter Miller;
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
 * MANIFEST: functions to manipulate list values
 */

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


static void construct _((rpt_value_ty *));

static void
construct(vp)
	rpt_value_ty	*vp;
{
	rpt_value_list_ty *this;

	this = (rpt_value_list_ty *)vp;
	assert(this->method->type == rpt_value_type_list);
	this->length = 0;
	this->max = 0;
	this->item = 0;
}


static void destruct _((rpt_value_ty *));

static void
destruct(vp)
	rpt_value_ty	*vp;
{
	rpt_value_list_ty *this;
	size_t		j;

	this = (rpt_value_list_ty *)vp;
	assert(this->method->type == rpt_value_type_list);
	for (j = 0; j < this->length; ++j)
		rpt_value_free(this->item[j]);
	if (this->item)
		mem_free(this->item);
}


static rpt_value_ty *lookup _((rpt_value_ty *, rpt_value_ty *, int));

static rpt_value_ty *
lookup(vp, rhs, lvalue)
	rpt_value_ty	*vp;
	rpt_value_ty	*rhs;
	int		lvalue;
{
	sub_context_ty	*scp;
	rpt_value_list_ty *this;
	rpt_value_ty	*rhs2;
	long		idx;
	string_ty	*s;
	rpt_value_ty	*result;

	if (lvalue)
	{
		scp = sub_context_new();
		sub_var_set(scp, "Name1", "%s", vp->method->name);
		sub_var_set(scp, "Name2", "%s", rhs->method->name);
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
	this = (rpt_value_list_ty *)vp;
	assert(this->method->type == rpt_value_type_list);
	rhs2 = rpt_value_arithmetic(rhs);
	switch (rhs2->method->type)
	{
	case rpt_value_type_integer:
		idx = rpt_value_integer_query(rhs2);
		break;

	case rpt_value_type_real:
		idx = rpt_value_real_query(rhs2);
		break;

	default:
		rpt_value_free(rhs2);
		scp = sub_context_new();
		sub_var_set(scp, "Name1", "%s", vp->method->name);
		sub_var_set(scp, "Name2", "%s", rhs->method->name);
		s = subst_intl(scp, i18n("illegal lookup ($name1[$name2])"));
		sub_context_delete(scp);
		result = rpt_value_error((struct rpt_pos_ty *)0, s);
		str_free(s);
		return result;
	}
	rpt_value_free(rhs2);
	if (idx < 0 || idx >= this->length)
		return rpt_value_nul();
	return rpt_value_copy(this->item[idx]);
}


static rpt_value_ty *keys _((rpt_value_ty *));

static rpt_value_ty *
keys(vp)
	rpt_value_ty	*vp;
{
	rpt_value_list_ty *this;
	rpt_value_ty	*result;
	long		j;
	rpt_value_ty	*n;

	this = (rpt_value_list_ty *)vp;
	result = rpt_value_list();
	for (j = 0; j < this->length; ++j)
	{
		n = rpt_value_integer(j);
		rpt_value_list_append(result, n);
		rpt_value_free(n);
	}
	return result;
}


static rpt_value_ty *count _((rpt_value_ty *));

static rpt_value_ty *
count(vp)
	rpt_value_ty	*vp;
{
	rpt_value_list_ty *this;

	this = (rpt_value_list_ty *)vp;
	return rpt_value_integer(this->length);
}


static rpt_value_method_ty method =
{
	sizeof(rpt_value_list_ty),
	"list",
	rpt_value_type_list,
	construct,
	destruct,
	0, /* arithmetic */
	0, /* stringize */
	0, /* booleanize */
	lookup,
	keys,
	count,
	0, /* type_of */
	0, /* undefer */
};


rpt_value_ty *
rpt_value_list()
{
	return rpt_value_alloc(&method);
}


void
rpt_value_list_append(vp, child)
	rpt_value_ty	*vp;
	rpt_value_ty	*child;
{
	rpt_value_list_ty *this;

	this = (rpt_value_list_ty *)vp;
	assert(this->method->type == rpt_value_type_list);
	if (this->length >= this->max)
	{
		size_t		nbytes;

		this->max = this->max * 2 + 4;
		nbytes = this->max * sizeof(rpt_value_ty *);
		this->item = mem_change_size(this->item, nbytes);
	}
	this->item[this->length++] = rpt_value_copy(child);
}


long
rpt_value_list_length(vp)
	rpt_value_ty	*vp;
{
	rpt_value_list_ty *this;

	this = (rpt_value_list_ty *)vp;
	assert(this->method->type == rpt_value_type_list);
	return this->length;
}


rpt_value_ty *
rpt_value_list_nth(vp, n)
	rpt_value_ty	*vp;
	long		n;
{
	rpt_value_list_ty *this;

	this = (rpt_value_list_ty *)vp;
	assert(this->method->type == rpt_value_type_list);
	assert(n >= 0);
	assert(n < this->length);
	return this->item[n];
}
