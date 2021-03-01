//
//      aegis - project change supervisor
//      Copyright (C) 2004-2008, 2011, 2012 Peter Miller
//      Copyright (C) 2007, 2008 Walter Franzini
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

#include <common/error.h>
#include <common/nstring/list.h>
#include <common/symtab/template.h>
#include <common/trace.h>
#include <libaegis/ael/column_width.h>
#include <libaegis/arglex/project.h>
#include <libaegis/col.h>
#include <libaegis/help.h>
#include <libaegis/input/bunzip2.h>
#include <libaegis/input/file.h>
#include <libaegis/input/gunzip.h>
#include <libaegis/os.h>
#include <libaegis/output.h>
#include <libaegis/project.h>
#include <libaegis/project/invento_walk.h>
#include <libaegis/url.h>
#include <libaegis/user.h>

#include <aeannotate/usage.h>
#include <aedist/arglex3.h>
#include <aedist/change/functor/invent_build.h>
#include <aedist/missing.h>
#include <aedist/replay/line.h>

void
missing_main(void)
{
    string_ty *project_name = NULL;
    nstring ifn;
    nstring_list exclude_uuid_list;
    nstring_list include_uuid_list;
    nstring_list exclude_version_list;
    nstring_list include_version_list;
    bool all_changes = false;
    arglex();
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(usage);
            continue;

        case arglex_token_project:
            arglex();
            arglex_parse_project(&project_name, usage);
            continue;

        case arglex_token_file:
            if (!ifn.empty())
                duplicate_option(usage);
            switch (arglex())
            {
            default:
                option_needs_file(arglex_token_file, usage);
                // NOTREACHED

            case arglex_token_string:
                ifn = arglex_value.alv_string;
                break;

            case arglex_token_stdio:
                ifn = "";
                break;
            }
            break;

        case arglex_token_exclude_uuid:
            switch (arglex())
            {
            default:
                option_needs_string(arglex_token_exclude_uuid, usage);
                // NOTREACHED

            case arglex_token_string:
                exclude_uuid_list.push_back(arglex_value.alv_string);
                break;
            }
            break;

        case arglex_token_exclude_uuid_not:
            switch (arglex())
            {
            default:
                option_needs_string(arglex_token_exclude_uuid_not, usage);
                // NOTREACHED

            case arglex_token_string:
                include_uuid_list.push_back(arglex_value.alv_string);
                break;
            }
            break;

        case arglex_token_exclude_version:
            switch (arglex())
            {
            default:
                option_needs_string(arglex_token_exclude_version, usage);
                // NOTREACHED

            case arglex_token_string:
                exclude_version_list.push_back(arglex_value.alv_string);
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
                include_version_list.push_back(arglex_value.alv_string);
                break;
            }
            break;

        case arglex_token_maximum:
            all_changes = true;
            break;
        }
        arglex();
    }

    trace_nstring(ifn);

    //
    // locate project data
    //
    if (!project_name)
    {
        nstring n = user_ty::create()->default_project();
        project_name = n.get_ref_copy();
    }
    project *pp = project_alloc(project_name);
    pp->bind_existing();

    symtab<change> local_inventory;
    bool include_branches = true;
    bool ignore_original_uuid = false;
    change_functor_inventory_builder cf(include_branches, all_changes,
        ignore_original_uuid, pp, &local_inventory);
    project_inventory_walk(pp, cf);

    //
    // Parse the input file name to work out whether it is a file name
    // or a URL.  This is difficult, and made worse by the possibilities
    // that it could be a simple file name or a simple hostname.
    //
    url smart_url(ifn);
    if (smart_url.is_a_file())
        ifn = smart_url.get_path();
    else
    {
        smart_url.set_path_if_empty
        (
            nstring::format("cgi-bin/aeget/%s", project_name_get(pp).c_str())
        );
        smart_url.set_query_if_empty("inventory");
        ifn = smart_url.reassemble();
    }
    trace_nstring(ifn);

    //
    // Open the file (or URL) containing the inventory.
    //
    os_become_orig();
    input ifp = input_file_open(ifn.get_ref());
    ifp = input_bunzip2_open(ifp);
    ifp = input_gunzip_open(ifp);
    os_become_undo();

    //
    // Create the output columns.
    //
    col::pointer colp = col::open(0);
    string_ty *line1 =
        str_format
        (
            "Project \"%s\", Missing Change Set Inventory",
            project_name_get(pp).c_str()
        );
    colp->title(line1->str_text, ifn.c_str());
    str_free(line1);
    line1 = 0;

    int left = 0;
    output::pointer vers_col =
        colp->create(left, left + VERSION_WIDTH, "Change\n-------");
    left += VERSION_WIDTH + 1;
    output::pointer uuid_col =
        colp->create(left, left + UUID_WIDTH, "UUID\n------");
    left += UUID_WIDTH + 1;
    output::pointer desc_col =
        colp->create(left, 0, "Description\n------------");

    //
    // Fetch and list the remote change sets.
    //
    int n = 0;
    int r = 0;
    for (;;)
    {
        nstring line;
        os_become_orig();
        bool ok = ifp->one_line(line);
        os_become_undo();
        trace_nstring(line);
        if (!ok)
            break;

        replay_line parts;
        if (!parts.extract(line))
            continue;

        r++;

        if (local_inventory.query(parts.get_uuid()))
            continue;

        //
        // we exclude from further processing:
        // 1) UUIDs specified by --exclude-uuid, the match must be
        // exact;
        // 2) versions specified by --exclude-version, the match is
        // against a glob pattern.
        //
        if (exclude_uuid_list.member(parts.get_uuid()))
            continue;
        if (exclude_version_list.gmatch_candidate(parts.get_version()))
            continue;

        //
        // we exclude from further processing:
        // 1) UUIDs NOT specified by --include--uuid, the match must
        // be exact;
        // 2) versions NOT specified by --include-version, the match
        // is against a glob pattern.
        //
        if
        (
            !include_uuid_list.empty()
        &&
            !include_uuid_list.member(parts.get_uuid())
        )
            continue;
        if
        (
            !include_version_list.empty()
        &&
            !include_version_list.gmatch_candidate(parts.get_version())
        )
            continue;

        vers_col->fputs(parts.get_version());
        uuid_col->fputs(parts.get_uuid());
        desc_col->fputs(parts.get_description());
        colp->eoln();
        ++n;
    }
    uuid_col->fprintf("Remote change set%s: %d.", r == 1 ? "" : "s", r);
    colp->eoln();
    uuid_col->fprintf("Missing %d change set%s.", n, (n == 1 ? "" : "s"));
    colp->eoln();
}


// vim: set ts=8 sw=4 et :
