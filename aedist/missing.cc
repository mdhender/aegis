//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2005 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: implementation of the missing class
//

#include <ael/column_width.h>
#include <arglex3.h>
#include <arglex/project.h>
#include <change/functor/invent_build.h>
#include <col.h>
#include <error.h>
#include <help.h>
#include <input/file.h>
#include <missing.h>
#include <nstring/list.h>
#include <os.h>
#include <output.h>
#include <project.h>
#include <project/invento_walk.h>
#include <replay/line.h>
#include <symtab/template.h>
#include <trace.h>
#include <url.h>
#include <usage.h>
#include <user.h>


void
missing_main(void)
{
    string_ty *project_name = NULL;
    nstring ifn;
    nstring_list exclude_uuid_list;
    nstring_list include_uuid_list;
    nstring_list exclude_version_list;
    nstring_list include_version_list;
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
        }
        arglex();
    }

    trace_nstring(ifn);

    //
    // locate project data
    //
    if (!project_name)
        project_name = user_default_project();
    project_ty *pp = project_alloc(project_name);
    project_bind_existing(pp);

    symtab<change_ty> local_inventory;
    change_functor_inventory_builder cf(pp, &local_inventory);
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
	    nstring::format("/cgi-bin/aeget/%s", project_name->str_text)
	);
	smart_url.set_query_if_empty("inventory");
	ifn = smart_url.reassemble();
    }
    trace_nstring(ifn);

    //
    // Open the file (or URL) containing the inventory.
    //
    os_become_orig();
    input_ty *ifp = input_file_open(ifn.get_ref());
    os_become_undo();

    //
    // Create the output columns.
    //
    col_ty *colp = col_open(0);
    string_ty *line1 =
	str_format
	(
	    "Project \"%s\", Missing Change Set Inventory",
	    project_name_get(pp)->str_text
	);
    col_title(colp, line1->str_text, ifn.c_str());
    str_free(line1);
    line1 = 0;

    int left = 0;
    output_ty *vers_col =
	col_create(colp, left, left + VERSION_WIDTH, "Change\n-------");
    left += VERSION_WIDTH + 1;
    output_ty *uuid_col =
	col_create(colp, left, left + UUID_WIDTH, "UUID\n------");
    left += UUID_WIDTH + 1;
    output_ty *desc_col =
	col_create(colp, left, 0, "Description\n------------");

    //
    // Fetch and list the remote change sets.
    //
    int n = 0;
    for (;;)
    {
        os_become_orig();
        string_ty *line_p = input_one_line(ifp);
        os_become_undo();
        if (!line_p)
            break;
	nstring line(line_p);

        trace_nstring(line);

        replay_line parts;
	if (!parts.extract(line))
	    continue;

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
	col_eoln(colp);
	++n;
    }
    uuid_col->fprintf("Missing %d change set%s.", n, (n == 1 ? "" : "s"));
    col_eoln(colp);
    col_close(colp);
}
