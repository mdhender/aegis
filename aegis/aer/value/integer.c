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
 * MANIFEST: functions to manipulate integer values
 */

#include <ac/limits.h>
#include <math.h>

#include <aer/value/boolean.h>
#include <aer/value/integer.h>
#include <aer/value/real.h>
#include <aer/value/string.h>
#include <error.h>
#include <str.h>


typedef struct rpt_value_integer_ty rpt_value_integer_ty;
struct rpt_value_integer_ty
{
	RPT_VALUE
	long		value;
};


static rpt_value_ty *stringize _((rpt_value_ty *));

static rpt_value_ty *
stringize(vp)
	rpt_value_ty	*vp;
{
	rpt_value_integer_ty *this;
	string_ty	*s;
	rpt_value_ty	*result;

	this = (rpt_value_integer_ty *)vp;
	assert(this->method->type == rpt_value_type_integer);
	s = str_format("%ld", this->value);
	result = rpt_value_string(s);
	str_free(s);
	return result;
}


static rpt_value_ty *booleanize _((rpt_value_ty *));

static rpt_value_ty *
booleanize(vp)
	rpt_value_ty	*vp;
{
	rpt_value_integer_ty *this;

	this = (rpt_value_integer_ty *)vp;
	assert(this->method->type == rpt_value_type_integer);
	return rpt_value_boolean(this->value != 0);
}


static rpt_value_method_ty method =
{
	sizeof(rpt_value_integer_ty),
	"integer",
	rpt_value_type_integer,
	0, /* construct */
	0, /* destruct */
	0, /* arithmetic */
	stringize,
	booleanize,
	0, /* lookup */
	0, /* keys */
	0, /* count */
	0, /* type_of */
};


rpt_value_ty *
rpt_value_integer(n)
	long		n;
{
	rpt_value_integer_ty *this;

	this = (rpt_value_integer_ty *)rpt_value_alloc(&method);
	this->value = n;
	return (rpt_value_ty *)this;
}


long
rpt_value_integer_query(vp)
	rpt_value_ty	*vp;
{
	rpt_value_integer_ty *this;

	this = (rpt_value_integer_ty *)vp;
	assert(this->method->type == rpt_value_type_integer);
	return this->value;
}


rpt_value_ty *
rpt_value_integerize(v1)
	rpt_value_ty	*v1;
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
