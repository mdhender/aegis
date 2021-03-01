//
// aegis - project change supervisor
// Copyright (C) 2008, 2012 Peter Miller
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

#include <common/ac/assert.h>

#include <common/trace.h>
#include <common/nstring/list.h>
#include <libaegis/change/identifier.h>
#include <libaegis/file/event.h>
#include <libaegis/project/file/roll_forward.h>

#include <aexml/xml/project/filesbydelta.h>


xml_project_files_by_delta::~xml_project_files_by_delta()
{
}


xml_project_files_by_delta::xml_project_files_by_delta()
{
}


xml::pointer
xml_project_files_by_delta::create()
{
    return pointer(new xml_project_files_by_delta());
}


void
xml_project_files_by_delta::report(change_identifier &cid, output::pointer op)
{
    trace(("xml_project_files_by_dekta::report()\n{\n"));

    //
    // Do an historical reconstruction.
    //
    project_file_roll_forward *h = cid.get_historian();
    nstring_list filenames;
    h->keys(filenames);
    filenames.sort();

    //
    // List the last version of each file.
    //
    op->fputs("<fstate>\n<src>\n");
    for (size_t j = 0; j < filenames.size(); ++j)
    {
        nstring filename = filenames[j];
        file_event *fep = h->get_last(filename);
        assert(fep);
        fstate_src_write_xml(op, "fstate_src", fep->get_src());
    }
    op->fputs("</src>\n</fstate>\n");
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
