//
// aegis - project change supervisor
// Copyright (C) 2004-2008, 2012 Peter Miller
// Copyright (C) 2008 Walter Franzini
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

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>

#include <aediff/arglex3.h>
#include <aediff/diff.h>
#include <common/nstring/list.h>
#include <common/progname.h>
#include <common/trace.h>
#include <libaegis/arglex/project.h>
#include <libaegis/change/file.h>
#include <libaegis/change/functor.h>
#include <libaegis/change/identifi_sub.h>
#include <libaegis/file_revision.h>
#include <libaegis/help.h>
#include <libaegis/option.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/project/identifi_sub/branch.h>
#include <libaegis/project/identifi_sub/plain.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>


void
diff_usage()
{
    const char *progname = progname_get();
    fprintf(stderr, "Usage: %s [ <option>... ] <filename>\n", progname);
    fprintf(stderr, "       %s --list [ <option>... ]\n", progname);
    fprintf(stderr, "       %s --help\n", progname);
    exit(1);
}


class aediff_bad_state:
    public change_functor
{
public:
    aediff_bad_state() :
        change_functor(true)
    {
    }

    void
    operator()(change::pointer cp)
    {
        change_fatal(cp, 0, i18n("bad patch send state"));
    }
};

static aediff_bad_state barf_adev;


static const char *
ae2diff(const char *dflt)
{
    const char *ep = getenv("AE2DIFF");
    if (!ep || !*ep)
        return dflt;
    return ep;
}


