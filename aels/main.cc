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

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>

#include <common/env.h>
#include <common/error.h>
#include <common/language.h>
#include <common/nstring/list.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/rsrc_limits.h>
#include <libaegis/arglex2.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/change.h>
#include <libaegis/change/identifier.h>
#include <libaegis/help.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>
#include <libaegis/version.h>
#include <libaegis/zero.h>

#include <aels/list.h>
#include <aels/stack.h>


enum
{
    arglex_token_attr_hide = ARGLEX2_MAX,
    arglex_token_attr_show,
    arglex_token_dot_files_hide,
    arglex_token_dot_files_show,
    arglex_token_group_hide,
    arglex_token_group_show,
    arglex_token_long,
    arglex_token_mode_hide,
    arglex_token_mode_show,
    arglex_token_recursive,
    arglex_token_size_hide,
    arglex_token_size_show,
    arglex_token_user_hide,
    arglex_token_user_show,
    arglex_token_when_hide,
    arglex_token_when_show,
    ARGLEX3_MAX
};

static arglex_table_ty argtab[] =
{
    { "-Hide_Attributes", arglex_token_attr_hide, },
    { "-Show_Attributes", arglex_token_attr_show, },
    { "-Hide_Dot_Files", arglex_token_dot_files_hide, },
    { "-ALL", arglex_token_dot_files_show, },
    { "-Show_Dot_Files", arglex_token_dot_files_show, },
    { "-Hide_Group", arglex_token_group_hide, },
    { "-Show_Group", arglex_token_group_show, },
    { "-LOng", arglex_token_long, },
    { "-Hide_Mode", arglex_token_mode_hide, },
    { "-Show_Mode", arglex_token_mode_show, },
    { "-RECursive", arglex_token_recursive, },
    { "-Hide_Size", arglex_token_size_hide, },
    { "-Show_Size", arglex_token_size_show, },
    { "-Hide_User", arglex_token_user_hide, },
    { "-Show_User", arglex_token_user_show, },
    { "-Hide_When", arglex_token_when_hide, },
    { "-Show_When", arglex_token_when_show, },
    ARGLEX_END_MARKER
};


static void
list_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf(stderr, "usage: %s [ <option>... ][ <filename>... ]\n", progname);
    fprintf(stderr, "       %s -Help\n", progname);
    quit(1);
}


static void
list_help(void)
{
    help((char *)0, list_usage);
}


int
main(int argc, char **argv)
{
    resource_limits_init();
    os_become_init_mortal();
    arglex2_init3(argc, argv, argtab);
    env_initialize();
    language_init();

    switch (arglex())
    {
    case arglex_token_help:
        list_help();
        quit(0);

    case arglex_token_version:
        version();
        quit(0);
    }

    change_identifier cid;
    nstring_list name;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(list_usage);
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
            cid.command_line_parse(list_usage);
            continue;

        case arglex_token_string:
            name.push_back(arglex_value.alv_string);
            break;

        case arglex_token_base_relative:
        case arglex_token_current_relative:
            user_ty::relative_filename_preference_argument(list_usage);
            break;

        case arglex_token_recursive:
            if (recursive_flag)
                    duplicate_option(list_usage);
            recursive_flag = 1;
            break;

        case arglex_token_long:
            if (long_flag)
                    duplicate_option(list_usage);
            long_flag = 1;
            break;

        case arglex_token_dot_files_show:
            show_dot_files = 1;
            break;

        case arglex_token_dot_files_hide:
            show_dot_files = 0;
            break;

        case arglex_token_mode_show:
            mode_flag = 1;
            break;

        case arglex_token_mode_hide:
            mode_flag = 0;
            break;

        case arglex_token_attr_show:
            attr_flag = 1;
            break;

        case arglex_token_attr_hide:
            attr_flag = 0;
            break;

        case arglex_token_user_show:
            user_flag = 1;
            break;

        case arglex_token_user_hide:
            user_flag = 0;
            break;

        case arglex_token_group_show:
            group_flag = 1;
            break;

        case arglex_token_group_hide:
            group_flag = 0;
            break;

        case arglex_token_size_show:
            size_flag = 1;
            break;

        case arglex_token_size_hide:
            size_flag = 0;
            break;

        case arglex_token_when_show:
            when_flag = 1;
            break;

        case arglex_token_when_hide:
            when_flag = 0;
            break;
        }
        arglex();
    }
    cid.command_line_check(list_usage);

    //
    // Figure out where to get the search path from.
    //
    if (cid.get_baseline())
    {
        stack_from_project(cid.get_pp());
    }
    else
    {
        if (cid.get_cp()->is_completed())
        {
            stack_from_project(cid.get_pp());
        }
        else
        {
            //
            // It is an error if the change is not in the
            // being_developed state (if it does not have a
            // directory).
            //
            if (cid.get_cp()->is_awaiting_development())
                change_fatal(cid.get_cp(), 0, i18n("bad aels state"));

            stack_from_change(cid.get_cp());
        }
    }

    //
    // resolve the path of each path
    // 1. the absolute path of the file name is obtained
    // 2. if the file is inside the development directory, ok
    // 3. if the file is inside the baseline, ok
    // 4. if neither, error
    //
    bool based =
        (
            cid.get_up()->relative_filename_preference
            (
                uconf_relative_filename_preference_current
            )
        ==
            uconf_relative_filename_preference_base
        );
    if (name.empty())
    {
        os_become_orig();
        name.push_back(nstring(os_curdir()));
        os_become_undo();
    }
    nstring_list base_relative_names;
    for (size_t j = 0; j < name.size(); ++j)
    {
        nstring s0 = name[j];
        if (based && s0[0] != '/')
            s0 = os_path_join(stack_nth(0), s0);
        nstring s2 = stack_relative(s0);
        if (s2.empty())
        {
            sub_context_ty sc;
            sc.var_set_string("File_Name", s0);
            if (cid.set())
                change_fatal(cid.get_cp(), &sc, i18n("$filename unrelated"));
            else
                project_fatal(cid.get_pp(), &sc, i18n("$filename unrelated"));
            // NOTREACHED
        }
        base_relative_names.push_back(s2);
    }
    base_relative_names.sort();

    //
    // emit the listing
    //
    list(base_relative_names, cid.get_pp(), cid.set() ? cid.get_cp() : 0);

    //
    // report success
    //
    quit(0);
    return 0;
}


// vim: set ts=8 sw=4 et :
