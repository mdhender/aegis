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
 * MANIFEST: functions to manipulate reference values
 */

#include <aer/value/ref.h>
#include <error.h>
#include <trace.h>


typedef struct rpt_value_reference_ty rpt_value_reference_ty;
struct rpt_value_reference_ty
{
	RPT_VALUE
	rpt_value_ty	*value;
};


static void destruct _((rpt_value_ty *));

static void
destruct(vp)
	rpt_value_ty	*vp;
{
	rpt_value_reference_ty *this;

	trace(("reference::destruct(this = %08lX)\n{\n"/*}*/, (long)vp));
	this = (rpt_value_reference_ty *)vp;
	assert(this->method->type == rpt_value_type_reference);
	assert(this->value->reference_count >= 1);
	rpt_value_free(this->value);
	trace((/*{*/"}\n"));
}


static rpt_value_ty *arithmetic _((rpt_value_ty *));

static rpt_value_ty *
arithmetic(vp)
	rpt_value_ty	*vp;
{
	rpt_value_reference_ty *this;
	rpt_value_ty	*result;

	trace(("reference::destruct(this = %08lX)\n{\n"/*}*/, (long)vp));
	this = (rpt_value_reference_ty *)vp;
	assert(this->method->type == rpt_value_type_reference);
	assert(this->reference_count >= 1);
	assert(this->value->reference_count >= 1);
	assert(this->value->method->type != rpt_value_type_reference);
	result = rpt_value_arithmetic(this->value);
	trace(("return %08lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


static rpt_value_ty *stringize _((rpt_value_ty *));

static rpt_value_ty *
stringize(vp)
	rpt_value_ty	*vp;
{
	rpt_value_reference_ty *this;
	rpt_value_ty	*result;

	trace(("reference::stringize(this = %08lX)\n{\n"/*}*/, (long)vp));
	this = (rpt_value_reference_ty *)vp;
	assert(this->method->type == rpt_value_type_reference);
	assert(this->reference_count >= 1);
	assert(this->value->reference_count >= 1);
	assert(this->value->method->type != rpt_value_type_reference);
	result = rpt_value_stringize(this->value);
	trace(("return %08lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


static rpt_value_ty *booleanize _((rpt_value_ty *));

static rpt_value_ty *
booleanize(vp)
	rpt_value_ty	*vp;
{
	rpt_value_reference_ty *this;
	rpt_value_ty	*result;

	trace(("reference::booleanize(this = %08lX)\n{\n"/*}*/, (long)vp));
	this = (rpt_value_reference_ty *)vp;
	assert(this->method->type == rpt_value_type_reference);
	assert(this->reference_count >= 1);
	assert(this->value->reference_count >= 1);
	assert(this->value->method->type != rpt_value_type_reference);
	result = rpt_value_booleanize(this->value);
	trace(("return %08lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


static rpt_value_ty *lookup _((rpt_value_ty *, rpt_value_ty *, int));

static rpt_value_ty *
lookup(vp, rhs, lvalue)
	rpt_value_ty	*vp;
	rpt_value_ty	*rhs;
	int		lvalue;
{
	rpt_value_reference_ty *this;
	rpt_value_ty	*result;

	trace(("reference::lookup(this = %08lX)\n{\n"/*}*/, (long)vp));
	this = (rpt_value_reference_ty *)vp;
	assert(this->method->type == rpt_value_type_reference);
	assert(this->reference_count >= 1);
	assert(this->value->reference_count >= 1);
	assert(this->value->method->type != rpt_value_type_reference);
	result = rpt_value_lookup(this->value, rhs, lvalue);
	trace(("return %08lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


static rpt_value_ty *keys _((rpt_value_ty *));

static rpt_value_ty *
keys(vp)
	rpt_value_ty	*vp;
{
	rpt_value_reference_ty *this;
	rpt_value_ty	*result;

	trace(("reference::keys(this = %08lX)\n{\n"/*}*/, (long)vp));
	this = (rpt_value_reference_ty *)vp;
	assert(this->method->type == rpt_value_type_reference);
	assert(this->reference_count >= 1);
	assert(this->value->reference_count >= 1);
	assert(this->value->method->type != rpt_value_type_reference);
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
	rpt_value_reference_ty *this;
	rpt_value_ty	*result;

	trace(("reference::count(this = %08lX)\n{\n"/*}*/, (long)vp));
	this = (rpt_value_reference_ty *)vp;
	assert(this->method->type == rpt_value_type_reference);
	assert(this->reference_count >= 1);
	assert(this->value->reference_count >= 1);
	assert(this->value->method->type != rpt_value_type_reference);
	result = rpt_value_count(this->value);
	trace(("return %08lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


static char *type_of _((rpt_value_ty *));

static char *
type_of(that)
	rpt_value_ty	*that;
{
	rpt_value_reference_ty *this;

	this = (rpt_value_reference_ty *)that;
	assert(this->reference_count >= 1);
	assert(this->method->type == rpt_value_type_reference);
	assert(this->value->reference_count >= 1);
	assert(this->value->method->type != rpt_value_type_reference);
	return rpt_value_typeof(this->value);
}


static rpt_value_method_ty method =
{
	sizeof(rpt_value_reference_ty),
	"reference",
	rpt_value_type_reference,
	0, /* construct */
	destruct,
	arithmetic,
	stringize,
	booleanize,
	lookup,
	keys,
	count,
	type_of
};


rpt_value_ty *
rpt_value_reference(deeper)
	rpt_value_ty	*deeper;
{
	rpt_value_reference_ty *this;

	trace(("rpt_value_reference(deeper = %08lX)\n{\n"/*}*/, (long)deeper));
	assert(deeper->reference_count >= 1);
	assert(deeper->method->type != rpt_value_type_reference);
	this = (rpt_value_reference_ty *)rpt_value_alloc(&method);
	this->value = rpt_value_copy(deeper);
	trace(("return %08lX;\n", (long)this));
	trace((/*{*/"}\n"));
	return (rpt_value_ty *)this;
}


rpt_value_ty *
rpt_value_reference_get(vp)
	rpt_value_ty	*vp;
{
	rpt_value_reference_ty *this;
	rpt_value_ty	*result;

	trace(("reference::get(this = %08lX)\n{\n"/*}*/, (long)vp));
	this = (rpt_value_reference_ty *)vp;
	assert(this->method == &method);
	assert(this->reference_count >= 1);
	assert(this->value->reference_count >= 1);
	assert(this->value->method->type != rpt_value_type_reference);
	result = rpt_value_copy(this->value);
	trace(("return %08lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


void
rpt_value_reference_set(vp, deeper)
	rpt_value_ty	*vp;
	rpt_value_ty	*deeper;
{
	rpt_value_reference_ty *this;
	rpt_value_ty	*old;

	trace(("rpt_value_reference_set(vp = %08lX, deeper = %08lX)\n{\n"/*}*/,
		(long)vp, (long)deeper));
	assert(deeper->method->type != rpt_value_type_reference);
	assert(deeper->reference_count >= 1);
	this = (rpt_value_reference_ty *)vp;
	assert(this->method == &method);
	assert(this->reference_count >= 1);
	assert(this->value->reference_count >= 1);
	assert(this->value->method->type != rpt_value_type_reference);
	old = this->value;
	this->value = rpt_value_copy(deeper);
	assert(deeper->reference_count >= 2);
	rpt_value_free(old);
	trace((/*{*/"}\n"));
}
