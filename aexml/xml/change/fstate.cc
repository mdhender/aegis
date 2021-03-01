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
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/fstate.fmtgen.h>

#include <aexml/xml/change/fstate.h>


xml_change_fstate::~xml_change_fstate()
{
}


xml_change_fstate::xml_change_fstate()
{
}


xml::pointer
xml_change_fstate::create()
{
    return pointer(new xml_change_fstate());
}


void
xml_change_fstate::report(change_identifier &cid, output::pointer op)
{
    trace(("xml_change_fstate::report()\n{\n"));

    //
    // We don't actually use change::fstate_get() because very soon it
    // is going to be broken up into one-file-per-file and this will no
    // longer exist.  So we synthesize it.
    //
    op->fputs("<fstate>\n<src>\n");
    for (size_t j = 0; ; ++j)
    {
        fstate_src_ty *src = change_file_nth(cid.get_cp(), j, view_path_first);
        if (!src)
            break;
        fstate_src_write_xml(op, "fstate_src", src);
    }
    op->fputs("</src>\n</fstate>\n");
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
