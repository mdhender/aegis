//
//	aegis - project change supervisor
//	Copyright (C) 1994-1996, 1999, 2003, 2004 Peter Miller;
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
// MANIFEST: functions to manipulate error values
//

#include <aer/pos.h>
#include <aer/value/error.h>
#include <error.h>
#include <sub.h>


typedef struct rpt_value_error_ty rpt_value_error_ty;
struct rpt_value_error_ty
{
	RPT_VALUE
	rpt_pos_ty	*pp;
	string_ty	*value;
};


static void
destruct(rpt_value_ty *vp)
{
	rpt_value_error_ty *this_thing;

	this_thing = (rpt_value_error_ty *)vp;
	assert(this_thing->method->type == rpt_value_type_error);
	if (this_thing->pp)
		rpt_pos_free(this_thing->pp);
	if (this_thing->value)
		str_free(this_thing->value);
}


static rpt_value_method_ty method =
{
	sizeof(rpt_value_error_ty),
	"error",
	rpt_value_type_error,
	0, // construct
	destruct,
	0, // arithmetic
	0, // stringize
	0, // booleanize
	0, // lookup
	0, // keys
	0, // count
	0, // type_of
	0, // undefer
};


rpt_value_ty *
rpt_value_error(rpt_pos_ty *pp, string_ty *s)
{
	rpt_value_ty	*that;
	rpt_value_error_ty *this_thing;

	that = rpt_value_alloc(&method);
	this_thing = (rpt_value_error_ty *)that;
	this_thing->pp = (pp ? rpt_pos_copy(pp) : 0);
	this_thing->value = str_copy(s);
	return that;
}


string_ty *
rpt_value_error_query(rpt_value_ty *vp)
{
	rpt_value_error_ty *this_thing;

	this_thing = (rpt_value_error_ty *)vp;
	assert(vp->method == &method);
	return this_thing->value;
}


void
rpt_value_error_print(rpt_value_ty *vp)
{
	sub_context_ty	*scp;
	rpt_value_error_ty *this_thing;

	this_thing = (rpt_value_error_ty *)vp;
	assert(vp->method == &method);
	scp = sub_context_new();
	sub_var_set_string(scp, "Message", this_thing->value);
	rpt_pos_error(scp, this_thing->pp, i18n("$message"));
	sub_context_delete(scp);
}


void
rpt_value_error_setpos(rpt_value_ty *vp, rpt_pos_ty *pp)
{
	rpt_value_error_ty *this_thing;

	this_thing = (rpt_value_error_ty *)vp;
	assert(vp->method == &method);
	if (!this_thing->pp)
		this_thing->pp = rpt_pos_copy(pp);
}
