/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1996 Peter Miller;
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
 * MANIFEST: functions to manipulate time values
 */

#include <aer/value/boolean.h>
#include <aer/value/integer.h>
#include <aer/value/string.h>
#include <aer/value/time.h>
#include <error.h>
#include <str.h>


typedef struct rpt_value_time_ty rpt_value_time_ty;
struct rpt_value_time_ty
{
	RPT_VALUE
	time_t		value;
};


static rpt_value_ty *stringize _((rpt_value_ty *));

static rpt_value_ty *
stringize(vp)
	rpt_value_ty	*vp;
{
	rpt_value_time_ty *this;
	string_ty	*s;
	rpt_value_ty	*result;

	this = (rpt_value_time_ty *)vp;
	assert(this->method->type == rpt_value_type_time);
	s = str_format("%.24s", ctime(&this->value));
	result = rpt_value_string(s);
	str_free(s);
	return result;
}


static rpt_value_ty *arithmetic _((rpt_value_ty *));

static rpt_value_ty *
arithmetic(vp)
	rpt_value_ty	*vp;
{
	rpt_value_time_ty *this;

	this = (rpt_value_time_ty *)vp;
	assert(this->method->type == rpt_value_type_time);
	return rpt_value_integer((long)this->value);
}


static rpt_value_ty *booleanize _((rpt_value_ty *));

static rpt_value_ty *
booleanize(vp)
	rpt_value_ty	*vp;
{
	rpt_value_time_ty *this;

	this = (rpt_value_time_ty *)vp;
	assert(this->method->type == rpt_value_type_time);
	return rpt_value_boolean((long)this->value != 0);
}


static rpt_value_method_ty method =
{
	sizeof(rpt_value_time_ty),
	"time",
	rpt_value_type_time,
	0, /* construct */
	0, /* destruct */
	arithmetic,
	stringize,
	booleanize,
	0, /* lookup */
	0, /* keys */
	0, /* count */
	0, /* type_of */
	0, /* undefer */
};


rpt_value_ty *
rpt_value_time(n)
	time_t		n;
{
	rpt_value_time_ty *this;

	this = (rpt_value_time_ty *)rpt_value_alloc(&method);
	this->value = n;
	return (rpt_value_ty *)this;
}
