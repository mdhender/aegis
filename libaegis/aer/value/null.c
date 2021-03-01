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
 * MANIFEST: functions to manipulate nul values
 */

#include <aer/value/boolean.h>
#include <aer/value/integer.h>
#include <aer/value/list.h>
#include <aer/value/null.h>
#include <aer/value/string.h>
#include <str.h>


static rpt_value_ty *arithmetic _((rpt_value_ty *));

static rpt_value_ty *
arithmetic(vp)
	rpt_value_ty	*vp;
{
	return rpt_value_integer(0);
}


static rpt_value_ty *stringize _((rpt_value_ty *));

static rpt_value_ty *
stringize(vp)
	rpt_value_ty	*vp;
{
	string_ty	*s;
	rpt_value_ty	*result;

	s = str_from_c("");
	result = rpt_value_string(s);
	str_free(s);
	return result;
}


static rpt_value_ty *booleanize _((rpt_value_ty *));

static rpt_value_ty *
booleanize(vp)
	rpt_value_ty	*vp;
{
	return rpt_value_boolean(0);
}


static rpt_value_ty *lookup _((rpt_value_ty *, rpt_value_ty *, int));

static rpt_value_ty *
lookup(vp, rhs, lvalue)
	rpt_value_ty	*vp;
	rpt_value_ty	*rhs;
	int		lvalue;
{
	return rpt_value_copy(vp);
}


static rpt_value_ty *keys _((rpt_value_ty *));

static rpt_value_ty *
keys(vp)
	rpt_value_ty	*vp;
{
	return rpt_value_list();
}


static rpt_value_ty *count _((rpt_value_ty *));

static rpt_value_ty *
count(vp)
	rpt_value_ty	*vp;
{
	return rpt_value_integer(0);
}


static rpt_value_method_ty method =
{
	sizeof(rpt_value_ty),
	"nul",
	rpt_value_type_nul,
	0, /* construct */
	0, /* destruct */
	arithmetic,
	stringize,
	booleanize,
	lookup,
	keys,
	count,
	0, /* type_of */
	0, /* undefer */
};


rpt_value_ty *
rpt_value_nul()
{
	static rpt_value_ty *vp;

	if (!vp)
		vp = rpt_value_alloc(&method);
	return rpt_value_copy(vp);
}
