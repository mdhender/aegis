//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003, 2004 Peter Miller;
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
// MANIFEST: functions to manipulate adds
//

#include <change.h>
#include <error.h> // for assert


void
change_architecture_add(change_ty *cp, string_ty *name)
{
    cstate_ty       *cstate_data;
    type_ty         *type_p;
    string_ty       **who_p;

    assert(cp->reference_count >= 1);
    cstate_data = change_cstate_get(cp);
    if (!cstate_data->architecture)
    {
	cstate_data->architecture =
	    (cstate_architecture_list_ty *)
	    cstate_architecture_list_type.alloc();
    }
    who_p =
	(string_ty **)
	cstate_architecture_list_type.list_parse
	(
    	    cstate_data->architecture,
    	    &type_p
	);
    assert(type_p == &string_type);
    *who_p = str_copy(name);
}
