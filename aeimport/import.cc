//
// aegis - project change supervisor
// Copyright (C) 2001-2008, 2011, 2012 Peter Miller
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

#include <common/now.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/sizeof.h>
#include <common/trace.h>
#include <libaegis/ael/project/projects.h>
#include <libaegis/arglex/project.h>
#include <libaegis/change.h>
#include <libaegis/change/identifier.h>
#include <libaegis/commit.h>
#include <libaegis/file.h>
#include <libaegis/gonzo.h>
#include <libaegis/help.h>
#include <libaegis/io.h>
#include <libaegis/lock.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/project/pattr/set.h>
#include <libaegis/project/verbose.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>
#include <libaegis/version.h>
#include <libaegis/zero.h>

#include <aeimport/arglex3.h>
#include <aeimport/change_set/find.h>
#include <aeimport/change_set/list.h>
#include <aeimport/config_file.h>
#include <aeimport/format.h>
#include <aeimport/format/search_list.h>
#include <aeimport/import.h>
#include <aeimport/reconstruct.h>
#include <aeimport/synthesize.h>


//
// NAME
//      import_usage
//
// SYNOPSIS
//      void import_usage(void);
//
// DESCRIPTION
//      The import_usage function is used to
//      briefly describe how to used the `aeimport' command.
//

static void
import_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf(stderr, "usage: %s [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -Help\n", progname);
    quit(1);
}


static void
import_list(void)
{
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(import_usage);
    list_projects(cid, 0);
}


//
// NAME
//      import_help
//
// SYNOPSIS
//      void import_help(void);
//
// DESCRIPTION
//      The import_help function is used to
//      describe in detail how to use the 'aegis -RePorT' command.
//

static void
import_help(void)
{
    help("aeimport", import_usage);
}


static string_ty *
work_out_config_name(format_search_list_ty *fslp)
{
    string_ty       *s;
    int             j;

    s = str_from_c(THE_CONFIG_FILE_NEW);
    if (!format_search_list_query(fslp, s))
        return s;
    str_free(s);

    for (j = 1; ; ++j)
    {
        s = str_format("aegis.%d.conf", j);
        if (!format_search_list_query(fslp, s))
            return s;
        str_free(s);
    }
}


//
// NAME
//      import_main
//
// SYNOPSIS
//      void import_main(void);
//
// DESCRIPTION
//      The import_main function is used to
//      import a change in the "being developed" or "being integrated" states.
//      It extracts what to do from the command line.
//

