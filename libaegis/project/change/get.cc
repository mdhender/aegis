//
//      aegis - project change supervisor
//      Copyright (C) 2006-2008, 2011, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/trace.h>
#include <libaegis/change.h>
#include <libaegis/project.h>


change::pointer
project::change_get()
{
    trace(("project::change_get(this = %p)\n{\n", this));
    //
    // It could be an old project.  Make sure the pstate is read in,
    // and converted if necessary.
    //
    if (!parent)
        pstate_get();

    //
    // If the project change is not yet bound, bind to the trunk
    // change.  It will already exist.
    //
    if (!pcp)
    {
        pcp = change_alloc(this, TRUNK_CHANGE_NUMBER);
        change_bind_existing(pcp);
    }
    assert(pcp->reference_count >= 1);
    trace(("return %p;\n}\n", pcp));
    return pcp;
}


// vim: set ts=8 sw=4 et :
