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
 * MANIFEST: functions to manipulate void values
 */

#include <aer/value/void.h>


static rpt_value_method_ty method =
{
	sizeof(rpt_value_ty),
	"void",
	rpt_value_type_void,
	0, /* construct */
	0, /* destruct */
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
rpt_value_void()
{
	static rpt_value_ty *vp;

	if (!vp)
		vp =  rpt_value_alloc(&method);
	return rpt_value_copy(vp);
}