static void
import_main(void)
{
    sub_context_ty  *scp;
    string_ty       *project_name;
    project         *pp;
    user_ty::pointer up;
    string_ty       *bl;
    string_ty       *hp;
    string_ty       *ip;
    long            version_number[10];
    int             version_number_length;
    project         *version_pp[SIZEOF(version_number)];
    string_ty       *version_string;
    size_t          j;
    pattr_ty        *pattr_data;
    const char      *format_name =  0;
    format_ty       *format;
    format_search_list_ty *fslp;
    string_ty       *source_directory;
    change_set_list_ty *cslp;
    project         *ppp;
    int             mode;
    string_ty       *cfg;

    trace(("import_main()\n{\n"));
    project_name = 0;
    nstring home;
    version_number_length = 0;
    version_string = 0;
    source_directory = 0;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(import_usage);
            continue;

        case arglex_token_project:
            arglex();
            arglex_parse_project(&project_name, import_usage);
            continue;

        case arglex_token_directory:
            if (home)
                duplicate_option(import_usage);
            if (arglex() != arglex_token_string)
            {
                option_needs_dir(arglex_token_directory, import_usage);
            }

            //
            // To cope with automounters, directories are stored as
            // given, or are derived from the home directory in the
            // passwd file.  Within aegis, pathnames have their
            // symbolic links resolved, and any comparison of paths
            // is done on this "system idea" of the pathname.
            //
            home = arglex_value.alv_string;
            break;

        case arglex_token_version:
            if (version_string)
                duplicate_option(import_usage);
            switch (arglex())
            {
            default:
                option_needs_number(arglex_token_version, import_usage);

            case arglex_token_number:
            case arglex_token_string:
                version_string = str_from_c(arglex_value.alv_string);
                break;

            case arglex_token_stdio:
                version_string = str_from_c("");
                break;
            }
            break;

        case arglex_token_string:
            if (source_directory)
                fatal_too_many_files();
            source_directory = str_from_c(arglex_value.alv_string);
            break;

        case arglex_token_format:
            if (format_name)
                duplicate_option(import_usage);
            if (arglex() != arglex_token_string)
            {
                option_needs_name(arglex_token_format, import_usage);
            }
            format_name = arglex_value.alv_string;
            break;
        }
        arglex();
    }
    if (!project_name)
    {
        error_intl(0, i18n("no project name"));
        import_usage();
    }
    extract_version_from_project_name
    (
        &project_name,
        version_number,
        (int)SIZEOF(version_number),
        &version_number_length
    );
    if (!project_name_ok(project_name))
        fatal_bad_project_name(project_name);
    if (version_string)
    {
        if (version_string->str_length)
        {
            int             err;

            err =
                break_up_version_string
                (
                    version_string->str_text,
                    version_number,
                    (int)SIZEOF(version_number),
                    &version_number_length,
                    0
                );
            if (err)
            {
                scp = sub_context_new();
                sub_var_set_string(scp, "Number", version_string);
                fatal_intl(scp, i18n("bad version $number"));
                // NOTREACHED
                sub_context_delete(scp);
            }
        }
    }
    else if (!version_number_length)
    {
        version_number[0] = 1;
        version_number[1] = 0;
        version_number_length = 2;
    }

    //
    // Make sure we understand the file format specified.
    //
    format = format_find(format_name);

    // --------------------------------------------------------------

    //
    // Walk down the source directory tree looking for history files.
    //
    error_intl(0, i18n("read history files"));
    if (!source_directory)
        source_directory = str_from_c(".");
    os_become_orig();
    fslp = format_search(format, source_directory);
    os_become_undo();

    //
    // It is an error if no history files are found.
    //
    if (!fslp->length)
        fatal_intl(0, i18n("no history files found"));

    //
    // Extract the list of staff.
    //
    // This is used to populate the developer, reviewer and integrator
    // lists for the project.
    //
    string_list_ty staff;
    format_search_list_staff(fslp, &staff);

    //
    // Work out the name to use for the project configuration file.
    //
    cfg = work_out_config_name(fslp);

    // --------------------------------------------------------------

    //
    // Grope the file version information in order to produce a list
    // of change sets.
    //
    error_intl(0, i18n("find change sets"));
    cslp = change_set_find(fslp);

    //
    // Now set the current time to just before the first change set
    // discovered.
    //
    if (cslp->length)
    {
        now_set(cslp->item[0]->when - 60);
        now_unclearable();
    }

    // --------------------------------------------------------------

    //
    // Build some attributes.
    //
    pattr_data = (pattr_ty *)pattr_type.alloc();
    pattr_data->description =
        str_format("The \"%s\" program.", project_name->str_text);
    pattr_data->reuse_change_numbers = true;
    pattr_data->mask |= pattr_reuse_change_numbers_mask;
    pattr_data->developers_may_create_changes = true;
    pattr_data->mask |= pattr_developers_may_create_changes_mask;
    pattr_data->developer_may_review = (staff.nstrings == 1);
    pattr_data->mask |= pattr_developer_may_review_mask;
    pattr_data->developer_may_integrate = true;
    pattr_data->mask |= pattr_developer_may_integrate_mask;
    pattr_data->reviewer_may_integrate = true;
    pattr_data->mask |= pattr_reviewer_may_integrate_mask;
    pattr_data->minimum_change_number = 10;
    pattr_data->mask |= pattr_minimum_change_number_mask;
    pattr_data->minimum_branch_number = 1;
    pattr_data->mask |= pattr_minimum_branch_number_mask;
    pattr_data->default_test_exemption = true;
    pattr_data->mask |= pattr_default_test_exemption_mask;
    pattr_data->default_test_regression_exemption = true;
    pattr_data->mask |= pattr_default_test_regression_exemption_mask;

    //
    // locate user data
    //
    up = user_ty::create();

    //
    // read in the project table
    //
    gonzo_gstate_lock_prepare_new();
    lock_take();

    //
    // make sure not too privileged
    //
    if (!up->check_uid())
        fatal_user_too_privileged(up->name());
    if (!up->check_gid())
        fatal_group_too_privileged(up->get_group_name());

    //
    // it is an error if the name is already in use
    //
    if (gonzo_alias_to_actual(project_name))
        fatal_project_alias_exists(project_name);
    pp = project_alloc(project_name);
    str_free(project_name);
    pp->bind_new();

    //
    // The user who ran the command is the project administrator.
    //
    project_administrator_add(pp, up->name());

    //
    // Add the staff to the project.
    //
    // All staff found are automatically authorised as developers,
    // reviewers and integrators.
    //
    // Usually we wouldn't authorise an administrator, except in the
    // case where exactly one staff member is found, when it's pretty
    // clear who should be an administrator.
    //
    if (staff.nstrings == 1 && nstring(staff.string[0]) != up->name())
        project_administrator_add(pp, nstring(staff.string[0]));
    for (j = 0; j < staff.nstrings; ++j)
    {
        project_developer_add(pp, nstring(staff.string[j]));
        project_reviewer_add(pp, nstring(staff.string[j]));
        project_integrator_add(pp, nstring(staff.string[j]));
    }

    //
    // If no project directory was specified
    // create the directory in their home directory.
    //
    if (!home)
    {
        nstring s1 = up->default_project_directory();
        assert(s1);
        os_become_orig();
        int name_max = os_pathconf_name_max(s1);
        os_become_undo();
        if ((int)project_name_get(pp).length() > name_max)
            fatal_project_name_too_long
            (
                project_name_get(pp).get_ref(),
                name_max
            );
        home = os_path_cat(s1, nstring(project_name_get(pp)));

        project_verbose_directory(pp, home.get_ref());
    }
    pp->home_path_set(home);

    //
    // Create the directory and subdirectories.
    // It is an error if the directories can't be created.
    //
    home = nstring(pp->home_path_get());
    bl = pp->baseline_path_get();
    hp = pp->history_path_get();
    ip = pp->info_path_get();

    project_become(pp);
    os_mkdir(home, 02755);
    undo_rmdir_errok(home);
    os_mkdir(bl, 02755);
    undo_rmdir_errok(bl);
    os_mkdir(hp, 02755);
    undo_rmdir_errok(hp);
    os_mkdir(ip, 02755);
    undo_rmdir_errok(ip);
    project_become_undo(pp);

    //
    // add a row to the table
    //
    gonzo_project_add(pp);

    //
    // copy the attributes into the project
    //
    project_pattr_set(pp, pattr_data);
    pattr_type.free(pattr_data);

    //
    // create each of the branches
    //
    ppp = pp;
    for (j = 0; j < (size_t)version_number_length; ++j)
    {
        long            change_number;

        trace(("ppp = %8.8lX\n", (long)ppp));
        change_number = magic_zero_encode(version_number[j]);
        trace(("change_number = %ld;\n", change_number));
        ppp = project_new_branch(ppp, up, change_number);
        version_pp[j] = ppp;
    }

    //
    // write the project state
    //      (the trunk change state is implicitly written)
    //
    // Write each of the branch state.  You must write *after* the
    // next branch down is created, because creating a branch alters
    // pstate.
    //
    pp->pstate_write();
    for (j = 0; j < (size_t)version_number_length; ++j)
        version_pp[j]->pstate_write();
    gonzo_gstate_write();

    //
    // release locks
    //
    commit();
    lock_release();

    //
    // verbose success message
    //
    project_verbose_new_project_complete(pp);
    for (j = 0; j < (size_t)version_number_length; ++j)
        project_verbose_new_branch_complete(version_pp[j]);

    // --------------------------------------------------------------

    //
    // Copy all of the files into the history directory.
    //
    project_error(pp, 0, i18n("copy history files"));
    project_become(pp);
    mode = 0755 & ~project_umask_get(pp);
    for (j = 0; j < fslp->length; ++j)
    {
        format_search_ty *fsp;
        string_ty       *tail;
        string_ty       *dst;

        fsp = fslp->item[j];
        tail = os_below_dir(source_directory, fsp->filename_physical);
        string_ty *sanitized_tail = format_sanitize(format, tail, 0);
        dst = os_path_cat(hp, sanitized_tail);
        os_mkdir_between(hp, sanitized_tail, mode);
        str_free(tail);
        str_free(sanitized_tail);
        copy_whole_file(fsp->filename_physical, dst, 1);

        //
        // Also need to make sure the copied file is unlocked,
        // if this format supports the concept of locking.
        //
        format_unlock(format, dst);
        str_free(dst);
    }
    project_become_undo(pp);

    //
    // after this point, we do not use fslp any more
    //
    format_search_list_delete(fslp);
    fslp = 0;

    //
    // After this point, we just use the terminal branch.
    //
    ppp = pp;
    if (version_number_length)
    {
        ppp = version_pp[version_number_length - 1];
        project_free(pp);
        pp = 0;
        for (j = 0; j < (size_t)version_number_length - 1; j++)
        {
            project_free(version_pp[j]);
            version_pp[j] = 0;
        }
    }
    project_name = project_name_get(ppp).get_ref_copy();
    //    ppp = 0;

    // --------------------------------------------------------------

    //
    // The first change creates the project configuration file.  This is
    // necessary so that we can do all of the commits and produce
    // a complete project at the end of the process.
    //
    config_file(project_name, format, now(), cfg);

    // experimental
    project_free(ppp);
    ppp = 0;
    //
    // Create a change for each change set
    //
    for (j = 0; j < cslp->length; ++j)
        synthesize(project_name, cslp->item[j]);

    // the change set list will not be used below this point, so free
    // it here
    change_set_list_delete(cslp);

    // --------------------------------------------------------------

    //
    // Check out a copy of each file into the baseline,
    // and produce a diff file to go with it.
    //
    trace(("mark\n"));
    reconstruct(project_name);
    trace(("}\n"));
    str_free(project_name);
}


void
import(void)
{
    switch (arglex())
    {
    default:
        import_main();
        break;

    case arglex_token_help:
        import_help();
        break;

    case arglex_token_list:
        import_list();
        break;

    case arglex_token_version:
        version();
        break;
    }
}


// vim: set ts=8 sw=4 et :
