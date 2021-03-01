//
//	aegis - project change supervisor
//	Copyright (C) 1994, 1996, 2003, 2004 Peter Miller;
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
// MANIFEST: functions to manipulate enumeration values
//

#include <aer/value/boolean.h>
#include <aer/value/enum.h>
#include <aer/value/integer.h>
#include <aer/value/string.h>
#include <error.h>
#include <str.h>


struct rpt_value_enumeration_ty
{
	RPT_VALUE
	long		value_n;
	string_ty	*value_s;
};


static void
destruct(rpt_value_ty *vp)
{
	rpt_value_enumeration_ty *this_thing;

	this_thing = (rpt_value_enumeration_ty *)vp;
	assert(this_thing->method->type == rpt_value_type_enumeration);
	str_free(this_thing->value_s);
}


static rpt_value_ty *
stringize(rpt_value_ty *vp)
{
	rpt_value_enumeration_ty *this_thing;

	this_thing = (rpt_value_enumeration_ty *)vp;
	assert(this_thing->method->type == rpt_value_type_enumeration);
	return rpt_value_string(this_thing->value_s);
}


static rpt_value_ty *
arithmetic(rpt_value_ty *vp)
{
	rpt_value_enumeration_ty *this_thing;

	this_thing = (rpt_value_enumeration_ty *)vp;
	assert(this_thing->method->type == rpt_value_type_enumeration);
	return rpt_value_integer(this_thing->value_n);
}


static rpt_value_ty *
booleanize(rpt_value_ty *vp)
{
	rpt_value_enumeration_ty *this_thing;

	this_thing = (rpt_value_enumeration_ty *)vp;
	assert(this_thing->method->type == rpt_value_type_enumeration);
	return rpt_value_boolean(this_thing->value_n != 0);
}


static rpt_value_method_ty method =
{
	sizeof(rpt_value_enumeration_ty),
	"enumeration",
	rpt_value_type_enumeration,
	0, // construct
	destruct,
	arithmetic,
	stringize,
	booleanize,
	0, // lookup
	0, // keys
	0, // count
	0, // type_of
	0, // undefer
};


rpt_value_ty *
rpt_value_enumeration(long n, string_ty *s)
{
	rpt_value_enumeration_ty *this_thing;

	this_thing = (rpt_value_enumeration_ty *)rpt_value_alloc(&method);
	this_thing->value_n = n;
	this_thing->value_s = str_copy(s);;
	return (rpt_value_ty *)this_thing;
}
