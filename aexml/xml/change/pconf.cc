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

#include <aexml/xml/change/pconf.h>


xml_change_pconf::~xml_change_pconf()
{
}


xml_change_pconf::xml_change_pconf()
{
}


xml::pointer
xml_change_pconf::create()
{
    return pointer(new xml_change_pconf());
}


void
xml_change_pconf::report(change_identifier &cid, output::pointer op)
{
    trace(("xml_change_cstate()\n{\n"));

    //
    // Write out the change state data.
    //
    pconf_ty *pconf_data = change_pconf_get(cid.get_cp(), 0);
    pconf_write_xml(op, pconf_data);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
