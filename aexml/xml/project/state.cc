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

#include <libaegis/ael/change/inappropriat.h>
#include <libaegis/output.h>
#include <libaegis/project.h>
#include <common/trace.h>
#include <libaegis/user.h>
#include <aexml/xml/project/state.h>


void
xml_project_pstate(string_ty *project_name, long change_number,
    output::pointer op)
{
    project_ty      *pp;
    project_ty      *ppp;
    pstate_ty       *pstate_data;

    trace(("xml_project_pstate()\n{\n"));
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

    ppp = pp->trunk_get();

    pstate_data = ppp->pstate_get();
    pstate_write_xml(op, pstate_data);
    project_free(pp);
    trace(("}\n"));
}
