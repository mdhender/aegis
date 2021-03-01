//
//	aegis - project change supervisor
//	Copyright (C) 1994, 1996, 2003-2005 Peter Miller;
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
// MANIFEST: functions to manipulate time values
//

#include <libaegis/aer/value/boolean.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/aer/value/time.h>
#include <common/error.h>
#include <common/str.h>


struct rpt_value_time_ty
{
	RPT_VALUE
	time_t		value;
};


static rpt_value_ty *
stringize(rpt_value_ty *vp)
{
	rpt_value_time_ty *this_thing;
	string_ty	*s;
	rpt_value_ty	*result;

	this_thing = (rpt_value_time_ty *)vp;
	assert(this_thing->method->type == rpt_value_type_time);
	s = str_format("%.24s", ctime(&this_thing->value));
	result = rpt_value_string(s);
	str_free(s);
	return result;
}


static rpt_value_ty *
arithmetic(rpt_value_ty *vp)
{
	rpt_value_time_ty *this_thing;

	this_thing = (rpt_value_time_ty *)vp;
	assert(this_thing->method->type == rpt_value_type_time);
	return rpt_value_integer((long)this_thing->value);
}


static rpt_value_ty *
booleanize(rpt_value_ty *vp)
{
	rpt_value_time_ty *this_thing;

	this_thing = (rpt_value_time_ty *)vp;
	assert(this_thing->method->type == rpt_value_type_time);
	return rpt_value_boolean((long)this_thing->value != 0);
}


static rpt_value_method_ty method =
{
	sizeof(rpt_value_time_ty),
	"time",
	rpt_value_type_time,
	0, // construct
	0, // destruct
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
rpt_value_time(time_t n)
{
	rpt_value_time_ty *this_thing;

	this_thing = (rpt_value_time_ty *)rpt_value_alloc(&method);
	this_thing->value = n;
	return (rpt_value_ty *)this_thing;
}
