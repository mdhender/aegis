//
//	aegis - project change supervisor
//	Copyright (C) 1995-1999, 2001, 2003-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <libaegis/change/branch.h>
#include <common/error.h>


void
change_branch_new(change::pointer cp)
{
    cstate_ty       *cstate_data;

    cstate_data = cp->cstate_get();
    assert(!cstate_data->branch);
    cstate_data->cause = change_cause_internal_enhancement;
    cstate_data->branch = (cstate_branch_ty *)cstate_branch_type.alloc();
    cstate_data->branch->change =
        (cstate_branch_change_list_ty *)cstate_branch_change_list_type.alloc();
}
