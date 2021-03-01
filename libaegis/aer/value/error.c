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
 * MANIFEST: functions to manipulate error values
 */

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


static void destruct _((rpt_value_ty *));

static void
destruct(vp)
	rpt_value_ty	*vp;
{
	rpt_value_error_ty *this;

	this = (rpt_value_error_ty *)vp;
	assert(this->method->type == rpt_value_type_error);
	if (this->pp)
		rpt_pos_free(this->pp);
	if (this->value)
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
	0, /* undefer */
};


rpt_value_ty *
rpt_value_error(pp, s)
	rpt_pos_ty	*pp;
	string_ty	*s;
{
	rpt_value_ty	*that;
	rpt_value_error_ty *this;

	that = rpt_value_alloc(&method);
	this = (rpt_value_error_ty *)that;
	this->pp = (pp ? rpt_pos_copy(pp) : 0);
	this->value = str_copy(s);
	return that;
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


void
rpt_value_error_print(vp)
	rpt_value_ty	*vp;
{
	sub_context_ty	*scp;
	rpt_value_error_ty *this;

	this = (rpt_value_error_ty *)vp;
	assert(vp->method == &method);
	scp = sub_context_new();
	sub_var_set_string(scp, "Message", this->value);
	rpt_pos_error(scp, this->pp, i18n("$message"));
	sub_context_delete(scp);
}


void
rpt_value_error_setpos(vp, pp)
	rpt_value_ty	*vp;
	rpt_pos_ty	*pp;
{
	rpt_value_error_ty *this;

	this = (rpt_value_error_ty *)vp;
	assert(vp->method == &method);
	if (!this->pp)
		this->pp = rpt_pos_copy(pp);
}
