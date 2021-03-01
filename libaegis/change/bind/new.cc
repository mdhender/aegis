//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2004 Peter Miller;
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
// MANIFEST: functions to manipulate news
//

#include <change.h>
#include <change/cstate/improve.h>
#include <error.h> // for assert
#include <trace.h>


void
change_bind_new(change_ty *cp)
{
	trace(("change_bind_new(cp = %08lX)\n{\n", (long)cp));
	assert(cp->reference_count >= 1);
	assert(!cp->cstate_data);
	cp->cstate_is_a_new_file = 1;
	cp->cstate_data = (cstate_ty *)cstate_type.alloc();
	cp->fstate_is_a_new_file = 1;
	cp->fstate_data = (fstate_ty *)fstate_type.alloc();
	change_cstate_improve(cp->cstate_data);
	trace(("}\n"));
}
