//
//      aegis - project change supervisor
//      Copyright (C) 2005-2008 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/error.h> // for assert
#include <common/nstring/list.h>
#include <common/quit.h>
#include <common/trace.h>
#include <libaegis/arglex2.h>
#include <libaegis/attribute.h>
#include <libaegis/change.h>
#include <libaegis/change/identifier.h>
#include <libaegis/help.h>
#include <libaegis/sub.h>

#include <aede-policy/policy.h>
#include <aede-policy/validation/list.h>
#include <aede-policy/usage.h>


static nstring
project_specific_find(change_identifier &cid, const char *name)
{
    pconf_ty *pconf_data = change_pconf_get(cid.get_cp(), 0);
    assert(pconf_data);
    attributes_ty *psp =
        attributes_list_find(pconf_data->project_specific, name);
    if (!psp)
        return 0;
    assert(psp->value);
    return nstring(psp->value);
}



void
policy()
{
    trace(("policy()\n{\n"));
    change_identifier chg;
    validation_list to_do;
    arglex();
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(usage);
            continue;

        case arglex_token_project:
        case arglex_token_baseline:
        case arglex_token_branch:
        case arglex_token_change:
        case arglex_token_delta:
        case arglex_token_delta_date:
        case arglex_token_delta_from_change:
        case arglex_token_grandparent:
        case arglex_token_number:
        case arglex_token_trunk:
            chg.command_line_parse(usage);
            continue;

        case arglex_token_string:
            to_do.push_back(validation::factory(arglex_value.alv_string));
            break;
        }
        arglex();
    }

    //
    // If they didn't specify anything in particular,
    // look in the project attributes.
    //
    if (to_do.empty())
    {
        nstring s = project_specific_find(chg, "aede-policy");
        if (!s.empty())
        {
            nstring_list names;
            names.split(s, " ");
            for (size_t j = 0; j < names.size(); ++j)
            {
                nstring name = names[j];
                to_do.push_back(validation::factory(name.c_str()));
            }
        }
    }

    //
    // If they didn't specify anything in particular,
    // just check that the descriptions have been edited.
    //
    if (to_do.empty())
        to_do.push_back(validation::factory("description"));

    //
    // Make sure the change is in the "being developed" state.
    // The aede-policy command makes no sense in any other state.
    //
    if (!chg.get_cp()->is_being_developed())
        change_fatal(chg.get_cp(), 0, i18n("bad de state"));

    //
    // Perform all of the validations.
    // If any of them fail, quit with a failure exit status.
    //
    bool ok = to_do.run(chg.get_cp());
    if (!ok)
        quit(1);
    trace(("}\n"));
}
