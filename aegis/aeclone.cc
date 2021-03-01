//
// aegis - project change supervisor
// Copyright (C) 1998-2009, 2011, 2012 Peter Miller
// Copyright (C) 2006-2009 Walter Franzini
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
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>
#include <common/ac/stdio.h>

#include <common/progname.h>
#include <common/quit.h>
#include <common/sizeof.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <common/uuidentifier.h>
#include <libaegis/ael/change/by_state.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/arglex2.h>
#include <libaegis/attribute.h>
#include <libaegis/change.h>
#include <libaegis/change/attributes.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/commit.h>
#include <libaegis/file.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/project/file.h>
#include <libaegis/project/history.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>

#include <aegis/aeclone.h>
#include <aegis/aenc.h>


static void
clone_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf(stderr, "usage: %s -CLone [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -CLone -List [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -CLone -Help\n", progname);
    quit(1);
}


static void
clone_help(void)
{
    help("aeclone", clone_usage);
}


static void
clone_list(void)
{
    trace(("clone_list()\n{\n"));
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(clone_usage);
    list_changes_in_state_mask(cid, ~(1 << cstate_state_awaiting_development));
    trace(("}\n"));
}


static void
clone_main(void)
{
    string_ty       *s;
    sub_context_ty  *scp;
    string_ty       *project_name;
    long            change_number;
    long            change_number2;
    project      *pp;
    project      *pp2;
    user_ty::pointer up;
    change::pointer cp;
    change::pointer cp2;
    cstate_ty       *cstate_data;
    cstate_ty       *cstate_data2;
    cstate_history_ty *history_data;
    string_ty       *devdir;
    size_t          j;
    pconf_ty        *pconf_data;
    const char      *branch;
    int             trunk;
    int             grandparent;
    int             mode;
    const char      *output_filename;
    int             the_umask;

    trace(("clone_main()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    change_number2 = 0;
    devdir = 0;
    branch = 0;
    trunk = 0;
    grandparent = 0;
    output_filename = 0;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(clone_usage);
            continue;

        case arglex_token_change:
            arglex();
            // fall through...

        case arglex_token_number:
            if (!change_number2)
            {
                // change to clone
                arglex_parse_change_with_branch
                (
                    &project_name,
                    &change_number2,
                    &branch,
                    clone_usage
                );
            }
            else
            {
                // change to create
                arglex_parse_change(&project_name, &change_number, clone_usage);
            }
            continue;

        case arglex_token_project:
            arglex();
            arglex_parse_project(&project_name, clone_usage);
            continue;

        case arglex_token_directory:
            if (devdir)
                duplicate_option(clone_usage);
            if (arglex() != arglex_token_string)
                option_needs_dir(arglex_token_directory, clone_usage);
            devdir = str_from_c(arglex_value.alv_string);
            break;

        case arglex_token_branch:
            if (branch)
                duplicate_option(clone_usage);
            switch (arglex())
            {
            default:
                option_needs_number(arglex_token_branch, clone_usage);

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
                duplicate_option(clone_usage);
            ++trunk;
            break;

        case arglex_token_grandparent:
            if (grandparent)
                duplicate_option(clone_usage);
            ++grandparent;
            break;

        case arglex_token_wait:
        case arglex_token_wait_not:
            user_ty::lock_wait_argument(clone_usage);
            break;

        case arglex_token_whiteout:
        case arglex_token_whiteout_not:
            user_ty::whiteout_argument(clone_usage);
            break;

        case arglex_token_output:
            if (output_filename)
                duplicate_option(clone_usage);
            switch (arglex())
            {
            default:
                option_needs_file(arglex_token_output, clone_usage);
                // NOTREACHED

            case arglex_token_string:
                output_filename = arglex_value.alv_string;
                break;

            case arglex_token_stdio:
                output_filename = "";
                break;
            }
            break;
        }
        arglex();
    }
    if (!change_number2)
        fatal_intl(0, i18n("no change number"));
    if (change_number && output_filename)
    {
        mutually_exclusive_options
        (
            arglex_token_change,
            arglex_token_output,
            clone_usage
        );
    }
    if (grandparent)
    {
        if (branch)
        {
            mutually_exclusive_options
            (
                arglex_token_branch,
                arglex_token_grandparent,
                clone_usage
            );
        }
        if (trunk)
        {
            mutually_exclusive_options
            (
                arglex_token_trunk,
                arglex_token_grandparent,
                clone_usage
            );
        }
        branch = "..";
    }
    if (trunk)
    {
        if (branch)
        {
            mutually_exclusive_options
            (
                arglex_token_branch,
                arglex_token_trunk,
                clone_usage
            );
        }
        branch = "";
    }

    //
    // locate project data
    //
    if (!project_name)
    {
        nstring n = user_ty::create()->default_project();
        project_name = n.get_ref_copy();
    }
    pp = project_alloc(project_name);
    str_free(project_name);
    pp->bind_existing();

    //
    // make sure this branch of the project is still active
    //
    if (!pp->change_get()->is_a_branch())
        project_fatal(pp, 0, i18n("branch completed"));

    //
    // locate user data
    //
    up = user_ty::create();

    //
    // Lock the project state file.
    // Block if necessary.
    //
    pp->pstate_lock_prepare();
    up->ustate_lock_prepare();
    lock_take();

    //
    // make sure they are allowed to
    //
    new_change_check_permission(pp, up);

    //
    // locate which branch
    //
    if (branch)
        pp2 = pp->find_branch(branch);
    else
        pp2 = project_copy(pp);

    //
    // locate change data
    //      on the other branch
    //
    assert(change_number2);
    cp2 = change_alloc(pp2, change_number2);
    change_bind_existing(cp2);
    cstate_data2 = cp2->cstate_get();

    //
    // create a new change
    //
    if (!change_number)
        change_number = project_next_change_number(pp, 1);
    else
    {
        if (project_change_number_in_use(pp, change_number))
        {
            scp = sub_context_new();
            sub_var_set_long(scp, "Number", magic_zero_decode(change_number));
            project_fatal(pp, scp, i18n("change $number used"));
            // NOTREACHED
            sub_context_delete(scp);
        }
    }
    cp = change_alloc(pp, change_number);
    change_bind_new(cp);
    cstate_data = cp->cstate_get();

    //
    // copy change attributes from the old change
    //
    assert(cstate_data2->description);
    if (cstate_data2->description)
        cstate_data->description = str_copy(cstate_data2->description);
    assert(cstate_data2->brief_description);
    s = str_trim(cstate_data2->brief_description);
    cstate_data->brief_description =
        str_format
        (
            "%s (clone of %s)",
            s->str_text,
            cp2->version_get().c_str()
        );
    str_free(s);
    cstate_data->cause = cstate_data2->cause;
    cstate_data->test_exempt = cstate_data2->test_exempt;
    cstate_data->test_baseline_exempt = cstate_data2->test_baseline_exempt;
    cstate_data->regression_test_exempt = cstate_data2->regression_test_exempt;
    if (cstate_data2->architecture)
    {
        change_architecture_clear(cp);
        for (j = 0; j < cstate_data2->architecture->length; ++j)
            change_architecture_add(cp, cstate_data2->architecture->list[j]);
    }
    change_copyright_years_now(cp);
    change_copyright_years_merge(cp, cp2);

    //
    // Copy also the user defined attributes in the new change,
    // removing attributes that may be wrong on the other side.
    //
    if (cstate_data2->attribute)
        cstate_data->attribute =
            attributes_list_clone(cstate_data2->attribute);
    attributes_list_remove(cstate_data->attribute, HISTORY_GET_COMMAND);

    //
    // Copy the UUID into a user defined attribute.
    //
    if (cstate_data2->uuid)
    {
        change_attributes_append
        (
            cstate_data,
            ORIGINAL_UUID,
            cstate_data2->uuid->str_text
        );
    }

    //
    // add to history for change creation
    //
    cstate_data->state = cstate_state_awaiting_development;
    history_data = change_history_new(cp, up);
    history_data->what = cstate_history_what_new_change;
    history_data->why =
        str_format
        (
            "Cloned from change %ld.",
            magic_zero_decode(change_number2)
        );

    the_umask = cp->umask_get();

    if (cstate_data2->state >= cstate_state_being_developed)
    {
        //
        // Construct the name of the development directory.
        //
        // (Do this before the state advances to being developed,
        // it tries to find the config file in the as-yet
        // non-existant development directory.)
        //
        if (!devdir)
        {
            scp = sub_context_new();
            devdir = change_development_directory_template(cp, up);
            sub_var_set_string(scp, "File_Name", devdir);
            change_verbose
            (
                cp,
                scp,
                i18n("development directory \"$filename\"")
            );
            sub_context_delete(scp);
        }
        change_development_directory_set(cp, devdir);

        //
        // add to history for develop begin
        //
        cstate_data->state = cstate_state_being_developed;
        history_data = change_history_new(cp, up);
        history_data->what = cstate_history_what_develop_begin;

        //
        // Clear the build-time field.
        // Clear the test-time field.
        // Clear the test-baseline-time field.
        // Clear the src field.
        //
        change_build_times_clear(cp);

        //
        // Assign the new change to the user.
        //
        up->own_add(pp, change_number);

        //
        // Create the development directory.
        //
        up->become_begin();
        os_mkdir(devdir, 02755);
        undo_rmdir_bg(devdir);
        up->become_end();

        //
        // run the develop begin early command
        //
        cp->run_develop_begin_early_command(up);

        //
        // add all of the files to the new change
        // copy the files into the development directory
        //
        change_verbose(cp, 0, i18n("copy change source files"));
        for (j = 0;; ++j)
        {
            string_ty       *from;
            string_ty       *to;
            fstate_src_ty   *src_data;
            fstate_src_ty   *src_data2;
            fstate_src_ty   *p_src_data;

            //
            // find the file
            //
            // There are many files we will ignore.
            //
            src_data2 = change_file_nth(cp2, j, view_path_first);
            if (!src_data2)
                break;
            switch (src_data2->action)
            {
            case file_action_insulate:
                continue;

            case file_action_transparent:
                if (cp2->was_a_branch())
                    continue;
                break;

            case file_action_modify:
                switch (src_data2->usage)
                {
                case file_usage_build:
                    continue;

                case file_usage_source:
                case file_usage_config:
                case file_usage_test:
                case file_usage_manual_test:
                    break;
                }
                break;

            case file_action_create:
            case file_action_remove:
                break;
            }

            //
            // find the file in the project
            //
            p_src_data = pp->file_find(src_data2, view_path_extreme);
            if (!p_src_data)
            {
                switch (src_data2->action)
                {
                case file_action_remove:
                    continue;

                case file_action_transparent:
                case file_action_create:
                    break;

                case file_action_modify:
                case file_action_insulate:
#ifndef DEBUG
                default:
#endif
                    src_data2->action = file_action_create;
                    break;
                }
            }

            //
            // create the file in the new change
            //
            src_data = cp->file_new(src_data2);
            src_data->action = src_data2->action;
            change_file_copy_basic_attributes(src_data, src_data2);
            if (src_data2->move)
                src_data->move = str_copy(src_data2->move);

            switch (src_data->action)
            {
            case file_action_remove:
                //
                // removed files aren't copied,
                // they have whiteout instead.
                //
                change_file_whiteout_write(cp, src_data->file_name, up);
                assert(p_src_data->edit);
                assert(p_src_data->edit->revision);
                src_data->edit_origin = history_version_copy(p_src_data->edit);
                continue;

            case file_action_transparent:
                //
                // Transparent files are copied, but from the project's
                // parent, not from the other change.
                //
                // construct the paths to the files
                //
                from =
                    project_file_path(pp->parent_get(), src_data2);
                to = os_path_join(devdir, src_data2->file_name);

                //
                // copy the file
                //
                up->become_begin();
                os_mkdir_between(devdir, src_data2->file_name, 02755);
                if (os_exists(to))
                    os_unlink(to);
                copy_whole_file(from, to, 0);

                //
                // Set the file mode.
                //
                mode = 0444;
                if (p_src_data->executable)
                    mode |= 0111;
                mode &= ~the_umask;
                os_chmod(to, mode);
                up->become_end();

                //
                // If possible, use the edit number origin of the project
                // and copy the head revision number.
                //
                if (p_src_data && p_src_data->edit)
                {
                    src_data->edit_origin =
                        history_version_copy(p_src_data->edit);
                }

                //
                // Go on to the next file.
                //
                continue;

            case file_action_create:
            case file_action_modify:
            case file_action_insulate:
                break;
            }

            //
            // If the change has already been completed, get the
            // file from history, but if it is still active, get
            // the file from the old development directory.
            //
            if (cstate_data2->state == cstate_state_completed)
            {
                //
                // We could be creating the file, from the point
                // of view of this branch.
                //
                assert(src_data2->edit);
                assert(src_data2->edit->revision);
                if (p_src_data && src_data2->edit)
                {
                    src_data->edit_origin =
                        history_version_copy(src_data2->edit);
                }
                else if (p_src_data && p_src_data->edit)
                {
                    src_data->edit_origin =
                        history_version_copy(p_src_data->edit);
                }
                else
                    src_data->action = file_action_create;

                //
                // figure where to send it
                //
                to = os_path_join(devdir, src_data2->file_name);

                //
                // make sure there is a directory for it
                //
                up->become_begin();
                os_mkdir_between(devdir, src_data2->file_name, 02755);
                if (os_exists(to))
                    os_unlink(to);
                os_unlink_errok(to);
                up->become_end();

                //
                // get the file from history
                //
                change_run_history_get_command(cp2, src_data2, to, up);

                //
                // set the file mode
                //
                up->become_begin();
                mode = 0644;
                if (src_data2->executable)
                    mode |= 0111;
                mode &= ~the_umask;
                os_chmod(to, mode);
                up->become_end();

                //
                // clean up afterwards
                //
                str_free(to);
            }
            else
            {
                //
                // If possible, use the edit number origin of
                // the change we are cloning, this gives us the
                // best chance to merge correctly.
                //
                // Otherwise, see if the file exists in the
                // project and copy the head revision number
                //
                if (p_src_data && src_data2->edit_origin)
                {
                    src_data->edit_origin =
                        history_version_copy(src_data2->edit_origin);
                }
                else if (p_src_data && p_src_data->edit)
                {
                    src_data->edit_origin =
                        history_version_copy(p_src_data->edit);
                }
                else
                    src_data->action = file_action_create;
                if (p_src_data && src_data2->edit_origin_new)
                {
                    src_data->edit_origin_new =
                        history_version_copy(src_data2->edit_origin_new);
                }

                //
                // construct the paths to the files
                //
                from = cp2->file_path(src_data2->file_name);
                to = os_path_join(devdir, src_data2->file_name);

                //
                // copy the file
                //
                up->become_begin();
                os_mkdir_between(devdir, src_data2->file_name, 02755);
                if (os_exists(to))
                    os_unlink(to);
                copy_whole_file(from, to, 0);

                //
                // set the file mode
                //
                mode = 0644;
                if (os_executable(from))
                    mode |= 0111;
                mode &= ~the_umask;
                os_chmod(to, mode);
                up->become_end();

                //
                // clean up afterwards
                //
                str_free(from);
                str_free(to);
            }
        }
    }

    //
    // Write out the change file.
    // There is no need to lock this file
    // as it does not exist yet;
    // the project state file, with the number in it, is locked.
    //
    cp->cstate_write();

    //
    // Add the change to the list of existing changes.
    // Increment the next_change_number.
    // and write pstate back out.
    //
    project_change_append(pp, change_number, 0);

    //
    // If there is an --output option,
    // write the change number to the file.
    //
    if (output_filename)
    {
        string_ty *content =
            str_format("%ld", magic_zero_decode(change_number));
        if (*output_filename)
        {
            up->become_begin();
            string_ty *fn = str_from_c(output_filename);
            file_from_string(fn, content, 0644);
            str_free(fn);
            up->become_end();
        }
        else
            cat_string_to_stdout(content);
        str_free(content);
    }

    // remember that we are about to
    if (cstate_data2->state >= cstate_state_being_developed)
        cp->run_project_file_command_done();

    //
    // Write the change table row.
    // Write the user table rows.
    // Release advisory locks.
    //
    pp->pstate_write();
    up->ustate_write();
    commit();
    lock_release();

    if (cstate_data2->state >= cstate_state_being_developed)
    {
        //
        // run the develop begin command
        //
        cp->run_develop_begin_command(up);

        //
        // run the change file command
        // and the project file command if necessary
        //
        string_list_ty wl_nf;
        string_list_ty wl_nt;
        string_list_ty wl_cp;
        string_list_ty wl_rm;
        string_list_ty wl_mt;
        for (j = 0;; ++j)
        {
            fstate_src_ty   *c_src;

            c_src = change_file_nth(cp, j, view_path_first);
            if (!c_src)
                break;
            switch (c_src->action)
            {
            case file_action_create:
                switch (c_src->usage)
                {
                case file_usage_test:
                case file_usage_manual_test:
                    wl_nt.push_back(c_src->file_name);
                    break;

                case file_usage_source:
                case file_usage_config:
                case file_usage_build:
                    wl_nf.push_back(c_src->file_name);
                    break;
                }
                break;

            case file_action_modify:
            case file_action_insulate:
                wl_cp.push_back(c_src->file_name);
                break;

            case file_action_transparent:
                wl_mt.push_back(c_src->file_name);
                break;

            case file_action_remove:
                wl_rm.push_back(c_src->file_name);
                break;
            }
        }
        if (wl_nf.nstrings)
            cp->run_new_file_command(&wl_nf, up);
        if (wl_nt.nstrings)
            cp->run_new_test_command(&wl_nt, up);
        if (wl_cp.nstrings)
            cp->run_copy_file_command(&wl_cp, up);
        if (wl_rm.nstrings)
            cp->run_remove_file_command(&wl_rm, up);
        if (wl_mt.nstrings)
            cp->run_make_transparent_command(&wl_mt, up);

        cp->run_project_file_command(up);

        //
        // if symlinks are being used to pander to dumb DMT,
        // and they are not removed after each build,
        // create them now, rather than waiting for the first build.
        // This will present a more uniform interface to the developer.
        //
        pconf_data = change_pconf_get(cp, 0);
        assert(pconf_data->development_directory_style);
        if (!pconf_data->development_directory_style->during_build_only)
        {
            work_area_style_ty style = *pconf_data->development_directory_style;
            change_create_symlinks_to_baseline(cp, up, style);
        }
    }

    //
    // verbose success message
    //
    scp = sub_context_new();
    sub_var_set_string(scp, "ORiginal", cp2->version_get());
    sub_var_optional(scp, "ORiginal");
    change_verbose(cp, scp, i18n("clone complete"));
    sub_context_delete(scp);

    //
    // clean up and go home
    //
    change_free(cp);
    project_free(pp);
    trace(("}\n"));
}


void
aeclone(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
        { arglex_token_help, clone_help, 0 },
        { arglex_token_list, clone_list, 0 },
    };

    trace(("clone()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), clone_main);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
