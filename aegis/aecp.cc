//
// aegis - project change supervisor
// Copyright (C) 1991-1999, 2001-2009, 2011, 2012 Peter Miller
// Copyright (C) 2007-2011 Walter Franzini
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
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/unistd.h>
#include <common/ac/libintl.h>

#include <common/gettime.h>
#include <common/now.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/sizeof.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/ael/project/files.h>
#include <libaegis/arglex2.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/commit.h>
#include <libaegis/file/event.h>
#include <libaegis/file.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/log.h>
#include <libaegis/os.h>
#include <libaegis/project/file.h>
#include <libaegis/project/file/roll_forward.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/search_path/base_get.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>

#include <aegis/aecp.h>


static void
copy_file_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
        stderr,
        "usage: %s -CoPy_file [ <option>... ] <filename>...\n",
        progname
    );
    fprintf
    (
        stderr,
        "       %s -CoPy_file -INDependent [ <option>... ] <filename>...\n",
        progname
    );
    fprintf(stderr, "       %s -CoPy_file -List [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -CoPy_file -Help\n", progname);
    quit(1);
}


static void
copy_file_help(void)
{
    help("aecp", copy_file_usage);
}


static void
copy_file_list(void)
{
    trace(("copy_file_list()\n{\n"));
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(copy_file_usage);
    list_project_files(cid, 0);
    trace(("}\n"));
}


#define NO_TIME_SET ((time_t)(-1))


