//
//	aegis - project change supervisor
//	Copyright (C) 2003-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/trace.h>
#include <libaegis/ael/change/inappropriat.h>
#include <libaegis/fstate.h>
#include <libaegis/output.h>
#include <libaegis/project/file.h>
#include <libaegis/project.h>
#include <libaegis/user.h>

#include <aexml/xml/project/fstate.h>


void
xml_project_fstate(struct string_ty *project_name, long change_number,
    output::pointer op)
{
    project_ty      *pp;
    size_t          j;

    trace(("xml_project_fstate()\n{\n"));
    if (change_number)
	    list_change_inappropriate();

    //
    // locate project data
    //
    if (!project_name)
    {
        nstring n = user_ty::create()->default_project();
	project_name = str_copy(n.get_ref());
    }
    else
	project_name = str_copy(project_name);
    pp = project_alloc(project_name);
    str_free(project_name);
    pp->bind_existing();

    //
    // We don't actually use project_fstate_get() because very soon it
    // is going to be broken up into one-file-per-file and this will no
    // longer exist.  So we synthesize it.
    //
    op->fputs("<fstate>\n<src>\n");
    for (j = 0; ; ++j)
    {
	fstate_src_ty   *src;

	src = pp->file_nth(j, view_path_simple);
	if (!src)
	    break;
	fstate_src_write_xml(op, "fstate_src", src);
    }
    op->fputs("</src>\n</fstate>\n");

    project_free(pp);
    trace(("}\n"));
}
