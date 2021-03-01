//
// aegis - project change supervisor
// Copyright (C) 2003-2006, 2008, 2011, 2012 Peter Miller
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

#include <common/ac/string.h>

#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/ael/change/inappropriat.h>
#include <libaegis/ael/project/inappropriat.h>
#include <libaegis/change/identifier.h>
#include <libaegis/io.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>

#include <aexml/xml/project/list.h>


xml_project_list::~xml_project_list()
{
}


xml_project_list::xml_project_list()
{
}


xml::pointer
xml_project_list::create()
{
    return pointer(new xml_project_list());
}


void
xml_project_list::report(change_identifier &cid, output::pointer op)
{
    trace(("xml_project_list()\n{\n"));
    if (cid.project_set())
        list_project_inappropriate();
    if (cid.set())
        list_change_inappropriate();

    //
    // list the projects
    //
    string_list_ty name;
    project_list_get(&name);

    //
    // list each project
    //
    op->fputs("<gstate>\n<where>\n");
    for (size_t j = 0; j < name.nstrings; ++j)
    {
        op->fputs("<gstate_where>\n");
        project *pp = project_alloc(name.string[j]);
        pp->bind_existing();
        string_write_xml(op, "name", project_name_get(pp));

        int err = project_is_readable(pp);

        if (err)
        {
            op->fprintf("<error>%s</error>\n", strerror(err));
        }
        else
        {
            string_write_xml(op, "description", project_description_get(pp));

            //
            // The development directory of the project change is the
            // one which contains the trunk or branch baseline.
            //
            string_ty *top = project_top_path_get(pp, 0);
            string_write_xml(op, "directory", top);
        }
        project_free(pp);
        op->fputs("</gstate_where>\n");
    }
    op->fputs("</where>\n</gstate>\n");
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