static void
copy_file_independent(void)
{
    trace(("copy_file_independent()\n{\n"));
    arglex();
    nstring_list filenames;
    change_identifier cid;
    nstring output_filename;
    nstring output_directory;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(copy_file_usage);
            continue;

        case arglex_token_output_directory:
            if (arglex() != arglex_token_string)
            {
                option_needs_dir
                (
                    arglex_token_output_directory,
                    copy_file_usage
                );
            }
            if (!output_directory.empty())
            {
                duplicate_option_by_name
                (
                    arglex_token_output_directory,
                    copy_file_usage
                );
            }
            output_directory = arglex_value.alv_string;
            break;

        case arglex_token_directory:
            if (arglex() != arglex_token_string)
                option_needs_dir(arglex_token_directory, copy_file_usage);
            goto get_file_names;

        case arglex_token_file:
            if (arglex() != arglex_token_string)
                option_needs_files(arglex_token_file, copy_file_usage);
            // fall through...

        case arglex_token_string:
            get_file_names:
            {
                nstring s2(arglex_value.alv_string);
                filenames.push_back(s2);
            }
            break;

        case arglex_token_baseline:
        case arglex_token_branch:
        case arglex_token_change:
        case arglex_token_delta:
        case arglex_token_delta_date:
        case arglex_token_delta_from_change:
        case arglex_token_number:
        case arglex_token_project:
        case arglex_token_trunk:
            cid.command_line_parse(copy_file_usage);
            continue;

        case arglex_token_base_relative:
        case arglex_token_current_relative:
            user_ty::relative_filename_preference_argument(copy_file_usage);
            break;

        case arglex_token_output:
            if (!output_filename.empty())
                duplicate_option(copy_file_usage);
            switch (arglex())
            {
            default:
                option_needs_file(arglex_token_output, copy_file_usage);

            case arglex_token_stdio:
                output_filename = "-";
                break;

            case arglex_token_string:
                output_filename = arglex_value.alv_string;
                break;
            }
            break;
        }
        arglex();
    }
    cid.command_line_check(copy_file_usage);
    if (!output_directory.empty() && !output_filename.empty())
    {
        mutually_exclusive_options
        (
            arglex_token_output_directory,
            arglex_token_output,
            copy_file_usage
        );
    }
    if (filenames.empty())
    {
        // FIXME: given the the most probable file list is "."
        // should we accept no files to imply "."?
        error_intl(0, i18n("no file names"));
        copy_file_usage();
    }

    //
    // In order to behave as users expect, we need the original umask
    // when Aegis was invoked.
    //
    int original_umask = 022;
    os_become_orig_query(0, 0, &original_umask);

    //
    // Take a read lock on the baseline, to ensure that it does
    // not change (aeip) for the duration of the copy.
    //
    if (output_filename.empty())
    {
        project_baseline_read_lock_prepare(cid.get_pp());
        lock_take();
    }

    //
    // All of the file validations are to be done from the historical
    // perspective, not the branch head revision.
    //
    if (!cid.set() || cid.get_baseline())
        cid.set_delta_from_baseline();
    project_file_roll_forward *historian = cid.get_historian();

    //
    // build the list of places to look
    // when resolving the file name
    //
    // To cope with automounters, directories are stored as given,
    // or are derived from the home directory in the passwd file.
    // Within aegis, pathnames have their symbolic links resolved,
    // and any comparison of paths is done on this "system idea"
    // of the pathname.
    //
    if (output_directory.empty())
    {
        os_become_orig();
        output_directory = nstring(os_curdir());
        os_become_undo();
    }
    nstring_list search_path;
    cid.get_pp()->search_path_get(search_path, true);
    assert(!search_path.empty());

    //
    // Find the base for relative filenames.
    //
    bool base_relative =
        (
            cid.get_up()->relative_filename_preference
            (
                uconf_relative_filename_preference_base
            )
        ==
            uconf_relative_filename_preference_base
        );
    nstring base = (base_relative ? search_path.back() : output_directory);
    search_path.push_back(output_directory);

    //
    // resolve the path of each file
    // 1. the absolute path of the file name is obtained
    // 2. if the file is inside the search list
    // 3. if neither, error
    //
    unsigned number_of_errors = 0;
    {
        nstring_list names2;
        for (size_t j = 0; j < filenames.size(); ++j)
        {
            nstring fn = filenames[j];
            nstring afn = (fn[0] == '/' ? fn : os_path_cat(base, fn));
            cid.get_up()->become_begin();
            nstring s1 = os_pathname(afn, true);
            cid.get_up()->become_end();

            nstring s3;
            for (size_t k = 0; k < search_path.size(); ++k)
            {
                s3 = os_below_dir(search_path[k], s1);
                if (!s3.empty())
                    break;
            }
            if (s3.empty())
            {
                sub_context_ty sc;
                sc.var_set_string("File_Name", s1);
                project_error(cid.get_pp(), &sc, i18n("$filename unrelated"));
                ++number_of_errors;
                continue;
            }
            nstring_list names3 = historian->get_last_dir(s3);
            if (!names3.empty())
                names2.push_back_unique(names3);
            else
                names2.push_back_unique(s3);
        }
        filenames = names2;
    }

    //
    // ensure that each file
    // is in the historical baseline
    //
    for (size_t j = 0; j < filenames.size(); ++j)
    {
        nstring fn = filenames[j];
        file_event *src_data = historian->get_last(fn);
        if (!src_data)
        {
            nstring guess = historian->get_last_fuzzy(fn);
            sub_context_ty sc;
            sc.var_set_string("File_Name", fn);
            if (!guess.empty())
            {
                sc.var_set_string("Guess", guess);
                project_error
                (
                    cid.get_pp(),
                    &sc,
                    i18n("no $filename, closest is $guess")
                );
            }
            else
                project_error(cid.get_pp(), &sc, i18n("no $filename"));
            ++number_of_errors;
        }
    }
    if (number_of_errors)
    {
        sub_context_ty sc;
        sc.var_set_long("Number", number_of_errors);
        sc.var_optional("Number");
        project_fatal(cid.get_pp(), &sc, i18n("no files copied"));
    }

    //
    // make sure output is unambiguous
    //
    if (!output_filename.empty())
    {
        if (filenames.size() != 1)
        {
            sub_context_ty sc;
            sc.var_set_long("Number", filenames.size());
            sc.var_optional("Number");
            sc.fatal_intl(i18n("single file with -Output"));
            // NOTREACHED
        }
    }

    //
    // Only create the output directory when we definiely know that we
    // will produce some output.
    //
    if (!output_directory.empty())
    {
        os_become_orig();
        os_mkdir(output_directory, 0777 & ~original_umask, true);
        os_become_undo();
    }

    //
    // Copy each file into the destination directory.
    // Create any necessary directories along the way.
    //
    for (size_t j = 0; j < filenames.size(); ++j)
    {
        nstring fn = filenames[j];
        file_event *fep = historian->get_last(fn);
        assert(fep);
        if (!fep)
            continue;

        fstate_src_ty *old_src = fep->get_src();
        assert(old_src);
        switch (old_src->action)
        {
        case file_action_remove:
            //
            // The file had been removed at this
            // delta.  Omit it.
            //
            filenames.remove(fn);
            --j;
            continue;

        case file_action_create:
            //
            // The file has been renamed and we asked for the
            // removed name.  Omit it.
            //
            if (fn != nstring(old_src->file_name))
            {
                filenames.remove(fn);
                --j;
                continue;
            }
            // FALLTHROUGH

        case file_action_modify:
        case file_action_insulate:
        case file_action_transparent:
            break;
        }
        int from_unlink = 0;
        nstring from
        (
            project_file_version_path(cid.get_pp(), old_src, &from_unlink)
        );

        //
        // figure where to send it
        //
        nstring to =
            (
                output_filename.empty()
            ?
                os_path_cat(output_directory, fn)
            :
                output_filename
            );

        //
        // copy the file
        //
        os_become_orig();
        if (output_filename.empty())
        {
            os_mkdir_between(output_directory, fn, 0777 & ~original_umask);
            if (os_exists(to))
                os_unlink(to);
        }
        if (to == "-")
            to = "";
        copy_whole_file(from, to, false);

        //
        // set the file mode
        //
        if (output_filename.empty())
        {
            int mode = 0666;
            if (old_src->executable)
                mode |= 0111;
            mode &= ~original_umask;
            os_chmod(to, mode);
        }
        os_become_undo();

        //
        // clean up afterwards
        //
        if (from_unlink)
        {
            os_become_orig();
            os_unlink_errok(from);
            os_become_undo();
        }
    }

    //
    // release the baseline lock
    //
    if (output_filename.empty())
        lock_release();

    //
    // verbose success message
    //
    for (size_t j = 0; j < filenames.size(); ++j)
    {
        sub_context_ty sc;
        sc.var_set_string("File_Name", filenames[j]);
        project_verbose(cid.get_pp(), &sc, i18n("copied $filename"));
    }
    trace(("}\n"));
}


