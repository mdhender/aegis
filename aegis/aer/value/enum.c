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
 * MANIFEST: functions to manipulate enumeration values
 */

#include <aer/value/boolean.h>
#include <aer/value/enum.h>
#include <aer/value/integer.h>
#include <aer/value/string.h>
#include <error.h>
#include <str.h>


typedef struct rpt_value_enumeration_ty rpt_value_enumeration_ty;
struct rpt_value_enumeration_ty
{
	RPT_VALUE
	long		value_n;
	string_ty	*value_s;
};


static void destruct _((rpt_value_ty *));

static void
destruct(vp)
	rpt_value_ty	*vp;
{
	rpt_value_enumeration_ty *this;

	this = (rpt_value_enumeration_ty *)vp;
	assert(this->method->type == rpt_value_type_enumeration);
	str_free(this->value_s);
}


static rpt_value_ty *stringize _((rpt_value_ty *));

static rpt_value_ty *
stringize(vp)
	rpt_value_ty	*vp;
{
	rpt_value_enumeration_ty *this;

	this = (rpt_value_enumeration_ty *)vp;
	assert(this->method->type == rpt_value_type_enumeration);
	return rpt_value_string(this->value_s);
}


static rpt_value_ty *arithmetic _((rpt_value_ty *));

static rpt_value_ty *
arithmetic(vp)
	rpt_value_ty	*vp;
{
	rpt_value_enumeration_ty *this;

	this = (rpt_value_enumeration_ty *)vp;
	assert(this->method->type == rpt_value_type_enumeration);
	return rpt_value_integer(this->value_n);
}


static rpt_value_ty *booleanize _((rpt_value_ty *));

static rpt_value_ty *
booleanize(vp)
	rpt_value_ty	*vp;
{
	rpt_value_enumeration_ty *this;

	this = (rpt_value_enumeration_ty *)vp;
	assert(this->method->type == rpt_value_type_enumeration);
	return rpt_value_boolean(this->value_n != 0);
}


static rpt_value_method_ty method =
{
	sizeof(rpt_value_enumeration_ty),
	"enumeration",
	rpt_value_type_enumeration,
	0, /* construct */
	destruct,
	arithmetic,
	stringize,
	booleanize,
	0, /* lookup */
	0, /* keys */
	0, /* count */
	0, /* type_of */
};


rpt_value_ty *
rpt_value_enumeration(n, s)
	long		n;
	string_ty	*s;
{
	rpt_value_enumeration_ty *this;

	this = (rpt_value_enumeration_ty *)rpt_value_alloc(&method);
	this->value_n = n;
	this->value_s = str_copy(s);;
	return (rpt_value_ty *)this;
}
