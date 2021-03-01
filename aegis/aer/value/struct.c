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
 * MANIFEST: functions to manipulate struct values
 */

#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <aer/value/list.h>
#include <aer/value/nul.h>
#include <aer/value/ref.h>
#include <aer/value/string.h>
#include <aer/value/struct.h>
#include <error.h>
#include <symtab.h>
#include <trace.h>


typedef struct rpt_value_struct_ty rpt_value_struct_ty;
struct rpt_value_struct_ty
{
	RPT_VALUE
	symtab_ty	*value;
};


static void reap _((void *));

static void
reap(p)
	void		*p;
{
	rpt_value_ty	*vp;

	trace(("reap(%08lX);\n", (long)p));
	vp = p;
	rpt_value_free(vp);
}


#ifdef DEBUG

static void func _((symtab_ty *, string_ty *, void *, void *));

static void
func(stp, key, data, arg)
	symtab_ty	*stp;
	string_ty	*key;
	void		*data;
	void		*arg;
{
	trace(("key=\"%s\", data=%08lX\n", key->str_text, (long)data));
}

#endif


static void destruct _((rpt_value_ty *));

static void
destruct(vp)
	rpt_value_ty	*vp;
{
	rpt_value_struct_ty *this;

	trace(("value_struct::destruct(this = %08lX)\n{\n"/*}*/, (long)vp));
	this = (rpt_value_struct_ty *)vp;
	assert(this->method->type == rpt_value_type_structure);
#ifdef DEBUG
	symtab_walk(this->value, func, (void *)0);
#endif
	symtab_free(this->value);
	trace((/*{*/"}\n"));
}


static rpt_value_ty *lookup _((rpt_value_ty *, rpt_value_ty *, int));

static rpt_value_ty *
lookup(vp, rhs, lvalue)
	rpt_value_ty	*vp;
	rpt_value_ty	*rhs;
	int		lvalue;
{
	rpt_value_struct_ty *this;
	rpt_value_ty	*rhs2;
	rpt_value_ty	*data;
	rpt_value_ty	*result;

	trace(("value_struct::lookup(lhs = %08lX, rhs = %08lX, lvalue = %d)\n{\n"/*}*/,
		(long)vp, (long)rhs, lvalue));
	this = (rpt_value_struct_ty *)vp;
	assert(this->reference_count >= 1);
	assert(this->method->type == rpt_value_type_structure);
	rhs2 = rpt_value_stringize(rhs);
	if (rhs2->method->type != rpt_value_type_string)
	{
		rpt_value_free(rhs2);
		trace((/*{*/"}\n"));
		return
			rpt_value_error
			(
				"illegal lookup (%s[%s])",
				vp->method->name,
				rhs->method->name
			);
	}
	trace(("find the datum\n"));
	data = symtab_query(this->value, rpt_value_string_query(rhs2));
	trace(("data = %08lX;\n", (long)data));
	if (!data)
	{
		result = rpt_value_nul();
		if (lvalue)
		{
			trace(("create new element\n"));
			data = rpt_value_reference(result);
			/* reference takes a copy */
			rpt_value_free(result);
			symtab_assign
			(
				this->value,
				rpt_value_string_query(rhs2),
				data
			);
			result = rpt_value_copy(data);
		}
		else
		{
			/*
			 * Note: a nul value will always have ref count
			 * >= 2 (see the assert below) because there is
			 * only ever one allocated.  It is OK for the
			 * nul returned from this function to ve
			 * rpt_value_free()ed later, because it is not
			 * anywhere attached.
			 */
			trace(("element not found, and none created\n"));
		}
	}
	else
	{
		if (data->method->type == rpt_value_type_reference && !lvalue)
		{
			/*
			 * this returns a copy of the referenced value
			 */
			trace(("resolve the reference\n"));
			result = rpt_value_reference_get(data);
		}
		else
		{
			trace(("copy the data\n"));
			result = rpt_value_copy(data);
		}
	}
	assert(rhs2->reference_count >= 1);
	rpt_value_free(rhs2);
	assert(result->reference_count >= 2);
	trace(("return %08lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


static void keys_callback _((symtab_ty *, string_ty *, void *, void *));

static void
keys_callback(stp, key, data, arg)
	symtab_ty	*stp;
	string_ty	*key;
	void		*data;
	void		*arg;
{
	rpt_value_ty	*vlp;
	rpt_value_ty	*s;

	vlp = arg;
	s = rpt_value_string(key);
	rpt_value_list_append(vlp, s);
	rpt_value_free(s);
}


static rpt_value_ty *keys _((rpt_value_ty *));

static rpt_value_ty *
keys(vp)
	rpt_value_ty	*vp;
{
	rpt_value_struct_ty *this;
	rpt_value_ty	*result;

	this = (rpt_value_struct_ty *)vp;
	assert(this->method->type == rpt_value_type_structure);
	result = rpt_value_list();
	symtab_walk(this->value, keys_callback, result);
	return result;
}


static rpt_value_ty *count _((rpt_value_ty *));

static rpt_value_ty *
count(vp)
	rpt_value_ty	*vp;
{
	rpt_value_struct_ty *this;

	this = (rpt_value_struct_ty *)vp;
	assert(this->method->type == rpt_value_type_structure);
	return rpt_value_integer(this->value->hash_load);
}


static rpt_value_method_ty method =
{
	sizeof(rpt_value_struct_ty),
	"struct",
	rpt_value_type_structure,
	0, /* construct */
	destruct,
	0, /* arithmetic */
	0, /* stringize */
	0, /* booleanize */
	lookup,
	keys,
	count,
	0, /* type_of */
};


rpt_value_ty *
rpt_value_struct(stp)
	symtab_ty	*stp;
{
	rpt_value_struct_ty *this;

	this = (rpt_value_struct_ty *)rpt_value_alloc(&method);
	if (!stp)
		stp = symtab_alloc(5);
	stp->reap = reap;
	this->value = stp;
	return (rpt_value_ty *)this;
}


symtab_ty *
rpt_value_struct_query(vp)
	rpt_value_ty	*vp;
{
	rpt_value_struct_ty *this;

	this = (rpt_value_struct_ty *)vp;
	assert(this->method == &method);
	return this->value;
}


rpt_value_ty *
rpt_value_struct_lookup(vp, name)
	rpt_value_ty	*vp;
	string_ty	*name;
{
	rpt_value_struct_ty *this;

	this = (rpt_value_struct_ty *)vp;
	assert(this->method == &method);
	return symtab_query(this->value, name);
}


void
rpt_value_struct__set(vp, name, value)
	rpt_value_ty	*vp;
	string_ty	*name;
	rpt_value_ty	*value;
{
	rpt_value_struct_ty *this;

	trace(("rpt_value_struct__set(this = %08lX, name = \"%s\", \
value = %08lX)\n{\n"/*}*/, (long)vp, name->str_text, (long)value));
	this = (rpt_value_struct_ty *)vp;
	assert(this->method == &method);
	if (value->method->type == rpt_value_type_reference)
		value = rpt_value_copy(value);
	else
		value = rpt_value_reference(value);
	symtab_assign(this->value, name, value);
	trace((/*{*/"}\n"));
}
