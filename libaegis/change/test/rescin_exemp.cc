//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003-2008 Peter Miller
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

#include <libaegis/change.h>
#include <common/error.h> // for assert


void
change_rescind_test_exemption(change::pointer cp)
{
    cstate_ty       *cstate_data;

    assert(cp->reference_count >= 1);
    cstate_data = cp->cstate_get();
    cstate_data->test_exempt = false;
}


void
change_rescind_test_exemption_undo(change::pointer cp)
{
    cstate_ty       *cstate_data;

    assert(cp->reference_count >= 1);
    cstate_data = cp->cstate_get();
    cstate_data->test_exempt = cstate_data->given_test_exemption;
}
