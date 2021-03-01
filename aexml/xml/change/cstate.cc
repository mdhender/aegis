//
// aegis - project change supervisor
// Copyright (C) 2003-2008, 2012 Peter Miller
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
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/trace.h>
#include <libaegis/change/identifier.h>

#include <aexml/xml/change/cstate.h>


xml_change_cstate::~xml_change_cstate()
{
}


xml_change_cstate::xml_change_cstate()
{
}


xml::pointer
xml_change_cstate::create()
{
    return pointer(new xml_change_cstate());
}


void
xml_change_cstate::report(change_identifier &cid, output::pointer op)
{
    trace(("xml_change_cstate::report()\n{\n"));

    //
    // Write out the change state data.
    //
    cstate_ty *cstate_data = cid.get_cp()->cstate_get();
    cstate_write_xml(op, cstate_data);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
