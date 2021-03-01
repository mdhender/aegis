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
#include <libaegis/change.h>
#include <libaegis/cstate.h>
#include <libaegis/output.h>
#include <libaegis/project.h>
#include <libaegis/user.h>

#include <aexml/xml/change/pconf.h>


void
xml_change_pconf(string_ty *project_name, long change_number,
    output::pointer op)
{
    project_ty      *pp;
    user_ty::pointer up;
    change::pointer cp;
    pconf_ty        *pconf_data;

    trace(("xml_change_cstate()\n{\n"));

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
    // locate user data
    //
    up = user_ty::create();

    //
    // locate change data
    //
    if (!change_number)
	change_number = up->default_change(pp);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    //
    // Write out the change state data.
    //
    pconf_data = change_pconf_get(cp, 0);
    pconf_write_xml(op, pconf_data);

    //
    // Clean up and go home.
    //
    change_free(cp);
    project_free(pp);
    trace(("}\n"));
}
