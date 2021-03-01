//
// aegis - project change supervisor
// Copyright (C) 2007 Walter Franzini
// Copyright (C) 2008 Peter Miller
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

#include <common/error.h>
#include <common/str.h>
#include <common/nstring.h>
#include <common/nstring/list.h>

#include <libaegis/arglex/project.h>
#include <libaegis/change/functor.h>
#include <libaegis/help.h>
#include <libaegis/http.h>
#include <libaegis/option.h>
#include <libaegis/project/identifi_sub/plain.h>
#include <libaegis/project/invento_walk.h>

#include <aedist/archive.h>
#include <aedist/arglex3.h>
#include <aedist/change/functor/inventory.h>
#include <aedist/inventory.h>
#include <aedist/usage.h>

void
inventory_main(void)
{
    bool incl_orig = false;
    bool run_by_aeget = false;

    nstring prefix;
    nstring_list include_change;
    nstring_list exclude_change;
    project_identifier_subset_plain project;

    arglex();
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(usage);
            break;

        case arglex_token_aeget:
            run_by_aeget = true;
            break;

        case arglex_token_project:
            project.command_line_parse(usage);
            continue;

        case arglex_token_exclude_version:
            switch (arglex())
            {
            default:
                option_needs_string(arglex_token_exclude_version, usage);
                // NOTREACHED

            case arglex_token_string:
                exclude_change.push_back(arglex_value.alv_string);
                break;
            }
            break;

        case arglex_token_exclude_version_not:
            switch (arglex())
            {
            default:
                option_needs_string(arglex_token_exclude_version_not, usage);
                // NOTREACHED

            case arglex_token_string:
                include_change.push_back(arglex_value.alv_string);
                break;
            }
            break;

        case arglex_token_path_prefix_add:
            if (!prefix.empty())
                duplicate_option(usage);
            switch (arglex())
            {
            default:
                option_needs_string(arglex_token_path_prefix_add, usage);
                // NOTREACHED

            case arglex_token_string:
                prefix = nstring(arglex_value.alv_string);
                break;
            }
            break;

        case arglex_token_all:
            incl_orig = true;
            break;
        }
        arglex();
    }

    nstring generator;
    bool uuid_links = !run_by_aeget;

    change_functor_inventory
        cf
        (
            false,
            project.get_pp(),
            incl_orig,
            uuid_links,
            prefix,
            ARCHIVE_SUFFIX,
            include_change,
            exclude_change
        );
    project_inventory_walk(project.get_pp(), cf);
}
