/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1995, 1996, 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate values
 */

#include <aer/expr.h>
#include <aer/value.h>
#include <aer/value/error.h>
#include <error.h>
#include <mem.h>
#include <sub.h>
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
	sub_context_ty	*scp;
	string_ty	*s;
	rpt_value_ty	*result;

	if (lhs->method->lookup)
		return lhs->method->lookup(lhs, rhs, lvalue);

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name1", lhs->method->name);
	sub_var_set_charstar(scp, "Name2", rhs->method->name);
	s = subst_intl(scp, i18n("illegal lookup ($name1[$name2])"));
	sub_context_delete(scp);
	result = rpt_value_error((struct rpt_pos_ty *)0, s);
	str_free(s);
	return result;
}


rpt_value_ty *
rpt_value_keys(vp)
	rpt_value_ty	*vp;
{
	sub_context_ty	*scp;
	string_ty	*s;
	rpt_value_ty	*result;

	if (vp->method->keys)
		return vp->method->keys(vp);

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name", vp->method->name);
	s = subst_intl(scp, i18n("illegal keys request ($name)"));
	sub_context_delete(scp);
	result = rpt_value_error((struct rpt_pos_ty *)0, s);
	str_free(s);
	return result;
}


rpt_value_ty *
rpt_value_count(vp)
	rpt_value_ty	*vp;
{
	sub_context_ty	*scp;
	string_ty	*s;
	rpt_value_ty	*result;

	if (vp->method->count)
		return vp->method->count(vp);

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name", vp->method->name);
	s = subst_intl(scp, i18n("illegal count request ($name)"));
	sub_context_delete(scp);
	result = rpt_value_error((struct rpt_pos_ty *)0, s);
	str_free(s);
	return result;
}


char *
rpt_value_typeof(vp)
	rpt_value_ty	*vp;
{
	char		*result;

	trace(("value::typeof(vp = %08lX)\n{\n"/*}*/, (long)vp));
	if (vp->method->type_of)
		result = vp->method->type_of(vp);
	else
		result = vp->method->name;
	trace(("result = \"%s\";\n", result));
	trace((/*{*/"}\n"));
	return result;
}


rpt_value_ty *
rpt_value_undefer(vp)
	rpt_value_ty	*vp;
{
	rpt_value_ty	*result;

	trace(("value::undefer(vp = %08lX)\n{\n"/*}*/, (long)vp));
	if (vp->method->undefer)
		result = vp->method->undefer(vp);
	else
		result = rpt_value_copy(vp);
	assert(result->method->type != rpt_value_type_deferred);
	trace(("result = \"%s\";\n", result));
	trace((/*{*/"}\n"));
	return result;
}
