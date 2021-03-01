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
// MANIFEST: functions to manipulate nul values
//

#include <libaegis/aer/value/boolean.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/list.h>
#include <libaegis/aer/value/null.h>
#include <libaegis/aer/value/string.h>
#include <common/str.h>


static rpt_value_ty *
arithmetic(rpt_value_ty *vp)
{
	return rpt_value_integer(0);
}


static rpt_value_ty *
stringize(rpt_value_ty *vp)
{
	string_ty	*s;
	rpt_value_ty	*result;

	s = str_from_c("");
	result = rpt_value_string(s);
	str_free(s);
	return result;
}


static rpt_value_ty *
booleanize(rpt_value_ty *vp)
{
	return rpt_value_boolean(0);
}


static rpt_value_ty *
lookup(rpt_value_ty *vp, rpt_value_ty *rhs, int lvalue)
{
	return rpt_value_copy(vp);
}


static rpt_value_ty *
keys(rpt_value_ty *vp)
{
	return rpt_value_list();
}


static rpt_value_ty *
count(rpt_value_ty *vp)
{
	return rpt_value_integer(0);
}


static rpt_value_method_ty method =
{
	sizeof(rpt_value_ty),
	"nul",
	rpt_value_type_nul,
	0, // construct
	0, // destruct
	arithmetic,
	stringize,
	booleanize,
	lookup,
	keys,
	count,
	0, // type_of
	0, // undefer
};


rpt_value_ty *
rpt_value_nul()
{
	static rpt_value_ty *vp;

	if (!vp)
		vp = rpt_value_alloc(&method);
	return rpt_value_copy(vp);
}
