//
// aegis - project change supervisor
// Copyright (C) 2007 Walter Franzini
// Copyright (C) 2008 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/error.h>       // for assert
#include <common/itab.h>
#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <libaegis/project.h>


time_t
project_ty::change_completion_timestamp(long change_number)
{
    assert(change2time_stp);
    time_t result = (time_t)itab_query(change2time_stp, change_number);
    if (!result)
    {
        change::pointer cp = change_alloc(this, change_number);
        change_bind_existing(cp);
        result = ::change_completion_timestamp(cp);
        itab_assign (change2time_stp, cp->number, (void*)result);
        change_free(cp);
    }

    return result;
}
