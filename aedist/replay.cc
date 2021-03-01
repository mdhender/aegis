//
//      aegis - project change supervisor
//      Copyright (C) 2005-2008, 2011, 2012 Peter Miller,
//      Copyright (C) 2004, 2005, 2007-2009 Walter Franzini;
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

#include <common/ac/assert.h>
#include <common/ac/ctype.h>
#include <common/ac/string.h>
#include <common/ac/sys/types.h>
#include <common/ac/regex.h>
#include <common/ac/stdlib.h>

#include <common/nstring.h>
#include <common/nstring/list.h>
#include <common/quit.h>
#include <common/str.h>
#include <common/symtab/template.h>
#include <common/trace.h>
#include <common/uuidentifier.h>
#include <common/version_stmp.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/change/branch.h>
#include <libaegis/change.h>
#include <libaegis/change/lock_sync.h>
#include <libaegis/help.h>
#include <libaegis/input/bunzip2.h>
#include <libaegis/input/file.h>
#include <libaegis/input/gunzip.h>
#include <libaegis/input.h>
#include <libaegis/option.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/project/invento_walk.h>
#include <libaegis/sub.h>
#include <libaegis/url.h>
#include <libaegis/user.h>

#include <aedist/usage.h>
#include <aedist/arglex3.h>
#include <aedist/change/functor/invent_build.h>
#include <aedist/replay/line.h>


static nstring
fix_compatibility_modifier(const nstring &uri, bool use_compat)
{
    trace(("fix_compatibility_modifier(uri = %s)\n{\n", uri.quote_c().c_str()));
    if
    (
        0 != memcmp("http:", uri.c_str(), 5)
    &&
        0 != memcmp("https:", uri.c_str(), 6)
    )
    {
        // Only mess with HTTP URLs
        trace(("return %s;\n", uri.quote_c().c_str()));
        trace(("}\n"));
        return uri;
    }
    if (!strstr(uri.c_str(), "/cgi-bin/"))
    {
        // Only add +compat=xxx to CGI scripts
        trace(("return %s;\n", uri.quote_c().c_str()));
        trace(("}\n"));
        return uri;
    }
    const char *cp = strstr(uri.c_str(), "compat=");
    if (!cp)
    {
        if (!use_compat)
            return uri;
        nstring result(uri + "+compat=" + version_stamp());
        trace(("return %s;\n", result.quote_c().c_str()));
        trace(("}\n"));
        return result;
    }
    cp += 7;
    nstring left(uri.c_str(), cp - uri.c_str());
    while (*cp && *cp != '+')
        ++cp;
    nstring result(left + version_stamp() + cp);
    trace(("return %s;\n", result.quote_c().c_str()));
    trace(("}\n"));
    return result;
}


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
    bool all_changes = false;
    bool use_compat = true;
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
            if (arglex() != arglex_token_string)
            {
                option_needs_file(arglex_token_file, usage);
                // NOTREACHED
            }
            ifn = arglex_value.alv_string;
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
            user_ty::persevere_argument(usage);
            break;

        case arglex_token_maximum:
            all_changes = true;
            break;

        case arglex_token_compatibility_not:
            use_compat = false;
        }
        arglex();
    }

    trace_nstring(ifn);
    if (ifn.empty())
        option_needs_url(arglex_token_file, usage);

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
    user_ty::pointer up = user_ty::create();

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

    nstring_list remote_change;
    for (;;)
    {
        nstring line;
        os_become_orig();
        bool ok = ifp->one_line(line);
        os_become_undo();
        if (!ok)
            break;

        replay_line parts;
        if (!parts.extract(line))
            continue;

        change::pointer cp = local_inventory.query(parts.get_uuid());
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

        remote_change.push_back_unique(parts.get_url2());
    }

    trace(("remote_change.size() = %zd;\n", remote_change.size()));

    os_become_orig();
    nstring dot(os_curdir());
    os_become_undo();

    //
    // Receive the changes:
    // 1) create an absolute URL to download the archive from
    // 2) run aedist -receive
    // 3) integrate the change, if possible.
    //
    for (size_t c = 0; c < remote_change.size(); ++c)
    {
        project *pp2 = project_alloc(project_name);
        pp2->bind_existing();

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

        //
        // There could be a compat=n.nn modifier in the
        // URL, if so replace it, otherwise add one.
        //
        url_abs = fix_compatibility_modifier(url_abs, use_compat);

        //
        // Start building the command.
        //
        nstring trace_options(trace_args());
        nstring change_number_file(os_edit_filename(0));
        nstring aedist_cmd =
            nstring::format
            (
                "aedist -receive -project=%s -output=%s -file %s%s%s%s",
                nstring(project_name_get(pp2)).quote_shell().c_str(),
                change_number_file.c_str(),
                url_abs.quote_shell().c_str(),
                trojan.c_str(),
                trace_options.c_str(),
                (option_verbose_get() ? " --verbose" : "")
            );
        trace_nstring(aedist_cmd);
        os_become_orig();
        int rc =
            os_execute_retcode
            (
                aedist_cmd.get_ref(),
                OS_EXEC_FLAG_INPUT,
                dot.get_ref()
            );
        os_become_undo();

        project_free(pp2);
        pp2 = 0;

        trace_int(rc);
        if (rc && !up->persevere_preference(false))
            quit(1);

        pp2 = project_alloc(project_name);
        pp2->bind_existing();

        //
        // Read the change number from the file, there is no more need
        // to guess the right value in advance since it's now written
        // by aedist -rec itself.
        //
        os_become_orig();
        input ifp2 = input_file_open(change_number_file, true);
        nstring s;
        if (!ifp2->one_line(s) || s.empty())
            ifp2->fatal_error("short file");
        ifp2.close();
        os_become_undo();

        long change_number = s.to_long();

        change::pointer cp = change_alloc(pp2, change_number);
        int change_exists = change_bind_existing_errok(cp);

        trace_int(change_exists);
        if (!change_exists)
            continue;

        assert(cp);
        cstate_ty *cstate_data = cp->cstate_get();
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
            if (!up->persevere_preference(false))
                quit(0);
            continue;
        }

        assert(cstate_data->state == cstate_state_awaiting_integration);

        nstring aeintq_cmd =
            nstring::format
            (
                "aeintegratq -p %s -c %ld",
                nstring(project_name_get(pp2)).quote_shell().c_str(),
                change_number
            );

        trace_nstring(aeintq_cmd);
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
        cstate_data = cp->cstate_get();
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
        case cstate_state_being_developed:
#ifndef DEBUG
        default:
#endif
            if (!up->persevere_preference(false))
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


// vim: set ts=8 sw=4 et :
