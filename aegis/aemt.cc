//
// aegis - project change supervisor
// Copyright (C) 2002-2009, 2011, 2012 Peter Miller
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
#include <common/ac/stdio.h>

#include <common/progname.h>
#include <common/quit.h>
#include <common/sizeof.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/ael/project/files.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/arglex2.h>
#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/commit.h>
#include <libaegis/file.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/log.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/project/file.h>
#include <libaegis/search_path/base_get.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>

#include <aegis/aemt.h>


//
// NAME
//      make_transparent_usage
//
// SYNOPSIS
//      void make_transparent_usage(void);
//
// DESCRIPTION
//      The make_transparent_usage function is used to
//      tell the user how to use the 'aegis -Make_Transparent' command.
//

static void
make_transparent_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
        stderr,
        "usage: %s -Make_Transparent <filename>... [ <option>... ]\n",
        progname
    );
    fprintf
    (
        stderr,
        "       %s -Make_Transparent -List [ <option>... ]\n",
        progname
    );
    fprintf(stderr, "       %s -Make_Transparent -Help\n", progname);
    quit(1);
}


//
// NAME
//      make_transparent_help
//
// SYNOPSIS
//      void make_transparent_help(void);
//
// DESCRIPTION
//      The make_transparent_help function is used to
//      describe the 'aegis -Make_Transparent' command to the user.
//

static void
make_transparent_help(void)
{
    help("aemt", make_transparent_usage);
}


//
// NAME
//      make_transparent_list
//
// SYNOPSIS
//      void make_transparent_list(void);
//
// DESCRIPTION
//      The make_transparent_list function is used to
//      list the file the user may wish to add to the change
//      as a transparency.  All project files are listed.
//

static void
make_transparent_list(void)
{
    trace(("make_transparent_list()\n{\n"));
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(make_transparent_usage);
    list_project_files(cid, 0);
    trace(("}\n"));
}


//
// NAME
//      make_transparent_main
//
// SYNOPSIS
//      void make_transparent_main(void);
//
// DESCRIPTION
//      The make_transparent_main function is used to add a file to a
//      change as a transparency (like aecpu for a branch).  The file
//      will be elided from the branch on successful integration of
//      the change.
//
//      The names of the relevant files are gleaned from the command line.
//

