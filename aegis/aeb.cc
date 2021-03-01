//
// aegis - project change supervisor
// Copyright (C) 1991-2009, 2011, 2012 Peter Miller
// Copyright (C) 2008, 2009 Walter Franzini
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
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>
#include <common/ac/errno.h>
#include <common/ac/libintl.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>
#include <common/ac/time.h>

#include <common/progname.h>
#include <common/quit.h>
#include <common/sizeof.h>
#include <common/str_list.h>
#include <common/trace.h>

#include <libaegis/ael/change/by_state.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/arglex2.h>
#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/col.h>
#include <libaegis/commit.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/log.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/search_path/base_get.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>

#include <aegis/aeb.h>

//
// NAME
//      build_usage
//
// SYNOPSIS
//      void build_usage(void);
//
// DESCRIPTION
//      The build_usage function is used to
//      briefly describe how to used the 'aegis -Build' command.
//

static void
build_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf(stderr, "usage: %s -Build [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -Build -List [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -Build -Help\n", progname);
    quit(1);
}


//
// NAME
//      build_help
//
// SYNOPSIS
//      void build_help(void);
//
// DESCRIPTION
//      The build_help function is used to
//      describe in detail how to use the 'aegis -Build' command.
//

static void
build_help(void)
{
    help("aeb", build_usage);
}


//
// NAME
//      build_list
//
// SYNOPSIS
//      void build_list(void);
//
// DESCRIPTION
//      The build_list function is used to
//      list the changes which may be built within the project.
//

static void
build_list(void)
{
    trace(("build_list()\n{\n"));
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(build_usage);
    list_changes_in_state_mask
    (
        cid,
        (
            (1 << cstate_state_being_developed)
        |
            (1 << cstate_state_being_integrated)
        )
    );
    trace(("}\n"));
}


//
// NAME
//      build_main
//
// SYNOPSIS
//      void build_main(void);
//
// DESCRIPTION
//      The build_main function is used to build a change in the "being
//      developed" or "being integrated" states.  It extracts what to
//      do from the command line.
//

