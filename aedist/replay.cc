//
//	aegis - project change supervisor
//	Copyright (C) 2004 Walter Franzini;
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
// MANIFEST: implementation of the replay class
//

#include <ac/sys/types.h>
#include <ac/regex.h>
#include <ac/stdlib.h>

#include <arglex3.h>
#include <arglex/change.h>
#include <arglex/project.h>
#include <change.h>
#include <change/branch.h>
#include <change/functor/invent_build.h>
#include <change/lock_sync.h>
#include <error.h>              // for assert
#include <help.h>
#include <input.h>
#include <input/file.h>
#include <nstring.h>
#include <nstring/list.h>
#include <os.h>
#include <project.h>
#include <project/history.h>
#include <project/invento_walk.h>
#include <quit.h>
#include <replay/line.h>
#include <str.h>
#include <sub.h>
#include <symtab/template.h>
#include <url.h>
#include <usage.h>
#include <user.h>
#include <uuidentifier.h>
#include <trace.h>


void
replay_main(void)
{
    string_ty *project_name = NULL;
    nstring ifn;
    nstring trojan;
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

        case arglex_token_trojan:
            if (!trojan.empty())
                duplicate_option(usage);
            trojan = " --trojan";
            break;

        case arglex_token_trojan_not:
            if (!trojan.empty())
                duplicate_option(usage);
            trojan = " --no-trojan";
            break;

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

        case arglex_token_persevere:
	case arglex_token_persevere_not:
	    user_persevere_argument(usage);
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
    user_ty *up = user_executing(pp);

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

    nstring_list remote_change;
    for (;;)
    {
        os_become_orig();
        string_ty *line_p = input_one_line(ifp);
        os_become_undo();
        if (!line_p)
            break;
	nstring line(line_p);

	replay_line parts;
	if (!parts.extract(line))
            continue;

        change_ty *cp = local_inventory.query(parts.get_uuid());
        if (cp)
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

        remote_change.push_back(parts.get_url2());
    }

    trace(("remote_change.size() = %d;\n", remote_change.size()));

    os_become_orig();
    nstring dot = os_curdir();
    os_become_undo();

    //
    // Receive the changes:
    // 1) create an absolute URL to download the archive from
    // 2) run aedist -receive
    // 3) integrate the change, if possible.
    //
    for (size_t c = 0; c < remote_change.size(); ++c)
    {
        project_ty *pp2 = project_alloc(project_name);
        project_bind_existing(pp2);

        long change_number = project_next_change_number(pp2, 1);

        //
        // The URL as present in the change set inventory is not
        // absolute so we must create one.
        //
        nstring url_abs = remote_change[c];
	if (!url_abs.downcase().starts_with("http://"))
	{
	    url relative(url_abs);
	    relative.set_host_part_from(smart_url);
	    url_abs = relative.reassemble();
	}
        nstring aedist_cmd =
            nstring::format
            (
                "aedist -receive -project=%s -change=%ld -file '%s'%s",
                project_name->str_text,
                change_number,
                url_abs.c_str(),
                trojan.c_str()
            );
        trace_string(aedist_cmd.c_str());
        os_become_orig();
        int rc =
            os_execute_retcode
            (
                aedist_cmd.get_ref(),
                OS_EXEC_FLAG_INPUT,
                dot.get_ref()
            );
        os_become_undo();

        //
        // The project_pstate_get call is here to force a sync of the
        // pp pointed project_ty structure.
        //
        project_free(pp2);
        pp2 = 0;

        pp2 = project_alloc(project_name);
        project_bind_existing(pp2);
        change_ty *cp = change_alloc(pp2, change_number);
        int change_exists = change_bind_existing_errok(cp);

        trace_int(change_exists);
        if (!change_exists)
            continue;

        trace_int(rc);
        if (rc && !user_persevere_preference(up, 0))
            quit(1);

        assert(cp);
        cstate_ty *cstate_data = change_cstate_get(cp);
        assert(cstate_data);
        trace(("state = %s;\n", cstate_state_ename(cstate_data->state)));
        switch (cstate_data->state)
        {
        case cstate_state_awaiting_integration:
            break;

        case cstate_state_being_integrated:
        case cstate_state_completed:
            assert(0);
            // FALLTHROUGH

        case cstate_state_awaiting_development:
        case cstate_state_being_developed:
        case cstate_state_awaiting_review:
        case cstate_state_being_reviewed:
#ifndef DEBUG
        default:
#endif
            if (!user_persevere_preference(up, 0))
                quit(0);
	    continue;
        }

        assert(cstate_data->state == cstate_state_awaiting_integration);

        nstring aeintq_cmd =
            nstring::format
            (
                "aeintegratq -p %s -c %ld",
                project_name->str_text,
                change_number
            );

        trace_string(aeintq_cmd.c_str());
        os_become_orig();
        rc =
            os_execute_retcode
            (
                aeintq_cmd.get_ref(),
                OS_EXEC_FLAG_INPUT,
                dot.get_ref()
            );
        os_become_undo();

        change_lock_sync_forced(cp);
        cstate_data = change_cstate_get(cp);
        assert(cstate_data);
        trace(("state = %s;\n", cstate_state_ename(cstate_data->state)));
        switch (cstate_data->state)
        {
        case cstate_state_completed:
            break;

        case cstate_state_awaiting_development:
        case cstate_state_awaiting_review:
        case cstate_state_being_reviewed:
        case cstate_state_awaiting_integration:
        case cstate_state_being_integrated:
            assert(0);
            // FALLTHROUGH

        case cstate_state_being_developed:
#ifndef DEBUG
        default:
#endif
            if (!user_persevere_preference(up, 0))
                quit(0);
	    continue;
        }
        assert(cstate_data->state == cstate_state_completed);
        change_free(cp);
        cp = 0;
        project_free(pp2);
        pp2 = 0;
    }
}
