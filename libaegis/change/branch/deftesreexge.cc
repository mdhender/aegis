//
//	aegis - project change supervisor
//	Copyright (C) 2005-2007 Peter Miller
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
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <libaegis/change/branch.h>
#include <common/error.h> // for assert


bool
change_branch_default_test_regression_exemption_get(change::pointer cp)
{
    cstate_ty *cstate_data = cp->cstate_get();
    cstate_branch_ty *bp = cstate_data->branch;
    assert(bp);
    return bp->default_test_regression_exemption;
}