static fstate_src_ty *
fake_removed_file(project *pp, string_ty *filename)
{
    fstate_src_ty   *p_src_data;
    fstate_src_ty   *old_src;

    p_src_data = pp->file_find(filename, view_path_simple);
    assert(p_src_data);
    old_src = (fstate_src_ty *)fstate_src_type.alloc();
    old_src->action = file_action_remove;
    change_file_copy_basic_attributes(old_src, p_src_data);
    if (p_src_data && p_src_data->edit)
    {
        assert(p_src_data->edit);
        old_src->edit = history_version_copy(p_src_data->edit);
    }
    else
    {
        // Should never happen.  Yeah, right.
        old_src->usage = file_usage_source;
        old_src->edit = (history_version_ty *)history_version_type.alloc();
        old_src->edit->revision = str_from_c("1.1");
    }
    return old_src;
}


static bool
delete_file_p(user_ty::pointer up, const nstring &filename)
{
    up->become_end();
    bool result = up->delete_file_query(filename, false, true);
    up->become_begin();
    return result;
}


static void
copy_file_main(void)
{
    string_ty       *dd;
    string_ty       *s1;
    string_ty       *s2;
    cstate_ty       *cstate_data;
    size_t          j;
    size_t          k;
    string_ty       *project_name;
    project      *pp;
    project      *pp2;
    long            change_number;
    change::pointer cp;
    log_style_ty    log_style;
    user_ty::pointer up;
    const char      *output_filename;
    time_t          delta_date;
    long            delta_number;
    const char      *delta_name;
    long            delta_from_change;
    int             config_seen;
    int             number_of_errors;
    const char      *branch;
    int             trunk;
    int             read_only;

    trace(("copy_file_main()\n{\n"));
    arglex();
    string_list_ty wl;
    bool overwriting = false;
    project_name = 0;
    change_number = 0;
    log_style = log_style_append_default;
    output_filename = 0;
    delta_date = NO_TIME_SET;
    delta_number = -1;
    delta_name = 0;
    delta_from_change = 0;
    branch = 0;
    trunk = 0;
    read_only = 0;
    bool rescind = false;
    bool build_files_are_ok = false;
    bool as_needed = false;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(copy_file_usage);
            continue;

        case arglex_token_overwriting:
            if (overwriting)
                duplicate_option(copy_file_usage);
            overwriting = true;
            break;

        case arglex_token_directory:
            if (arglex() != arglex_token_string)
                option_needs_dir(arglex_token_directory, copy_file_usage);
            goto get_file_names;

        case arglex_token_file:
            if (arglex() != arglex_token_string)
                option_needs_files(arglex_token_file, copy_file_usage);
            // fall through...

        case arglex_token_string:
          get_file_names:
            s2 = str_from_c(arglex_value.alv_string);
            wl.push_back(s2);
            str_free(s2);
            break;

        case arglex_token_change:
            arglex();
            // fall through...

        case arglex_token_number:
            arglex_parse_change_with_branch
            (
                &project_name,
                &change_number,
                &branch,
                copy_file_usage
            );
            continue;

        case arglex_token_project:
            arglex();
            arglex_parse_project(&project_name, copy_file_usage);
            continue;

        case arglex_token_nolog:
            if (log_style == log_style_none)
                duplicate_option(copy_file_usage);
            log_style = log_style_none;
            break;

        case arglex_token_delta:
            if (delta_number >= 0 || delta_name)
                duplicate_option(copy_file_usage);
            switch (arglex())
            {
            default:
                option_needs_number(arglex_token_delta, copy_file_usage);
                // NOTREACHED

            case arglex_token_number:
                delta_number = arglex_value.alv_number;
                if (delta_number < 0)
                {
                    sub_context_ty sc;
                    sc.var_set_long("Number", delta_number);
                    sc.fatal_intl(i18n("delta $number out of range"));
                    // NOTREACHED
                }
                break;

            case arglex_token_string:
                delta_name = arglex_value.alv_string;
                break;
            }
            break;

        case arglex_token_delta_date:
            if (delta_date != NO_TIME_SET)
                duplicate_option(copy_file_usage);
            if (arglex() != arglex_token_string)
            {
                option_needs_string(arglex_token_delta_date, copy_file_usage);
                // NOTREACHED
            }
            delta_date = date_scan(arglex_value.alv_string);
            if (delta_date == NO_TIME_SET)
            {
                sub_context_ty sc;
                sc.var_set_charstar("Name", arglex_value.alv_string);
                sc.fatal_intl(i18n("date $name unknown"));
                // NOTREACHED
            }
            break;

        case arglex_token_delta_from_change:
            if (arglex() != arglex_token_number)
            {
                option_needs_number
                (
                    arglex_token_delta_from_change,
                    copy_file_usage
                );
            }
            if (delta_from_change)
            {
                duplicate_option_by_name
                (
                    arglex_token_delta_from_change,
                    copy_file_usage
                );
            }
            delta_from_change = arglex_value.alv_number;
            if (delta_from_change == 0)
                delta_from_change = MAGIC_ZERO;
            else if (delta_from_change < 1)
            {
                sub_context_ty sc;
                sc.var_set_long("Number", change_number);
                sc.fatal_intl(i18n("change $number out of range"));
                // NOTREACHED
            }
            break;

        case arglex_token_output:
            if (output_filename)
                duplicate_option(copy_file_usage);
            switch (arglex())
            {
            default:
                option_needs_file(arglex_token_output, copy_file_usage);

            case arglex_token_stdio:
                output_filename = "";
                break;

            case arglex_token_string:
                output_filename = arglex_value.alv_string;
                break;
            }
            break;

        case arglex_token_branch:
            if (branch)
                duplicate_option(copy_file_usage);
            switch (arglex())
            {
            default:
                option_needs_number(arglex_token_branch, copy_file_usage);

            case arglex_token_number:
            case arglex_token_string:
                branch = arglex_value.alv_string;
                break;

            case arglex_token_stdio:
                branch = "";
                break;
            }
            break;

        case arglex_token_trunk:
            if (trunk)
                duplicate_option(copy_file_usage);
            ++trunk;
            break;

        case arglex_token_read_only:
            if (read_only)
                duplicate_option(copy_file_usage);
            ++read_only;
            break;

        case arglex_token_wait:
        case arglex_token_wait_not:
            user_ty::lock_wait_argument(copy_file_usage);
            break;

        case arglex_token_base_relative:
        case arglex_token_current_relative:
            user_ty::relative_filename_preference_argument(copy_file_usage);
            break;

        case arglex_token_rescind:
            if (rescind)
                duplicate_option(copy_file_usage);
            rescind = true;
            break;

        case arglex_token_keep:
        case arglex_token_interactive:
        case arglex_token_keep_not:
            user_ty::delete_file_argument(copy_file_usage);
            break;

        case arglex_token_build:
            // Undocumented, for use by aepatch
            build_files_are_ok = true;
            break;

        case arglex_token_as_needed:
            if (as_needed)
                duplicate_option(copy_file_usage);
            as_needed = true;
            break;
        }
        arglex();
    }
    if (overwriting && as_needed)
    {
        mutually_exclusive_options
        (
            arglex_token_as_needed,
            arglex_token_overwriting,
            copy_file_usage
        );
    }
    if (!wl.nstrings && !rescind)
    {
        error_intl(0, i18n("no file names"));
        copy_file_usage();
    }
    if (trunk)
    {
        if (branch)
        {
            mutually_exclusive_options
            (
                arglex_token_branch,
                arglex_token_trunk,
                copy_file_usage
            );
        }
        branch = "";
    }
    if (rescind && output_filename && wl.nstrings != 1)
    {
        mutually_exclusive_options
        (
            arglex_token_rescind,
            arglex_token_output,
            copy_file_usage
        );
    }
    if
    (
        (
            (delta_name || delta_number >= 0)
        +
            !!delta_from_change
        +
            (delta_date != NO_TIME_SET)
        )
    >
        1
    )
    {
        mutually_exclusive_options3
        (
            arglex_token_delta,
            arglex_token_delta_date,
            arglex_token_delta_from_change,
            copy_file_usage
        );
    }
    if
    (
        rescind
    &&
        !delta_name
    &&
        delta_number < 0
    &&
        !delta_from_change
    &&
        delta_date == NO_TIME_SET
    )
    {
        sub_context_ty sc;
        sc.var_set_charstar("Name1", arglex_token_name(arglex_token_rescind));
        sc.var_set_charstar("Name2", arglex_token_name(arglex_token_delta));
        sc.fatal_intl(i18n("$name1 needs $name2"));
        // NOTREACHED
    }

    //
    // make sure output is unambiguous
    //
    if (output_filename)
    {
        if (wl.nstrings != 1)
        {
            sub_context_ty sc;
            sc.var_set_long("Number", (long)wl.nstrings);
            sc.var_optional("Number");
            sc.fatal_intl(i18n("single file with -Output"));
            // NOTREACHED
        }
        overwriting = true;
    }

    //
    // locate project data
    //
    if (!project_name)
    {
        nstring n = user_ty::create()->default_project();
        project_name = str_copy(n.get_ref());
    }
    pp = project_alloc(project_name);
    str_free(project_name);
    pp->bind_existing();

    //
    // locate which branch
    //
    if (branch)
        pp2 = pp->find_branch(branch);
    else
        pp2 = project_copy(pp);

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
    // lock the change file
    //
    // Also take a read lock on the baseline, to ensure that it does
    // not change (aeip) for the duration of the build.
    //
    if (!output_filename)
    {
        change_cstate_lock_prepare(cp);
        project_baseline_read_lock_prepare(pp2);
        lock_take();

        log_open(change_logfile_get(cp), up, log_style);

        if (cp->file_promote())
        {
            trace(("The change::file_promote found somthing to do.\n"));

            //
            // Write out the file state, and then let go of the locks
            // and take them again.  This ensures the data is consistent
            // for the next stage of processing.
            //
            trace(("Write out what we've done so far.\n"));
            cp->cstate_write();
            commit();
            lock_release();

            trace(("Take the locks again.\n"));
            change_cstate_lock_prepare(cp);
            project_baseline_read_lock_prepare(pp2);
            lock_take();
        }
    }
    cstate_data = cp->cstate_get();

    //
    // When there is no explicit output file:
    // It is an error if the change is not in the being_developed state.
    // It is an error if the change is not assigned to the current user.
    //
    if (output_filename)
    {
        switch (cstate_data->state)
        {
        case cstate_state_awaiting_development:
        case cstate_state_awaiting_integration:
        case cstate_state_awaiting_review:
        case cstate_state_being_developed:
        case cstate_state_being_integrated:
        case cstate_state_being_reviewed:
        case cstate_state_completed:
            break;

#ifndef DEBUG
        default:
#endif
            wrong_state:
            change_fatal(cp, 0, i18n("bad cp state"));
        }
    }
    else
    {
        if (cstate_data->state != cstate_state_being_developed)
            goto wrong_state;
        if (cp->is_a_branch())
            change_fatal(cp, 0, i18n("bad branch cp"));
        if (nstring(cp->developer_name()) != up->name())
            change_fatal(cp, 0, i18n("not developer"));
    }

    //
    // it is an error if the delta does not exist
    //
    if (delta_name)
    {
        s1 = str_from_c(delta_name);
        delta_number = project_history_delta_by_name(pp2, s1, 0);
        str_free(s1);
    }
    if (delta_date != NO_TIME_SET)
    {
        //
        // If the time is in the future, you could get a different
        // answer for the same input at some point in the future.
        //
        // This is the "time safe" quality first described by
        // Damon Poole <damon@ede.com>
        //
        if (delta_date > now())
            project_error(pp2, 0, i18n("date in the future"));
    }
    if (delta_from_change)
    {
        delta_number =
            project_change_number_to_delta_number(pp2, delta_from_change);
    }
    if (delta_number >= 0)
    {
        delta_date = project_history_delta_to_timestamp(pp2, delta_number);
        if (delta_date == NO_TIME_SET)
        {
            sub_context_ty sc;
            sc.var_set_long("Name", delta_number);
            change_fatal(cp, &sc, i18n("no delta $name"));
            // NOTREACHED
        }
        trace(("delta %ld -> delta date %ld\n", delta_number,
               (long)delta_date));
    }

    //
    // We may need to consult the project historian
    // for the historical list of files.
    //
    project_file_roll_forward historian;
    if (delta_date != NO_TIME_SET)
    {
        historian.set(pp2, delta_date, 0);

        if (wl.nstrings == 0)
        {
            assert(rescind);
            //
            // If no files are named in an aecp -rescind command, the
            // list of files is implied by the change being rescinded.
            //
            // However, files which are already in the change are to be
            // avoided unless -overwriting is specified.
            //
            size_t used = 0;
            size_t available = 0;
            change::pointer cp2 = historian.get_last_change();
            assert(cp2);
            for (size_t n = 0; ; ++n)
            {
                fstate_src_ty *src = change_file_nth(cp2, n, view_path_first);
                if (!src)
                    break;
                if (overwriting || !cp->file_find(src, view_path_first))
                {
                    wl.push_back(src->file_name);
                    ++used;
                }
                ++available;
            }
            if (!used)
            {
                //
                // FIXME: This isn't exactly the best error message,
                // but it will do for now.
                //
                sub_context_ty sc;
                sc.var_set_charstar("File_Name", ".");
                sc.var_set_long("Number", (long)available);
                sc.var_optional("Number");
                change_fatal
                (
                    cp,
                    &sc,
                    i18n("directory $filename contains no relevant files")
                );
                // NOTREACHED
            }
        }
    }

    //
    // build the list of places to look
    // when resolving the file name
    //
    // To cope with automounters, directories are stored as given,
    // or are derived from the home directory in the passwd file.
    // Within aegis, pathnames have their symbolic links resolved,
    // and any comparison of paths is done on this "system idea"
    // of the pathname.
    //
    string_list_ty search_path;
    cp->search_path_get(&search_path, true);
    assert(search_path.nstrings >= 1);

    //
    // Find the base for relative filenames.
    //
    bool base_relative =
        (
            up->relative_filename_preference
            (
                uconf_relative_filename_preference_current
            )
        ==
            uconf_relative_filename_preference_base
        );

    os_become_orig();
    string_ty *curdir = os_curdir();
    os_become_undo();

    string_ty *base = str_copy(base_relative ? search_path.string[0] : curdir);

    //
    // resolve the path of each file
    // 1. the absolute path of the file name is obtained
    // 2. if the file is inside the search list
    // 3. if neither, error
    //
    config_seen = 0;
    string_list_ty wl2;
    number_of_errors = 0;
    for (j = 0; j < wl.nstrings; ++j)
    {
        s1 = wl.string[j];
        trace_string(s1->str_text);
        if (s1->str_text[0] == '/')
            s2 = str_copy(s1);
        else
            s2 = os_path_cat(base, s1);
        up->become_begin();
        s1 = os_pathname(s2, 1);
        up->become_end();
        str_free(s2);
        s2 = 0;
        for (k = 0; k < search_path.nstrings; ++k)
        {
            s2 = os_below_dir(search_path.string[k], s1);
            if (s2)
                break;
        }
        str_free(s1);
        if (!s2)
        {
            sub_context_ty sc;
            sc.var_set_string("File_Name", wl.string[j]);
            change_error(cp, &sc, i18n("$filename unrelated"));
            ++number_of_errors;
            continue;
        }
        string_list_ty wl_out;
        string_list_ty wl_in;
        pp2->file_directory_query
        (
            s2,
            &wl_in,
            &wl_out,
            view_path_simple
        );
        if (delta_date != NO_TIME_SET)
            wl_in.push_back(wl_out);
        if (wl_in.nstrings)
        {
            int             used;

            //
            // if the user named a directory,
            // add all of the source files in that directory,
            // provided they are not already in the change.
            //
            if (output_filename)
            {
                sub_context_ty sc;
                sc.var_set_charstar
                (
                    "Name",
                    arglex_token_name(arglex_token_output)
                );
                sc.error_intl(i18n("no dir with $name"));
                ++number_of_errors;
            }
            used = 0;
            for (k = 0; k < wl_in.nstrings; ++k)
            {
                string_ty       *s3;

                s3 = wl_in.string[k];
                trace_string(s3->str_text);
                if (overwriting || !cp->file_find(nstring(s3), view_path_first))
                {
                    if (wl2.member(s3))
                    {
                        sub_context_ty sc;
                        sc.var_set_string("File_Name", s3);
                        change_error(cp, &sc, i18n("too many $filename"));
                        ++number_of_errors;
                    }
                    else
                        wl2.push_back(s3);
                    if (cp->file_is_config(s3))
                        ++config_seen;
                    ++used;
                }
            }
            if (!used)
            {
                sub_context_ty sc;
                if (s2->str_length)
                    sc.var_set_string("File_Name", s2);
                else
                    sc.var_set_charstar("File_Name", ".");
                sc.var_set_long("Number", (long)wl_in.nstrings);
                sc.var_optional("Number");
                change_error
                (
                    cp,
                    &sc,
                    i18n("directory $filename contains no relevant files")
                );
                ++number_of_errors;
            }
        }
        else
        {
            if (wl2.member(s2))
            {
                sub_context_ty sc;
                sc.var_set_string("File_Name", s2);
                change_error(cp, &sc, i18n("too many $filename"));
                ++number_of_errors;
            }
            else
            {
                //
                // Ignore redundant requests.
                //
                bool copy_me = true;
                if (as_needed)
                {
                    fstate_src_ty *csrc =
                        cp->file_find(nstring(s2), view_path_first);
                    if (csrc && csrc->action == file_action_modify)
                        copy_me = false;
                }
                if (copy_me)
                    wl2.push_back(s2);
            }
            if (cp->file_is_config(s2))
                ++config_seen;
        }
        str_free(s2);
    }
    wl = wl2;

    //
    // ensure that each file
    // 1. is not already part of the change
    // 2. is in the baseline
    //
    for (j = 0; j < wl.nstrings; ++j)
    {
        fstate_src_ty   *src_data;

        s1 = wl.string[j];
        if
        (
            cp->file_find(nstring(s1), view_path_first)
        &&
            !overwriting
        &&
            !output_filename
        )
        {
            sub_context_ty sc;
            sc.var_set_string("File_Name", s1);
            change_error(cp, &sc, i18n("bad cp, file $filename dup"));
            ++number_of_errors;
            continue;
        }
        if (output_filename)
        {
            fstate_src_ty   *c_src_data;

            //
            // OK to use a file that "almost" exists
            // in combination with the -Output option
            //
            c_src_data = cp->file_find(nstring(s1), view_path_first);
            if (c_src_data)
            {
                switch (c_src_data->action)
                {
                case file_action_create:
                    continue;

                case file_action_modify:
                case file_action_remove:
                case file_action_insulate:
                case file_action_transparent:
                    break;
                }
            }
        }
        src_data = pp2->file_find(s1, view_path_simple);
        if
        (
            !src_data
        ||
            (delta_date == NO_TIME_SET && src_data->deleted_by)
        )
        {
            sub_context_ty sc;
            src_data = pp2->file_find_fuzzy(s1, view_path_extreme);
            sc.var_set_string("File_Name", s1);
            if (src_data)
            {
                sc.var_set_string("Guess", src_data->file_name);
                project_error
                (
                    pp2,
                    &sc,
                    i18n("no $filename, closest is $guess")
                );
            }
            else
                project_error(pp2, &sc, i18n("no $filename"));
            ++number_of_errors;
            continue;
        }
        if (src_data && !output_filename)
        {
            switch (src_data->usage)
            {
            case file_usage_source:
            case file_usage_config:
            case file_usage_test:
            case file_usage_manual_test:
                break;

            case file_usage_build:
                if (!build_files_are_ok)
                {
                    sub_context_ty sc;
                    sc.var_set_string("File_Name", s1);
                    change_error(cp, &sc, i18n("$filename is built"));
                    ++number_of_errors;
                }
                break;
            }
        }
    }
    if (number_of_errors)
    {
        sub_context_ty sc;
        sc.var_set_long("Number", number_of_errors);
        sc.var_optional("Number");
        change_fatal(cp, &sc, i18n("no files copied"));
    }

    //
    // Copy each file into the development directory.
    // Create any necessary directories along the way.
    //
    // Add each file to the change file,
    // or update the edit number.
    //
    dd = change_development_directory_get(cp, 0);
    for (j = 0; j < wl.nstrings; ++j)
    {
        string_ty       *from = 0;
        string_ty       *to = 0;
        fstate_src_ty   *old_src = 0;
        fstate_src_ty   *older_src = 0;
        int             from_unlink = 0;

        s1 = wl.string[j];
        trace(("s1 = \"%s\";\n", s1->str_text));
        if (delta_date != NO_TIME_SET)
        {
            file_event   *fep;

            fep = historian.get_last(s1);
            if (!fep)
            {
                //
                // This file had not yet been created at
                // the time of the delta.  Arrange for
                // it to look like it's being removed.
                //
                // In the case of -rescind, it doesn't exist at the
                // previous delta, either, so remove it in this case, too.
                //
                // This is a memory leak.
                //
                old_src = fake_removed_file(pp2, s1);
                older_src = old_src;
            }
            else
            {
                old_src = fep->get_src();
                if (rescind)
                {
                    fep = historian.get_older(s1);
                    trace(("fep = %lX\n", (long)fep));
                    if (fep)
                    {
                        older_src = fep->get_src();
                    }
                    else
                    {
                        // This is a memory leak.
                        older_src = fake_removed_file(pp2, s1);
                    }
                }
                else
                {
                    //
                    // If the file has been renamed we asked for the
                    // old name of the file and get the new one.  So
                    // we remove the file name from the list.
                    //
                    if (!str_equal(s1, old_src->file_name))
                    {
                        wl.remove(s1);
                        --j;
                        continue;
                    }
                    older_src = old_src;
                }
            }
            assert(old_src);
            trace(("old_src = %lX\n", (long)old_src));
            assert(older_src);
            trace(("older_src = %lX\n", (long)older_src));
            bool set_mode = true;
            switch (older_src->action)
            {
            case file_action_remove:
                trace_string(s1->str_text);
                change_file_whiteout_write(cp, s1, up);
                goto done;

            case file_action_create:
            case file_action_modify:
            case file_action_insulate:
            case file_action_transparent:
#ifndef DEBUG
            default:
#endif
                from = project_file_version_path(pp2, older_src, &from_unlink);
                break;
            }
            trace(("from = \"%s\";\n", from->str_text));

            //
            // figure where to send it
            //
            if (output_filename)
                to = str_from_c(output_filename);
            else
                to = os_path_cat(dd, s1);

            //
            // copy the file
            //
            // But only if it doesn't exist,
            // or the user didn't say --keep.
            //
            up->become_begin();
            if (output_filename)
            {
                copy_whole_file(from, to, 0);
            }
            else
            {
                if (os_exists(to) && !os_symlink_query(to))
                {
                    //
                    // File exists in development directory.
                    // Be careful replacing it.
                    //
                    if (overwriting || delete_file_p(up, nstring(s1)))
                    {
                        os_unlink(to);
                        copy_whole_file(from, to, 0);
                    }
                    else
                        set_mode = false;
                }
                else
                {
                    //
                    // File does not exist in the development directory.
                    // (But a symlink may.)
                    //
                    os_mkdir_between(dd, s1, 02755);
                    os_unlink(to);
                    copy_whole_file(from, to, 0);
                }
            }

            //
            // set the file mode
            //
            if (set_mode)
            {
                int mode = 0444;
                if (!read_only)
                    mode |= 0600;
                if (older_src->executable)
                    mode |= 0111;
                mode &= ~cp->umask_get();
                os_chmod(to, mode);
            }

            //
            // clean up afterwards
            //
            if (from_unlink)
                os_unlink_errok(from);
            up->become_end();
        done:
            str_free(from);
            str_free(to);
        }
        else
        {
            if (cstate_data->state == cstate_state_being_integrated)
            {
                from =
                    os_path_cat(change_integration_directory_get(cp, 0), s1);
            }
            else
            {
                from = project_file_path(pp2, s1);
            }
            if (output_filename)
                to = str_from_c(output_filename);
            else
                to = os_path_cat(dd, s1);

            //
            // We need the file information for the execuable bit.
            //
            old_src = pp2->file_find(s1, view_path_simple);
            assert(old_src);
            older_src = old_src;

            os_become_orig();
            int file_exists = os_exists(from);
            os_become_undo();
            if (!file_exists)
                from =
                    project_file_version_path (pp2, old_src, &from_unlink);

            //
            // copy the file
            //
            bool set_mode = true;
            up->become_begin();
            if (output_filename)
            {
                copy_whole_file(from, to, 0);
            }
            else
            {
                if (os_exists(to) && !os_symlink_query(to))
                {
                    //
                    // File exists in development directory.
                    // Be careful replacing it.
                    //
                    if (overwriting || delete_file_p(up, nstring(s1)))
                    {
                        os_unlink(to);
                        copy_whole_file(from, to, 0);
                    }
                    else
                        set_mode = false;
                }
                else
                {
                    //
                    // File does not exist in the development directory.
                    // (But a symlink may.)
                    //
                    os_mkdir_between(dd, s1, 02755);
                    os_unlink(to);
                    copy_whole_file(from, to, 0);
                }
            }

            //
            // set the file mode
            //
            if (set_mode)
            {
                int mode = 0444;
                if (!read_only)
                    mode |= 0600;
                if (old_src->executable)
                    mode |= 0111;
                mode &= ~cp->umask_get();
                os_chmod(to, mode);
            }
            up->become_end();

            //
            // clean up afterwards
            //
            str_free(from);
            str_free(to);
        }

        if (!output_filename)
        {
            fstate_src_ty   *c_src_data;
            fstate_src_ty   *p_src_data;

            assert(!old_src == !older_src);
            p_src_data = older_src;
            if (!p_src_data)
                p_src_data = pp2->file_find(s1, view_path_simple);
            assert(p_src_data);
            //
            // It's tempting to say:
            //
            // assert(p_src_data->edit);
            // assert(p_src_data->edit->revision);
            //
            // but it is not true for removed files.
            //
            // FIXME: shouldn't this be find-by-meta?
            c_src_data = cp->file_find(nstring(s1), view_path_first);
            if (!c_src_data)
            {
                c_src_data = cp->file_new(s1);
                // we copy the meta data soon,
                // so don't bother with cp->file_new(p_src_data);
            }
            switch (p_src_data->action)
            {
            case file_action_remove:
                c_src_data->action = file_action_remove;
                break;

            case file_action_insulate:
            case file_action_transparent:
                assert(0);
                // fall through...

            case file_action_create:
            case file_action_modify:
#ifndef DEBUG
            default:
#endif
                assert(p_src_data->edit);
                assert(p_src_data->edit->revision);
                c_src_data->action =
                    (read_only ? file_action_insulate : file_action_modify);
                break;
            }

            //
            // Copy the file usage, attributes and uuid.
            //
            change_file_copy_basic_attributes(c_src_data, p_src_data);

            //
            // Watch out for test times.
            //
            if (!read_only)
            {
                int             f_idx;
                int             more_tests;

                switch (c_src_data->usage)
                {
                case file_usage_test:
                case file_usage_manual_test:
                    //
                    // The change now has at least one test, so cancel
                    // any testing exemption.
                    // (But test_baseline_exempt is still viable.)
                    //
                    change_rescind_test_exemption(cp);

                    //
                    // If there are no more tests, then the change
                    // must be made regression test exempt
                    //
                    more_tests = 0;
                    for (f_idx = 0; ; ++f_idx)
                    {
                        fstate_src_ty   *p_src;

                        p_src = pp2->file_nth(f_idx, view_path_extreme);
                        if (!p_src)
                            break;
                        switch (p_src->usage)
                        {
                        case file_usage_test:
                        case file_usage_manual_test:
                            if
                            (
                                !cp->file_find
                                (
                                    nstring(p_src->file_name),
                                    view_path_first
                                )
                            )
                                more_tests = 1;
                            break;

                        case file_usage_source:
                        case file_usage_config:
                        case file_usage_build:
                            continue;
                        }
                        break;
                    }
                    if (!more_tests)
                        change_force_regression_test_exemption(cp);
                    break;

                case file_usage_source:
                case file_usage_config:
                case file_usage_build:
                    break;
                }
            }
            if (old_src != older_src)
            {
                //
                // In the case of -rescind, crank forward to the following
                // version.  That way we have copied the previous version,
                // but claim the following version.  This will have the
                // effect to backing out the delta specified.
                //
                assert(old_src);
                p_src_data = old_src;
                assert(p_src_data);
                assert(p_src_data->edit);
                assert(p_src_data->edit->revision);
            }

            //
            // p_src_data->edit
            //      The head revision of the branch.
            // p_src_data->edit_origin
            //      The version originally copied.
            //
            // c_src_data->edit
            //      Not meaningful until after integrate pass.
            // c_src_data->edit_origin
            //      The version originally copied.
            // c_src_data->edit_origin_new
            //      Updates branch edit_origin on
            //      integrate pass.
            //
            if (c_src_data->edit)
            {
                assert(c_src_data->edit->revision);
                history_version_type.free(c_src_data->edit);
                c_src_data->edit = 0;
            }
            if (c_src_data->edit_origin)
            {
                assert(c_src_data->edit_origin->revision);
                history_version_type.free(c_src_data->edit_origin);
                c_src_data->edit_origin = 0;
            }
            if (c_src_data->edit_origin_new)
            {
                assert(c_src_data->edit_origin_new->revision);
                history_version_type.free(c_src_data->edit_origin_new);
                c_src_data->edit_origin_new = 0;
            }
            if (p_src_data->edit)
            {
                assert(p_src_data->edit->revision);
                c_src_data->edit_origin =
                    history_version_copy(p_src_data->edit);
            }
            else
            {
                assert(p_src_data->edit_origin);
                assert(p_src_data->edit_origin->revision);
                assert(p_src_data->action == file_action_remove);
                c_src_data->edit_origin =
                    history_version_copy(p_src_data->edit_origin);
            }

            //
            // Copying the config file into a change
            // invalidates all of the file fingerprints.
            // This is because the diff command,
            // test_command, build_command, etc, could be
            // changed when the config file is edited.
            //
            if (config_seen && c_src_data->file_fp)
            {
                fingerprint_type.free(c_src_data->file_fp);
                c_src_data->file_fp = 0;
            }
        }

        //
        // verbose progress message
        //
        sub_context_ty sc;
        sc.var_set_string("File_Name", s1);
        change_verbose(cp, &sc, i18n("copied $filename"));
    }

    bool recent_integration = false;
    if (!output_filename)
    {
        //
        // the number of files changed,
        // so stomp on the validation fields.
        //
        change_build_times_clear(cp);

        //
        // update the copyright years
        //
        change_copyright_years_now(cp);

        //
        // If the file manifest of the change is altered (e.g. by aenf, aenfu,
        // aecp, aecpu, etc), or the contents of any file is changed, the
        // UUID is cleared.  This is because it is no longer the same change
        // as was received by aedist or aepatch, and the UUID is invalidated.
        //
        change_uuid_clear(cp);

        // remember that we are about to
        recent_integration = cp->run_project_file_command_needed();
        if (recent_integration)
            cp->run_project_file_command_done();

        //
        // release the locks
        //
        cp->cstate_write();
        commit();
        lock_release();
    }

    //
    // verbose success message
    //
    sub_context_ty sc;
    sc.var_set_long("Number", (long)wl.nstrings);
    sc.var_optional("Number");
    change_verbose(cp, &sc, i18n("copy file complete"));

    if (!output_filename)
    {
        //
        // run the change file command
        // and the project file command if necessary
        //
        // The -output option means the change set's state is not
        // altered in any way, so no notification is required.
        //
        cp->run_copy_file_command(&wl, up);
    }

    if (recent_integration)
        cp->run_project_file_command(up);

    project_free(pp);
    change_free(cp);
    trace(("}\n"));
}


void
copy_file(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
        { arglex_token_help, copy_file_help, 0 },
        { arglex_token_list, copy_file_list, 0 },
        { arglex_token_independent, copy_file_independent, 0 },
    };

    trace(("copy_file()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), copy_file_main);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
