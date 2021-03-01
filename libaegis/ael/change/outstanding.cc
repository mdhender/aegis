//
// aegis - project change supervisor
// Copyright (C) 1999, 2003-2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/str.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/ael/change/by_state.h>
#include <libaegis/ael/change/inappropriat.h>
#include <libaegis/ael/change/outstanding.h>
#include <libaegis/change/identifier.h>
#include <libaegis/cstate.fmtgen.h>


void
list_outstanding_changes(change_identifier &cid, string_list_ty *args)
{
    trace(("list_outstanding_changes()\n{\n"));
    if (cid.set())
        list_change_inappropriate();
    list_changes_in_state_mask_by_user
    (
        cid,
        ~(1 << cstate_state_completed),
        (args->nstrings ? args->string[0] : 0)
    );
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
