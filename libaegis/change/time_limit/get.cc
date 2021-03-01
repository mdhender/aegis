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

#include <common/arglex.h>
#include <common/error.h>       // for assert
#include <common/trace.h>
#include <libaegis/change.h>
#include <libaegis/change/branch.h>


time_t
change::time_limit_get()
{
    trace(("change::time_limit_get()\n{\n"));
    if (is_completed())
    {
        trace(("return completion_timestamp;\n"));
        trace(("}\n"));
        return ::change_completion_timestamp(this);
    }

    trace(("return TIME_NOT_SET;\n"));
    trace(("}\n"));
    return TIME_NOT_SET;
}