void
diff()
{
    trace(("diff()\n{\n"));
    nstring base_command(ae2diff(CONF_DIFF));
    nstring filename;
    project_identifier_subset_plain pid;
    project_identifier_subset_branch first_branch(pid);
    change_identifier_subset first(first_branch);
    project_identifier_subset_branch second_branch(pid);
    change_identifier_subset second(second_branch);
    int context = 0;
    int unified = 0;
    bool text = false;
    bool ignore_all_space = false;
    bool ignore_blank_lines = false;
    bool ignore_case = false;
    bool ignore_space_change = false;
    bool show_c_function = false;
    arglex();
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(diff_usage);
            continue;

        case arglex_token_project:
        case arglex_token_baseline:
        case arglex_token_branch:
        case arglex_token_change:
        case arglex_token_delta:
        case arglex_token_delta_date:
        case arglex_token_delta_from_change:
        case arglex_token_grandparent:
        case arglex_token_number:
        case arglex_token_trunk:
            if (!first.set())
                first.command_line_parse(diff_usage);
            else
                second.command_line_parse(diff_usage);
            continue;

        case arglex_token_string:
            if (filename)
                fatal_too_many_files();
            filename = arglex_value.alv_string;
            break;

        case arglex_token_context:
            if (context)
                duplicate_option(diff_usage);
            if (arglex() != arglex_token_number)
            {
                context = 3;
                continue;
            }
            context = arglex_value.alv_number;
            if (context < 1)
                context = 1;
            break;

        case arglex_token_unified:
            if (unified)
                duplicate_option(diff_usage);
            if (arglex() != arglex_token_number)
            {
                unified = 3;
                continue;
            }
            unified = arglex_value.alv_number;
            if (unified < 1)
                unified = 1;
            break;

        case arglex_token_text:
            if (text)
                duplicate_option(diff_usage);
            text = true;
            break;

        case arglex_token_ignore_all_space:
            if (ignore_all_space)
                duplicate_option(diff_usage);
            ignore_all_space = true;
            break;

        case arglex_token_ignore_blank_lines:
            if (ignore_blank_lines)
                duplicate_option(diff_usage);
            ignore_blank_lines = true;
            break;

        case arglex_token_ignore_case:
            if (ignore_case)
                duplicate_option(diff_usage);
            ignore_case = true;
            break;

        case arglex_token_ignore_space_change:
            if (ignore_space_change)
                duplicate_option(diff_usage);
            ignore_space_change = true;
            break;

        case arglex_token_show_c_function:
            if (show_c_function)
                duplicate_option(diff_usage);
            show_c_function = true;
            break;

        case arglex_token_command:
            if (arglex() != arglex_token_string)
                option_needs_string(arglex_token_command, diff_usage);
            base_command = arglex_value.alv_string;
            break;
        }
        arglex();
    }
    if (context && unified)
    {
        mutually_exclusive_options
        (
            arglex_token_context,
            arglex_token_unified,
            diff_usage
        );
    }
    if (!filename)
    {
        error_intl(0, i18n("too few files named"));
        diff_usage();
    }

    //
    // Default a few things if they gave zero or one change, rather than two.
    //
    if (!first.set() && !second.set())
        first.set_baseline();

    //
    // reject illegal combinations of options
    //
    first.command_line_check(diff_usage);
    second.command_line_check(diff_usage);

    //
    // Get the two revisions of the file.
    //
    // We get the meta-data for the file, so we have the UUID, if the
    // file has one.  We then use the meta-data to obtain the first and
    // second versions of the file.
    //
    // We look for meta-data separately to cope with a bug that make
    // it possible for two files to have the same name but different
    // UUIDs.
    //
    int number_of_errors = 0;
    fstate_src_ty *src1 = first.get_cp()->file_find(filename, view_path_simple);
    if (!src1)
    {
        src1 = first.get_cp()->file_find_fuzzy(filename, view_path_simple);
        if (src1)
        {
            sub_context_ty sc;
            sc.var_set_string("File_Name", filename);
            sc.var_set_string("Guess", src1->file_name);
            change_error
            (
                first.get_cp(),
                &sc,
                i18n("no $filename, closest is $guess")
            );
        }

        sub_context_ty sc;
        sc.var_set_string("File_Name", filename);
        change_error(first.get_cp(), &sc, i18n("no $filename"));
        ++number_of_errors;
    }

    fstate_src_ty *src2 =
        second.get_cp()->file_find(filename, view_path_simple);
    if (!src2)
    {
        src2 = second.get_cp()->file_find_fuzzy(filename, view_path_simple);
        if (src2)
        {
            sub_context_ty sc;
            sc.var_set_string("File_Name", filename);
            sc.var_set_string("Guess", src2->file_name);
            change_error
            (
                second.get_cp(),
                &sc,
                i18n("no $filename, closest is $guess")
            );
        }

        sub_context_ty sc;
        sc.var_set_string("File_Name", filename);
        change_error(second.get_cp(), &sc, i18n("no $filename"));
        ++number_of_errors;
    }

    if (number_of_errors > 0)
    {
        sub_context_ty sc;
        sc.var_set_long("Number", number_of_errors);
        sc.var_optional("Number");
        project_fatal(second.get_pp(), &sc, i18n("no files diffed"));
    }

    file_revision lhs = first.get_file_revision(src1, barf_adev);
    trace(("lhs=%s\n", lhs.get_path().c_str()));

    file_revision rhs = second.get_file_revision(src2, barf_adev);
    trace(("rhs=%s\n", rhs.get_path().c_str()));

    //
    // Build the command to be executed.
    //
    nstring_list command_args;
    command_args.push_back("set +e ;");

    command_args.push_back(base_command);
    if (base_command == CONF_DIFF)
    {
        if (text)
            command_args.push_back("--text");
        if (context)
            command_args.push_back(nstring::format("-C%d", context));
        else if (unified)
            command_args.push_back(nstring::format("-U%d", unified));
#ifdef HAVE_GNU_DIFF
        if (context || unified)
        {
            nstring label = first.get_change_version_string() + "/" + filename;
            command_args.push_back("--label=" + label.quote_shell());
            label = second.get_change_version_string() + "/" + filename;
            command_args.push_back("--label=" + label.quote_shell());
        }
#endif
        if (ignore_all_space)
            command_args.push_back("--ignore-all-space");
        if (ignore_blank_lines)
            command_args.push_back("--ignore-blank-lines");
        if (ignore_case)
            command_args.push_back("--ignore-case");
        if (ignore_space_change)
            command_args.push_back("--ignore-space-change");
        if (show_c_function)
            command_args.push_back("--show-c-function");
    }

    command_args.push_back(lhs.get_path().quote_shell());
    command_args.push_back(rhs.get_path().quote_shell());
    command_args.push_back("; test $? -le 1");

    nstring command = command_args.unsplit(" ");

    //
    // Execute the command.
    //
    trace(("command = \"%s\"\n", command.c_str()));
    int flags = OS_EXEC_FLAG_NO_INPUT;
    if (!option_verbose_get())
        flags |= OS_EXEC_FLAG_SILENT;
    os_become_orig();
    os_execute(command.get_ref(), flags, os_curdir());
    os_become_undo();
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
