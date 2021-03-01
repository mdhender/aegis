//
// aegis - project change supervisor
// Copyright (C) 2003-2008, 2011, 2012 Peter Miller
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
#include <libaegis/ael/change/inappropriat.h>
#include <libaegis/change/identifier.h>
#include <libaegis/project.h>

#include <aexml/xml/project/state.h>


xml_project_state::~xml_project_state()
{
}


xml_project_state::xml_project_state()
{
}


xml::pointer
xml_project_state::create()
{
    return pointer(new xml_project_state());
}


void
xml_project_state::report(change_identifier &cid, output::pointer op)
{
    trace(("xml_project_pstate()\n{\n"));
    if (cid.set())
        list_change_inappropriate();

    project *ppp = cid.get_pp()->trunk_get();

    pstate_ty *pstate_data = ppp->pstate_get();
    pstate_write_xml(op, pstate_data);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