static void
build_main(void)
{
    cstate_ty       *cstate_data;
    pconf_ty        *pconf_data;
    log_style_ty    log_style;
    string_ty       *s1;
    string_ty       *s2;
    bool            minimum;

    trace(("build_main()\n{\n"));
    change_identifier cid;
    arglex();
    log_style = log_style_snuggle_default;
    minimum = false;
    string_list_ty partial;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(build_usage);
            continue;

        case arglex_token_change:
        case arglex_token_number:
        case arglex_token_project:
            cid.command_line_parse(build_usage);
            continue;

        case arglex_token_file:
            if (arglex() != arglex_token_string)
                option_needs_files(arglex_token_file, build_usage);
            // fall through...

        case arglex_token_string:
            s2 = str_from_c(arglex_value.alv_string);
            partial.push_back(s2);
            str_free(s2);
            break;

        case arglex_token_nolog:
            if (log_style == log_style_none)
                duplicate_option(build_usage);
            log_style = log_style_none;
            break;

        case arglex_token_minimum:
            if (minimum)
                duplicate_option(build_usage);
            minimum = true;
            break;

        case arglex_token_wait:
        case arglex_token_wait_not:
            user_ty::lock_wait_argument(build_usage);
            break;

        case arglex_token_symbolic_links:
        case arglex_token_symbolic_links_not:
            user_ty::symlink_pref_argument(build_usage);
            break;

        case arglex_token_base_relative:
        case arglex_token_current_relative:
            user_ty::relative_filename_preference_argument(build_usage);
            break;
        }
        arglex();
    }
    cid.command_line_check(build_usage);

    //
    // Take an advisory write lock on this row of the change table.
    // Block if necessary.
    //
    // Also take a read lock on the baseline, to ensure that it does
    // not change (aeip) for the duration of the build.
    //
    if (!partial.nstrings)
        change_cstate_lock_prepare(cid.get_cp());
    project_baseline_read_lock_prepare(cid.get_pp());
    lock_take();
    cstate_data = cid.get_cp()->cstate_get();

    //
    // Extract the appropriate row of the change table.
    // It is an error if the change is not in one of the being_developed
    //     or being_integrated states.
    // It is an error if the change is not assigned to the current user.
    // It is an error if the change has no files assigned.
    //
    bool integrating = false;
    switch (cstate_data->state)
    {
    case cstate_state_awaiting_development:
    case cstate_state_awaiting_integration:
    case cstate_state_awaiting_review:
    case cstate_state_being_reviewed:
    case cstate_state_completed:
#ifndef DEBUG
    default:
#endif
        change_fatal(cid.get_cp(), 0, i18n("bad build state"));
        break;

    case cstate_state_being_developed:
        if (cid.get_cp()->is_a_branch())
            change_fatal(cid.get_cp(), 0, i18n("bad branch build"));
        if
        (
            nstring(cid.get_cp()->developer_name())
        !=
            cid.get_up()->name()
        )
            change_fatal(cid.get_cp(), 0, i18n("not developer"));
        if (!change_file_nth(cid.get_cp(), (size_t)0, view_path_first))
            change_fatal(cid.get_cp(), 0, i18n("no files"));
        break;

    case cstate_state_being_integrated:
        if
        (
            nstring(cid.get_cp()->integrator_name())
        !=
            cid.get_up()->name()
        )
            change_fatal(cid.get_cp(), 0, i18n("not integrator"));
        if (partial.nstrings)
            change_fatal(cid.get_cp(), 0, i18n("bad build, partial"));
        integrating = true;
        break;
    }

    if
    (
        !integrating
    &&
        (
            cid.get_cp()->run_project_file_command_needed()
        ||
            (!partial.nstrings && cid.get_cp()->file_promote())
        )
    )
    {
        //
        // Remember the fact that we are about to run the
        // project_file_command, but we actually run it outside the
        // locks, so that it can use the up-to-date lists of change
        // files and project files.
        //
        cid.get_cp()->run_project_file_command_done();

        //
        // Write out the file state, and then let go of the locks
        // and take them again.  This ensures the data is consistent
        // for the next stage of processing.
        //
        trace(("Write out what we've done so far.\n"));
        cid.get_cp()->cstate_write();
        commit();
        lock_release();

        //
        // Either the project files list changed, or the change's files
        // list changed (which can change the project files list seen
        // from this change) or both.
        //
        cid.get_cp()->run_project_file_command(cid.get_up());

        trace(("Take the locks again.\n"));
        change_cstate_lock_prepare(cid.get_cp());
        project_baseline_read_lock_prepare(cid.get_pp());
        lock_take();
    }
    cstate_data = cid.get_cp()->cstate_get();

    //
    // If no build is required, we stop here.
    // Note that this is *after* the symlinks have been repaired.
    //
    if (!change_build_required(cid.get_cp()))
    {
        change_verbose(cid.get_cp(), 0, i18n("no build required"));
        quit(0);
    }

    //
    // Resolve relative filenames into project filenames.
    // Do this after we know the change is in a buildable state.
    //
    if (partial.nstrings)
    {
        nstring_list search_path;
        size_t  j;
        size_t  k;

        //
        // Search path for resolving file names.
        //
        cid.get_cp()->search_path_get(search_path, true);

        //
        // Find the base for relative filenames.
        //
        nstring base = search_path_base_get(search_path, cid.get_up());

        //
        // resolve the path of each file
        // 1.   the absolute path of the file name is obtained
        // 2.   if the file is inside the development directory, ok
        // 3.   if the file is inside the baseline, ok
        // 4.   if neither, error
        //
        string_list_ty wl2;
        for (j = 0; j < partial.nstrings; ++j)
        {
            //
            // leave variable assignments alone
            //
            s1 = partial.string[j];
            if (strchr(s1->str_text, '='))
            {
                wl2.push_back(s1);
                continue;
            }

            //
            // resolve relative paths
            //
            if (s1->str_text[0] == '/')
                s2 = str_copy(s1);
            else
                s2 = os_path_join(base.get_ref(), s1);
            cid.get_up()->become_begin();
            s1 = os_pathname(s2, 0);
            cid.get_up()->become_end();
            str_free(s2);
            s2 = 0;
            for (k = 0; k < search_path.size(); ++k)
            {
                s2 = os_below_dir(search_path[k].get_ref(), s1);
                if (s2)
                    break;
            }
            str_free(s1);
            if (!s2)
            {
                sub_context_ty  *scp;

                scp = sub_context_new();
                sub_var_set_string(scp, "File_Name", partial.string[j]);
                change_fatal(cid.get_cp(), scp, i18n("$filename unrelated"));
                // NOTREACHED
                sub_context_delete(scp);
            }

            //
            // make sure it's unique
            //
            if (wl2.member(s2))
            {
                sub_context_ty  *scp;

                scp = sub_context_new();
                sub_var_set_string(scp, "File_Name", s2);
                change_fatal(cid.get_cp(), scp, i18n("too many $filename"));
                // NOTREACHED
                sub_context_delete(scp);
            }
            else
                wl2.push_back(s2);
            str_free(s2);
        }
        partial = wl2;
    }

    //
    // It is an error if the change attributes include architectures
    // not in the project.
    //
    change_check_architectures(cid.get_cp());

    //
    // Update the time the build was done.
    // This will not be written out if the build fails.
    //
    os_throttle();
    change_build_time_set(cid.get_cp());

    //
    // get the command to execute
    //  1. if the change is editing config, use that
    //  2. if the baseline contains config, use that
    //  3. error if can't find one (DON'T look for file existence)
    //
    pconf_data = change_pconf_get(cid.get_cp(), 1);

    //
    // If aeib had a -minimum, then aeb implicitly does
    //
    if
    (
        cstate_data->state == cstate_state_being_integrated
    &&
        cstate_data->minimum_integration
    )
    {
        minimum = true;
    }

    //
    // the program has changed, so it needs testing again,
    // so stomp on the validation fields.
    //
    trace(("nuke time stamps\n"));
    change_test_times_clear(cid.get_cp());

    //
    // do the build
    //
    trace(("open the log file\n"));
    trace(("do the build\n"));
    if (cstate_data->state == cstate_state_being_integrated)
    {
        user_ty::pointer pup = project_user(cid.get_pp());
        log_open(change_logfile_get(cid.get_cp()), pup, log_style);

        assert(pconf_data->integration_directory_style);
        work_area_style_ty style = *pconf_data->integration_directory_style;
        if (minimum || style.derived_at_start_only)
        {
            style.derived_file_link = false;
            style.derived_file_symlink = false;
            style.derived_file_copy = false;
        }
        change_create_symlinks_to_baseline(cid.get_cp(), pup, style);

        change_verbose(cid.get_cp(), 0, i18n("integration build started"));
        change_run_build_command(cid.get_cp());
        change_verbose(cid.get_cp(), 0, i18n("integration build complete"));

        change_remove_symlinks_to_baseline(cid.get_cp(), pup, style);
    }
    else
    {
        log_open(change_logfile_get(cid.get_cp()), cid.get_up(), log_style);

        assert(pconf_data->development_directory_style);
        work_area_style_ty style = *pconf_data->development_directory_style;
        if (minimum || style.derived_at_start_only)
        {
            style.derived_file_link = false;
            style.derived_file_symlink = false;
            style.derived_file_copy = false;
        }
        if
        (
            style.source_file_link
        ||
            style.source_file_symlink
        ||
            style.source_file_copy
        ||
            style.derived_file_link
        ||
            style.derived_file_symlink
        ||
            style.derived_file_copy
        )
        {
            bool verify_dflt =
                (
                    style.during_build_only
                ||
                    cid.get_cp()->run_project_file_command_needed()
                );
            if (cid.get_up()->symlink_pref(verify_dflt))
            {
                change_create_symlinks_to_baseline
                (
                    cid.get_cp(),
                    cid.get_up(),
                    style
                );
            }
        }

        if (partial.nstrings)
            change_verbose(cid.get_cp(), 0, i18n("partial build started"));
        else
            change_verbose(cid.get_cp(), 0, i18n("development build started"));

        change_run_development_build_command
        (
            cid.get_cp(),
            cid.get_up(),
            &partial
        );
        if (partial.nstrings)
            change_verbose(cid.get_cp(), 0, i18n("partial build complete"));
        else
            change_verbose(cid.get_cp(), 0, i18n("development build complete"));

        //
        // This looks unconditional.  The conditional logic is within
        // the function, rather than out here.
        //
        change_remove_symlinks_to_baseline(cid.get_cp(), cid.get_up(), style);
    }

    //
    // Update change data with result of build.
    // (This will be used when validating developer sign off.)
    // Release advisory write lock on row of change table.
    //
    if (!partial.nstrings)
    {
        change_file_list_metrics_check(cid.get_cp());
        cid.get_cp()->cstate_write();
        commit();
    }
    lock_release();
    trace(("}\n"));
}


//
// NAME
//      build
//
// SYNOPSIS
//      void build(void);
//
// DESCRIPTION
//      The build function is used to
//      dispatch the 'aegis -Build' command to the relevant functionality.
//      Where it goes depends on the command line.
//

void
build(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
        {arglex_token_help, build_help, 0 },
        {arglex_token_list, build_list, 0 },
    };

    trace(("build()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), build_main);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