static void
make_transparent_main(void)
{
    string_ty       *s1;
    string_ty       *s2;
    fstate_src_ty   *c_src_data;
    fstate_src_ty   *b_src_data;
    size_t          j;
    size_t          k;
    string_ty       *project_name;
    project      *pp;
    long            change_number;
    change::pointer cp;
    change::pointer bcp;
    log_style_ty    log_style;
    user_ty::pointer up;
    string_ty       *dd;
    int             number_of_errors;
    string_list_ty  search_path;
    sub_context_ty  *scp;
    int             unchanged;

    trace(("make_transparent_main()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    string_list_ty wl;
    log_style = log_style_append_default;
    unchanged = 0;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(make_transparent_usage);
            continue;

        case arglex_token_file:
        case arglex_token_directory:
            if (arglex() != arglex_token_string)
                make_transparent_usage();
            // fall through...

        case arglex_token_string:
            s2 = str_from_c(arglex_value.alv_string);
            wl.push_back(s2);
            str_free(s2);
            break;

        case arglex_token_change:
            arglex();
            // fall through...

        case arglex_token_number:
            arglex_parse_change
            (
                &project_name,
                &change_number,
                make_transparent_usage
            );
            continue;

        case arglex_token_project:
            arglex();
            arglex_parse_project(&project_name, make_transparent_usage);
            continue;

        case arglex_token_nolog:
            if (log_style == log_style_none)
                duplicate_option(make_transparent_usage);
            log_style = log_style_none;
            break;

        case arglex_token_unchanged:
            if (unchanged)
                duplicate_option(make_transparent_usage);
            unchanged = 1;
            break;

        case arglex_token_wait:
        case arglex_token_wait_not:
            user_ty::lock_wait_argument(make_transparent_usage);
            break;

        case arglex_token_whiteout:
        case arglex_token_whiteout_not:
            user_ty::whiteout_argument(make_transparent_usage);
            break;

        case arglex_token_base_relative:
        case arglex_token_current_relative:
            user_ty::relative_filename_preference_argument
            (
                make_transparent_usage
            );
            break;
        }
        arglex();
    }
    if (!unchanged && !wl.nstrings)
    {
        error_intl(0, i18n("no file names"));
        make_transparent_usage();
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
    // take the locks and read the change state
    //
    change_cstate_lock_prepare(cp);
    lock_take();

    log_open(change_logfile_get(cp), up, log_style);

    //
    // It is an error if the change is not in the in_development state.
    // It is an error if the change is not assigned to the current user.
    //
    if (!cp->is_being_developed())
        change_fatal(cp, 0, i18n("make transparent bad state"));
    if (cp->is_a_branch())
        change_fatal(cp, 0, i18n("bad nf branch"));
    if (nstring(cp->developer_name()) != up->name())
        change_fatal(cp, 0, i18n("not developer"));

    //
    // If no files were named and the -unchanged option was used,
    // add all of the project's unchanged files in the change.
    // It is an error if there are none.
    //
    if (!wl.nstrings)
        wl.push_back(change_development_directory_get(cp, 1));

    //
    // Where to search to resolve file names.
    //
    cp->search_path_get(&search_path, true);

    //
    // Find the base for relative filenames.
    //
    nstring base(search_path_base_get(search_path, up));

    //
    // resolve the path of each file
    // 1.   the absolute path of the file name is obtained
    // 2.   if the file is inside the development directory, ok
    // 3.   if the file is inside the baseline, ok
    // 4.   if neither, error
    //
    bcp = pp->change_get();
    number_of_errors = 0;
    string_list_ty wl2;
    for (j = 0; j < wl.nstrings; ++j)
    {
        s1 = wl.string[j];
        if (s1->str_text[0] == '/')
            s2 = str_copy(s1);
        else
            s2 = os_path_join(base.get_ref(), s1);
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
            scp = sub_context_new();
            sub_var_set_string(scp, "File_Name", wl.string[j]);
            change_error(cp, scp, i18n("$filename unrelated"));
            sub_context_delete(scp);
            ++number_of_errors;
            continue;
        }
        string_list_ty wl_in;
        change_file_directory_query(bcp, s2, &wl_in, 0);
        if (wl_in.nstrings)
        {
            int             used;

            //
            // If the user named a directory, add all of the
            // source files in that directory, provided they
            // are not already in the change.
            //
            used = 0;
            for (k = 0; k < wl_in.nstrings; ++k)
            {
                string_ty       *s3;

                s3 = wl_in.string[k];
                if (!cp->file_find(nstring(s3), view_path_first))
                {
                    if (wl2.member(s3))
                    {
                        scp = sub_context_new();
                        sub_var_set_string(scp, "File_Name", s3);
                        change_error(cp, scp, i18n("too many $filename"));
                        sub_context_delete(scp);
                        ++number_of_errors;
                    }
                    else
                        wl2.push_back(s3);
                    ++used;
                }
            }
            if (!used)
            {
                scp = sub_context_new();
                if (s2->str_length)
                    sub_var_set_string(scp, "File_Name", s2);
                else
                    sub_var_set_charstar(scp, "File_Name", ".");
                sub_var_set_long(scp, "Number", (long)wl_in.nstrings);
                sub_var_optional(scp, "Number");
                change_error
                (
                    cp,
                    scp,
                    i18n("directory $filename contains no relevant files")
                );
                sub_context_delete(scp);
                ++number_of_errors;
            }
        }
        else
        {
            if (wl2.member(s2))
            {
                scp = sub_context_new();
                sub_var_set_string(scp, "File_Name", s2);
                change_error(cp, scp, i18n("too many $filename"));
                sub_context_delete(scp);
                ++number_of_errors;
            }
            else
                wl2.push_back(s2);
        }
        str_free(s2);
    }
    wl = wl2;

    //
    // ensure that each file
    // 1. is not already part of the change
    // 2. is being modified or removed by the branch
    // add it to the change
    //
    for (j = 0; j < wl.nstrings; ++j)
    {
        fstate_src_ty   *pp_src_data;

        s1 = wl.string[j];
        trace(("s1 = \"%s\"\n", s1->str_text));
        if (cp->file_find(nstring(s1), view_path_first))
        {
            scp = sub_context_new();
            sub_var_set_string(scp, "File_Name", s1);
            change_error(cp, scp, i18n("file $filename dup"));
            sub_context_delete(scp);
            ++number_of_errors;
            continue;
        }
        b_src_data = bcp->file_find(nstring(s1), view_path_first);
        if (!b_src_data)
        {
            b_src_data = bcp->file_find_fuzzy(nstring(s1), view_path_first);
            if (b_src_data)
            {
                scp = sub_context_new();
                sub_var_set_string(scp, "File_Name", s1);
                sub_var_set_string(scp, "Guess", b_src_data->file_name);
                change_error(bcp, scp, i18n("no $filename, closest is $guess"));
                sub_context_delete(scp);
                ++number_of_errors;
            }
            else
            {
                scp = sub_context_new();
                sub_var_set_string(scp, "File_Name", s1);
                if
                (
                    !pp->is_a_trunk()
                &&
                    pp->parent_get()->file_find(s1, view_path_simple)
                )
                {
                    project_error
                    (
                        pp->parent_get(),
                        scp,
                        i18n("make $filename transparent, fail, too deep")
                    );
                }
                else
                {
                    change_error(bcp, scp, i18n("no $filename"));
                }
                sub_context_delete(scp);
                ++number_of_errors;
                continue;
            }
        }
        assert(b_src_data);

        pp_src_data = 0;
        switch (b_src_data->action)
        {
        case file_action_create:
        case file_action_modify:
        case file_action_remove:
            if (pp->is_a_trunk())
            {
                if (unchanged)
                {
                    trace(("not this one\n"));
                    continue;
                }

                //
                // You may not make a trunk file transparent.
                // What would you see through it?
                //
                scp = sub_context_new();
                sub_var_set_string(scp, "File_Name", s1);
                change_error
                (
                    cp,
                    scp,
                    i18n("make $filename transparent, fail, trunk")
                );
                sub_context_delete(scp);
                ++number_of_errors;
                continue;
            }

            pp_src_data = pp->parent_get()->file_find(s1, view_path_simple);
            if (!pp_src_data)
            {
                if (unchanged)
                {
                    trace(("not this one\n"));
                    continue;
                }
                scp = sub_context_new();
                sub_var_set_string(scp, "File_Name", s1);
                project_error
                (
                    pp->parent_get(),
                    scp,
                    i18n("make $filename transparent, fail, too shallow")
                );
                sub_context_delete(scp);
                ++number_of_errors;
                continue;
            }
            break;

        case file_action_transparent:
        case file_action_insulate:
            // not possible
            trace(("not possible\n"));
            assert(0);
            continue;
        }
        assert(pp_src_data);

        if (unchanged)
        {
            //
            // It isn't sufficient to compare versions strings, because
            // not everyone configures their history commands to avoid
            // unnecessary checkins.  We have to actually compare the
            // two files.
            //
            string_ty       *f1_path;
            int             f1_unlink;
            string_ty       *f2_path;
            int             f2_unlink;
            int             different;

            trace(("mark\n"));
            f1_path = project_file_version_path(pp, b_src_data, &f1_unlink);
            f2_path =
                project_file_version_path
                (
                    pp->parent_get(),
                    pp_src_data,
                    &f2_unlink
                );
            up->become_begin();
            different = files_are_different(f1_path, f2_path);
            if (f1_unlink)
                os_unlink(f1_path);
            if (f2_unlink)
                os_unlink(f2_path);
            up->become_end();

            str_free(f1_path);
            str_free(f2_path);
            if (different)
            {
                trace(("not this one, it's different\n"));
                continue;
            }
        }

        c_src_data = cp->file_new(pp_src_data);
        c_src_data->action = file_action_transparent;

        //
        // b_src_data->edit_number
        //      The head revision of the branch.
        // b_src_data->edit_number_origin
        //      The version originally copied.
        //
        // c_src_data->edit_number
        //      Not meaningful until after integrate pass.
        // c_src_data->edit_number_origin
        //      The version originally copied.
        // c_src_data->edit_number_origin_new
        //      Updates branch edit_number_origin on
        //      integrate pass.
        //
        assert(b_src_data->edit);
        assert(b_src_data->edit->revision);
        c_src_data->edit_origin = history_version_copy(b_src_data->edit);
    }
    if (number_of_errors)
    {
        scp = sub_context_new();
        sub_var_set_long(scp, "Number", number_of_errors);
        sub_var_optional(scp, "Number");
        change_fatal(cp, scp, i18n("remove file fail"));
        // NOTREACHED
        sub_context_delete(scp);
    }

    //
    // Create each file in the development directory.  If the file exists
    // in the grandparent project, copy it from there, otherwise put
    // a whiteout file there.  Create any necessary directories along
    // the way.
    //
    dd = change_development_directory_get(cp, 0);
    for (j = 0; ; ++j)
    {
        fstate_src_ty   *pp_src_data;

        c_src_data = change_file_nth(cp, j, view_path_first);
        if (!c_src_data)
            break;
        assert(pp->parent_get());
        pp_src_data =
            pp->parent_get()->file_find
            (
                c_src_data->file_name,
                view_path_extreme
            );
        if (pp_src_data)
        {
            string_ty       *from;
            string_ty       *to;
            int             mode;

            from = project_file_path(pp->parent_get(), c_src_data->file_name);
            assert(from);
            to = os_path_join(dd, c_src_data->file_name);

            //
            // copy the file
            //
            user_ty::become scoped(up);
            os_mkdir_between(dd, c_src_data->file_name, 02755);
            if (os_exists(to))
                os_unlink(to);
            copy_whole_file(from, to, 0);

            //
            // set the file mode
            //
            mode = 0444;
            if (pp_src_data->executable)
                mode |= 0111;
            mode &= ~cp->umask_get();
            os_chmod(to, mode);

            //
            // clean up afterwards
            //
            str_free(from);
            str_free(to);
        }
        else
        {
            //
            // Remove any existing file (this cleans up junk, and
            // breaks the link if we are using symlink trees).
            //
            change_file_whiteout_write(cp, c_src_data->file_name, up);
        }
    }

    //
    // the number of files changed, or the version did,
    // so stomp on the validation fields.
    //
    change_build_times_clear(cp);

    //
    // If the file manifest of the change is altered (e.g. by aenf, aenfu,
    // aecp, aecpu, etc), or the contents of any file is changed, the
    // UUID is cleared.  This is because it is no longer the same change
    // as was received by aedist or aepatch, and the UUID is invalidated.
    //
    change_uuid_clear(cp);

    // remember that we are about to
    bool recent_integration = cp->run_project_file_command_needed();
    if (recent_integration)
        cp->run_project_file_command_done();

    //
    // write the data and release the lock
    //
    cp->cstate_write();
    commit();
    lock_release();

    //
    // run the change file command
    // and the project file command if necessary
    //
    cp->run_make_transparent_command(&wl, up);
    if (recent_integration)
        cp->run_project_file_command(up);

    //
    // verbose success message
    //
    scp = sub_context_new();
    sub_var_set_long(scp, "Number", (long)wl.nstrings);
    sub_var_optional(scp, "Number");
    change_verbose(cp, scp, i18n("make transparent complete"));
    sub_context_delete(scp);

    change_free(cp);
    project_free(pp);
    trace(("}\n"));
}


//
// NAME
//      make_transparent
//
// SYNOPSIS
//      void make_transparent(void);
//
// DESCRIPTION
//      The make_transparent function is used to
//      dispatch the 'aegis -Make_Transparent' command to the relevant
//      function to do it's work.
//

void
make_transparent(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
        { arglex_token_help, make_transparent_help, 0 },
        { arglex_token_list, make_transparent_list, 0 },
    };

    trace(("make_transparent()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), make_transparent_main);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
