//
//	aegis - project change supervisor
//	Copyright (C) 2005, 2006 Walter Franzini;
//	Copyright (C) 2004-2007 Peter Miller
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
// MANIFEST: implementation of the pending class
//

#include <libaegis/ael/column_width.h>
#include <aedist/arglex3.h>
#include <libaegis/arglex/project.h>
#include <aedist/change/functor/pendin_print.h>
#include <libaegis/col.h>
#include <common/error.h>
#include <libaegis/help.h>
#include <libaegis/input/file.h>
#include <aedist/missing.h>
#include <common/nstring/list.h>
#include <libaegis/os.h>
#include <libaegis/output.h>
#include <libaegis/project.h>
#include <libaegis/project/invento_walk.h>
#include <aedist/replay/line.h>
#include <common/symtab/template.h>
#include <common/trace.h>
#include <libaegis/url.h>
#include <aeannotate/usage.h>
#include <libaegis/user.h>


void
pending_main(void)
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
	project_name = str_copy(n.get_ref());
    }
    project_ty *pp = project_alloc(project_name);
    pp->bind_existing();

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
	    nstring::format("/cgi-bin/aeget/%s", project_name_get(pp)->str_text)
	);
	smart_url.set_query_if_empty("inventory+all");
	ifn = smart_url.reassemble();
    }
    trace_nstring(ifn);

    //
    // Open the file (or URL) containing the inventory.
    //
    os_become_orig();
    input ifp = input_file_open(ifn.get_ref());
    os_become_undo();

    //
    // Fetch and accumulate the remote change sets.
    //
    symtab<nstring> remote_inventory;
    for (;;)
    {
	nstring line;
        os_become_orig();
        bool ok = ifp->one_line(line);
        os_become_undo();
        if (!ok)
            break;
        trace_nstring(line);

        replay_line parts;
	if (!parts.extract(line))
	    continue;

        remote_inventory.assign(parts.get_uuid(), parts.get_version());
    }

    //
    // Print the pending inventory.
    //
    change_functor_pending_printer
        cf
        (
	    false,
            pp,
            ifn,
            &remote_inventory,
            &include_uuid_list,
            &include_version_list,
            &exclude_uuid_list,
            &exclude_version_list
        );
    project_inventory_walk(pp, cf);
}
