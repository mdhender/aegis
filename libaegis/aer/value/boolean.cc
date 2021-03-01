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
// MANIFEST: functions to manipulate boolean values
//

#include <aer/value/boolean.h>
#include <aer/value/integer.h>
#include <aer/value/string.h>
#include <error.h>
#include <str.h>


struct rpt_value_boolean_ty
{
	RPT_VALUE
	int		value;
};


static rpt_value_ty *
arithmetic(rpt_value_ty *vp)
{
	rpt_value_boolean_ty *this_thing;
	rpt_value_ty	*result;

	this_thing = (rpt_value_boolean_ty *)vp;
	assert(this_thing->method->type == rpt_value_type_boolean);
	result = rpt_value_integer(this_thing->value);
	return result;
}


static rpt_value_ty *
stringize(rpt_value_ty *vp)
{
	rpt_value_boolean_ty *this_thing;
	string_ty	*s;
	rpt_value_ty	*result;

	this_thing = (rpt_value_boolean_ty *)vp;
	assert(this_thing->method->type == rpt_value_type_boolean);
	s = str_from_c(this_thing->value ? "true" : "false");
	result = rpt_value_string(s);
	str_free(s);
	return result;
}


static rpt_value_method_ty method =
{
	sizeof(rpt_value_boolean_ty),
	"boolean",
	rpt_value_type_boolean,
	0, // construct
	0, // destruct
	arithmetic,
	stringize,
	0, // booleanize
	0, // lookup
	0, // keys
	0, // count
	0, // type_of
};


rpt_value_ty *
rpt_value_boolean(int n)
{
	rpt_value_boolean_ty *this_thing;

	this_thing = (rpt_value_boolean_ty *)rpt_value_alloc(&method);
	this_thing->value = (n != 0);
	return (rpt_value_ty *)this_thing;
}


int
rpt_value_boolean_query(rpt_value_ty *vp)
{
	rpt_value_boolean_ty *this_thing;

	this_thing = (rpt_value_boolean_ty *)vp;
	assert(this_thing->method->type == rpt_value_type_boolean);
	return this_thing->value;
}
