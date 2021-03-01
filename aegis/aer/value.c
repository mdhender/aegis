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
 * MANIFEST: functions to manipulate values
 */

#include <aer/value.h>
#include <aer/value/error.h>
#include <error.h>
#include <mem.h>
#include <trace.h>


rpt_value_ty *
rpt_value_alloc(method)
	rpt_value_method_ty *method;
{
	rpt_value_ty	*this;

	trace(("value::alloc(method = %08lX)\n{\n"/*}*/, (long)method));
	trace(("method is \"%s\"\n", method->name));
	this = mem_alloc(method->size);
	this->method = method;
	this->reference_count = 1;
	if (method->construct)
		method->construct(this);
	trace(("return %08lX;\n", (long)this));
	trace((/*{*/"}\n"));
	return this;
}


rpt_value_ty *
rpt_value_copy(this)
	rpt_value_ty	*this;
{
	trace(("value::copy(this = %08lX)\n{\n"/*}*/, (long)this));
	assert(this->reference_count >= 1);
	trace(("method is \"%s\"\n", this->method->name));
	this->reference_count++;
	trace(("reference_count = %ld /* after */\n", this->reference_count));
	trace(("return %08lX;\n", (long)this));
	trace((/*{*/"}\n"));
	return this;
}


void
rpt_value_free(this)
	rpt_value_ty	*this;
{
	trace(("value::free(this = %08lX)\n{\n"/*}*/, (long)this));
	assert(this->reference_count >= 1);
	trace(("method is \"%s\"\n", this->method->name));
	trace(("reference_count = %ld /* before */\n", this->reference_count));
	this->reference_count--;
	if (this->reference_count <= 0)
	{
		if (this->method->destruct)
			this->method->destruct(this);
		mem_free(this);
	}
	trace((/*{*/"}\n"));
}


rpt_value_ty *
rpt_value_arithmetic(vp)
	rpt_value_ty	*vp;
{
	if (vp->method->arithmetic)
		return vp->method->arithmetic(vp);
	return rpt_value_copy(vp);
}


rpt_value_ty *
rpt_value_stringize(vp)
	rpt_value_ty	*vp;
{
	if (vp->method->stringize)
		return vp->method->stringize(vp);
	return rpt_value_copy(vp);
}


rpt_value_ty *
rpt_value_booleanize(vp)
	rpt_value_ty	*vp;
{
	if (vp->method->booleanize)
		return vp->method->booleanize(vp);
	return rpt_value_copy(vp);
}


rpt_value_ty *
rpt_value_lookup(lhs, rhs, lvalue)
	rpt_value_ty	*lhs;
	rpt_value_ty	*rhs;
	int		lvalue;
{
	if (lhs->method->lookup)
		return lhs->method->lookup(lhs, rhs, lvalue);
	return
		rpt_value_error
		(
			"illegal lookup (%s[%s])",
			lhs->method->name,
			rhs->method->name
		);
}


rpt_value_ty *
rpt_value_keys(vp)
	rpt_value_ty	*vp;
{
	if (vp->method->keys)
		return vp->method->keys(vp);
	return
		rpt_value_error
		(
			"illegal keys request (%s)",
			vp->method->name
		);
}


rpt_value_ty *
rpt_value_count(vp)
	rpt_value_ty	*vp;
{
	if (vp->method->count)
		return vp->method->count(vp);
	return
		rpt_value_error
		(
			"illegal count request (%s)",
			vp->method->name
		);
}


char *
rpt_value_typeof(vp)
	rpt_value_ty	*vp;
{
	if (vp->method->type_of)
		return vp->method->type_of(vp);
	return vp->method->name;
}
