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
#include <libaegis/ael/change/inappropriat.h>
#include <libaegis/project.h>

#include <aexml/xml/project/fstate.h>


xml_project_fstate::~xml_project_fstate()
{
}


xml_project_fstate::xml_project_fstate()
{
}


xml::pointer
xml_project_fstate::create()
{
    return pointer(new xml_project_fstate());
}


void
xml_project_fstate::report(change_identifier &cid, output::pointer op)
{
    trace(("xml_project_fstate()\n{\n"));
    if (cid.set())
        list_change_inappropriate();

    //
    // We don't actually use project_fstate_get() because very soon it
    // is going to be broken up into one-file-per-file and this will no
    // longer exist.  So we synthesize it.
    //
    op->fputs("<fstate>\n<src>\n");
    for (size_t j = 0; ; ++j)
    {
        fstate_src_ty *src = cid.get_pp()->file_nth(j, view_path_simple);
        if (!src)
            break;
        fstate_src_write_xml(op, "fstate_src", src);
    }
    op->fputs("</src>\n</fstate>\n");
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
