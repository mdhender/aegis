//
// aegis - project change supervisor
// Copyright (C) 2011 Peter Miller
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
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>

#include <common/env.h>
#include <common/language.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/rsrc_limits.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/help.h>
#include <libaegis/os.h>
#include <libaegis/version.h>

#include <aegrep/arglex3.h>


static void
grep_usage(void)
{
    const char *progname = progname_get();
    fprintf(stderr, "Usage: %s [ <option>... ] <pattern>\n", progname);
    fprintf(stderr, "       %s -Help\n", progname);
    quit(1);
}


static void
grep_help(void)
{
    help((char *)0, grep_usage);
}


int
main(int argc, char **argv)
{
    resource_limits_init();
    os_become_init_mortal();
    arglex3_init(argc, argv);
    env_initialize();
    language_init();

    switch (arglex())
    {
    case arglex_token_help:
        grep_help();
        quit(0);

    case arglex_token_version:
        version();
        quit(0);
    }

    change_identifier cid;
    nstring pattern;
    nstring grep_options;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(grep_usage);
            continue;

        case arglex_token_baseline:
        case arglex_token_branch:
        case arglex_token_change:
        case arglex_token_delta:
        case arglex_token_delta_date:
        case arglex_token_delta_from_change:
        case arglex_token_development_directory:
        case arglex_token_grandparent:
        case arglex_token_number:
        case arglex_token_project:
        case arglex_token_trunk:
            cid.command_line_parse(grep_usage);
            continue;

        case arglex_token_string:
            if (!pattern.empty())
                grep_usage();
            pattern = arglex_value.alv_string;
            break;

        case arglex_token_base_relative:
        case arglex_token_current_relative:
            user_ty::relative_filename_preference_argument(grep_usage);
            break;

        case arglex_token_after_context:
            if (arglex() != arglex_token_string)
                option_needs_string(arglex_token_after_context, grep_usage);
            grep_options += " --after-context="
                + nstring(arglex_value.alv_string).quote_shell();
            break;

        case arglex_token_basic_regexp:
            grep_options += " --basic-regexp";
            break;

        case arglex_token_before_context:
            if (arglex() != arglex_token_string)
                option_needs_string(arglex_token_before_context, grep_usage);
            grep_options += " --before-context="
                + nstring(arglex_value.alv_string).quote_shell();
            break;

        case arglex_token_binary:
            grep_options += " --binary";
            break;

        case arglex_token_binary_files:
            if (arglex() != arglex_token_string)
                option_needs_string(arglex_token_binary_files, grep_usage);
            grep_options += " --binary-files="
                + nstring(arglex_value.alv_string).quote_shell();
            break;

        case arglex_token_byte_offset:
            grep_options += " --byte-offset";
            break;

        case arglex_token_color:
            grep_options += " --color";
            break;

        case arglex_token_context:
            if (arglex() != arglex_token_number)
                option_needs_number(arglex_token_context, grep_usage);
            grep_options +=
                nstring::format(" --context=%ld", arglex_value.alv_number);
            break;

        case arglex_token_count:
            grep_options += " --count";
            break;

        case arglex_token_extended_regexp:
            grep_options += " --extended-regexp";
            break;

        case arglex_token_files_with_matches:
            grep_options += " --files-with-matches";
            break;

        case arglex_token_files_without_matches:
            grep_options += " --files-without-match"; // [sic]
            break;

        case arglex_token_fixed_strings:
            grep_options += " --fixed-strings";
            break;

        case arglex_token_ignore_case:
            grep_options += " --ignore-case";
            break;

        case arglex_token_invert_match:
            grep_options += " --invert-match";
            break;

        case arglex_token_initial_tab:
            grep_options += " --initial-tab";
            break;

        case arglex_token_line_buffered:
            grep_options += " --line-buffered";
            break;

        case arglex_token_line_number:
            grep_options += " --line-number";
            break;

        case arglex_token_line_regexp:
            grep_options += " --line-regexp";
            break;

        case arglex_token_maximum_count:
            if (arglex() != arglex_token_number)
                option_needs_number(arglex_token_maximum_count, grep_usage);
            grep_options +=
                nstring::format(" --max-count=%ld", arglex_value.alv_number);
            break;

        case arglex_token_no_messages:
            grep_options += " --no-messages";
            break;

        case arglex_token_null:
            grep_options += " --null";
            break;

        case arglex_token_null_data:
            grep_options += " --null-data";
            break;

        case arglex_token_only_matching:
            grep_options += " --only-matching";
            break;

        case arglex_token_perl_regexp:
            grep_options += " --perl-regexp";
            break;

        case arglex_token_quiet:
            grep_options += " --quiet";
            break;

        case arglex_token_text:
            grep_options += " --text";
            break;

        case arglex_token_unix_byte_offsets:
            grep_options += " --unix-byte-offsets";
            break;

        case arglex_token_word_regexp:
            grep_options += " --word-regexp";
            break;

        case arglex_token_list:
            grep_options += " --files-with-matches";
            break;

        // FIXME: --exclude
        // FIXME: --include
        }
        arglex();
    }
    cid.command_line_check(grep_usage);
    if (pattern.empty())
        grep_usage();

    //
    // Grope each source file.
    //
    for (size_t j = 0; ; ++j)
    {
        fstate_src_ty *src =
            change_file_nth(cid.get_cp(), j, view_path_extreme);
        if (!src)
            break;
        nstring fn(src->file_name);
        int unlink_p = 0;
        nstring rfn(change_file_version_path(cid.get_cp(), src, &unlink_p));
        nstring command =
            "cat " + rfn.quote_shell() + " | grep --label=" + fn.quote_shell()
            + " -H " + grep_options + nstring(pattern[0] == '-' ? "-e" : "")
            + " " + pattern.quote_shell();
        // Note: we deliberately don't echo the command.
        int n = system(command.c_str());
        if (unlink_p)
            os_unlink_errok(rfn);
        if (!WIFEXITED(n) || WEXITSTATUS(n) >= 2)
            quit(1);
    }

    //
    // report success
    //
    quit(0);
    return 0;
}
