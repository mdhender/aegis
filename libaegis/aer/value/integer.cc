//
//	aegis - project change supervisor
//	Copyright (C) 1994, 1996, 1999, 2003-2005 Peter Miller;
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
// MANIFEST: functions to manipulate integer values
//

#include <common/ac/limits.h>
#include <common/ac/math.h>

#include <libaegis/aer/value/boolean.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/real.h>
#include <libaegis/aer/value/string.h>
#include <common/error.h>
#include <common/str.h>


struct rpt_value_integer_ty
{
	RPT_VALUE
	long		value;
};


static rpt_value_ty *
stringize(rpt_value_ty *vp)
{
	rpt_value_integer_ty *this_thing;
	string_ty	*s;
	rpt_value_ty	*result;

	this_thing = (rpt_value_integer_ty *)vp;
	assert(this_thing->method->type == rpt_value_type_integer);
	s = str_format("%ld", this_thing->value);
	result = rpt_value_string(s);
	str_free(s);
	return result;
}


static rpt_value_ty *
booleanize(rpt_value_ty *vp)
{
	rpt_value_integer_ty *this_thing;

	this_thing = (rpt_value_integer_ty *)vp;
	assert(this_thing->method->type == rpt_value_type_integer);
	return rpt_value_boolean(this_thing->value != 0);
}


static rpt_value_method_ty method =
{
	sizeof(rpt_value_integer_ty),
	"integer",
	rpt_value_type_integer,
	0, // construct
	0, // destruct
	0, // arithmetic
	stringize,
	booleanize,
	0, // lookup
	0, // keys
	0, // count
	0, // type_of
	0, // undefer
};


rpt_value_ty *
rpt_value_integer(long n)
{
	rpt_value_integer_ty *this_thing;

	this_thing = (rpt_value_integer_ty *)rpt_value_alloc(&method);
	this_thing->value = n;
	return (rpt_value_ty *)this_thing;
}


long
rpt_value_integer_query(rpt_value_ty *vp)
{
	rpt_value_integer_ty *this_thing;

	this_thing = (rpt_value_integer_ty *)vp;
	assert(this_thing->method->type == rpt_value_type_integer);
	return this_thing->value;
}


rpt_value_ty *
rpt_value_integerize(rpt_value_ty *v1)
{
	rpt_value_ty	*v2;
	rpt_value_ty	*v3;
	double		n;

	v2 = rpt_value_arithmetic(v1);
	if (v2->method->type != rpt_value_type_real)
		return v2;
	n = floor(0.5 + rpt_value_real_query(v2));
	if (n < LONG_MIN || n > LONG_MAX)
		v3 = rpt_value_copy(v2);
	else
		v3 = rpt_value_integer((long)n);
	rpt_value_free(v2);
	return v3;
}
