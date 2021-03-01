//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003-2006 Peter Miller;
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
// MANIFEST: functions to manipulate filename_gets
//

#include <libaegis/change.h>
#include <common/error.h> // for assert
#include <libaegis/project.h>


string_ty *
change_cstate_filename_get(change_ty *cp)
{
    assert(cp->reference_count >= 1);
    if (!cp->cstate_filename)
       	cp->cstate_filename = cp->pp->change_path_get(cp->number);
    return cp->cstate_filename;
}
