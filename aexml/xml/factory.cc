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

#include <common/ac/string.h>

#include <common/arglex.h>
#include <common/nstring/list.h>
#include <common/sizeof.h>
#include <common/trace.h>
#include <libaegis/change/identifier.h>
#include <libaegis/col.h>
#include <libaegis/option.h>
#include <libaegis/sub.h>

#include <aexml/xml/change/cstate.h>
#include <aexml/xml/change/fstate.h>
#include <aexml/xml/change/pconf.h>
#include <aexml/xml/project/cstate.h>
#include <aexml/xml/project/filesbydelta.h>
#include <aexml/xml/project/fstate.h>
#include <aexml/xml/project/list.h>
#include <aexml/xml/project/state.h>
#include <aexml/xml/user/uconf.h>


struct table_ty
{
    const char *name;
    const char *description;
    xml::pointer (*func)(void);
};


static const table_ty table[] =
{
    {
        "Change_Files",
        "Internal change file state.  See aefstate(5) for structure.",
        xml_change_fstate::create,
    },
    {
        "Change_State",
        "Internal change state.  See aecstate(5) for structure.",
        xml_change_cstate::create,
    },
    {
        "Project",
        "List of projects.  See aegstate(5) for structure.",
        xml_project_list::create,
    },
    {
        "Project_Change_State",
        "Internal project change state.  See aecstate(5) for structure.",
        xml_project_cstate::create,
    },
    {
        "Project_Config_File",
        "The project config file.  See aepconf(5) for structure.",
        xml_change_pconf::create,
    },
    {
        "Project_Files",
        "Internal project file state.  See aefstate(5) for structure.",
        xml_project_fstate::create,
    },
    {
        "Project_Files_By_Delta",
        "Historical project file state, immediately after an historical "
            "integrate pass.  See aefstate(5) for structure.",
        xml_project_files_by_delta::create,
    },
    {
        "Project_State",
        "Internal project state.  See aepstate(5) for structure.",
        xml_project_state::create,
    },
    {
        "User_Config_File",
        "The user config file.  See aeuconf(5) for structure.",
        xml_user_uconf::create,
    },
};


xml::pointer
xml::factory(const nstring &name)
{
    size_t nhit = 0;
    const table_ty *hit[SIZEOF(table)];
    for (const table_ty *tp = table; tp < ENDOF(table); ++tp)
    {
        if (arglex_compare(tp->name, name.c_str(), 0))
            hit[nhit++] = tp;
    }
    switch (nhit)
    {
    case 0:
        {
            sub_context_ty sc;
            sc.var_set_string("Name", name);
            sc.fatal_intl(i18n("no $name list"));
            // NOTREACHED
        }

    case 1:
        return hit[0]->func();

    default:
        {
            nstring_list names;
            for (size_t j = 0; j < nhit; ++j)
                names.push_back(hit[j]->name);
            sub_context_ty sc;
            sc.var_set_string("Name", name);
            sc.var_set_string("Name_List", names.unsplit(", "));
            sc.var_optional("Name_List");
            sc.fatal_intl(i18n("list $name ambiguous"));
            // NOTREACHED
        }
        break;
    }
}


void
xml::factory_list(output::pointer op)
{
    trace(("xml_list()\n{\n"));

    //
    // create the columns
    //
    col::pointer colp = col::open(op);
    colp->title("List of XML Lists", (const char *)0);
    int width = 15;
    for (const table_ty *tp = table; tp < ENDOF(table); ++tp)
    {
        size_t len = strlen(tp->name);
        if (width < int(len))
            width = len;
    }
    if (width > 31)
        width = 31;
    output::pointer name_col = colp->create(0, width, "Name\n------");
    output::pointer desc_col;
    if (!option_terse_get())
    {
        desc_col = colp->create(width + 1, 0, "Description\n-------------");
    }

    //
    // list the lists
    //
    for (const table_ty *tp = table; tp < ENDOF(table); ++tp)
    {
        name_col->fputs(tp->name);
        if (desc_col)
            desc_col->fputs(tp->description);
        colp->eoln();
    }
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
