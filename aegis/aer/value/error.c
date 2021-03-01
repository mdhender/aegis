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
 * MANIFEST: functions to manipulate error values
 */

#include <aer/value/error.h>
#include <error.h>
#include <str.h>


typedef struct rpt_value_error_ty rpt_value_error_ty;
struct rpt_value_error_ty
{
	RPT_VALUE
	string_ty	*value;
};


static void destruct _((rpt_value_ty *));

static void
destruct(vp)
	rpt_value_ty	*vp;
{
	rpt_value_error_ty *this;

	this = (rpt_value_error_ty *)vp;
	assert(this->method->type == rpt_value_type_error);
	str_free(this->value);
}


static rpt_value_method_ty method =
{
	sizeof(rpt_value_error_ty),
	"error",
	rpt_value_type_error,
	0, /* construct */
	destruct,
	0, /* arithmetic */
	0, /* stringize */
	0, /* booleanize */
	0, /* lookup */
	0, /* keys */
	0, /* count */
	0, /* type_of */
};


rpt_value_ty *
rpt_value_error(fmt sva_last)
	char		*fmt;
	sva_last_decl
{
	va_list		ap;
	string_ty	*s;
	rpt_value_error_ty *this;

	sva_init(ap, fmt);
	s = str_vformat(fmt, ap);
	va_end(ap);

	this = (rpt_value_error_ty *)rpt_value_alloc(&method);
	this->value = s;
	return (rpt_value_ty *)this;
}


string_ty *
rpt_value_error_query(vp)
	rpt_value_ty	*vp;
{
	rpt_value_error_ty *this;

	this = (rpt_value_error_ty *)vp;
	assert(vp->method == &method);
	return this->value;
}
