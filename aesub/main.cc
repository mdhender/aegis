//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2001-2008, 2012 Peter Miller
//      Copyright (C) 2008 Walter Franzini
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

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>

#include <common/env.h>
#include <common/language.h>
#include <common/nstring/list.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/rsrc_limits.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/arglex2.h>
#include <libaegis/change.h>
#include <libaegis/change/identifier.h>
#include <libaegis/file.h>
#include <libaegis/help.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>
#include <libaegis/version.h>


//
// NAME
//      aesub_usage
//
// SYNOPSIS
//      void aesub_usage(void);
//
// DESCRIPTION
//      The aesub_usage function is used to
//      briefly describe how to used the 'aegis -RePorT' command.
//

static void
aesub_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf(stderr, "usage: %s [ <option>... ] <string>...\n", progname);
    fprintf(stderr, "       %s -List [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -Help\n", progname);
    quit(1);
}


//
// NAME
//      aesub_help
//
// SYNOPSIS
//      void aesub_help(void);
//
// DESCRIPTION
//      The aesub_help function is used to
//      describe in detail how to use the `aesub' command.
//

static void
aesub_help(void)
{
    help(0, aesub_usage);
}


//
// NAME
//      aesub_main
//
// SYNOPSIS
//      void aesub_main(void);
//
// DESCRIPTION
//      The aesub_main function is used to substiture the strings on the
//      command line, using the usual Aegis substitutions, and then echo
//      them to stdout.
//

static void
aesub_main(void)
{
    trace(("aesub_main()\n{\n"));
    nstring_list arg;
    change_identifier cid;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(aesub_usage);
            continue;

        case arglex_token_baseline:
        case arglex_token_change:
        case arglex_token_branch:
        case arglex_token_trunk:
        case arglex_token_grandparent:
        case arglex_token_delta:
        case arglex_token_delta_name:
        case arglex_token_project:
            cid.command_line_parse(aesub_usage);
            continue;

        case arglex_token_string:
        case arglex_token_number:
            arg.push_back(nstring(arglex_value.alv_string));
            break;

        case arglex_token_file:
            switch (arglex())
            {
            default:
                option_needs_file(arglex_token_file, aesub_usage);
                // NOTREACHED

            case arglex_token_string:
                {
                    os_become_orig();
                    nstring s(read_whole_file(arglex_value.alv_string));
                    os_become_undo();
                    arg.push_back(s);
                }
                break;

            case arglex_token_stdio:
                {
                    os_become_orig();
                    nstring s(read_whole_file((char *)0));
                    os_become_undo();
                    arg.push_back(s);
                }
                break;
            }
            break;
        }
        arglex();
    }
    cid.command_line_check(aesub_usage);

    //
    //make the substitutions and print out the strings
    //
    if (cid.get_baseline())
    {
        for (size_t j = 0; j < arg.size(); ++j)
        {
            if (j)
                putchar(' ');
            sub_context_ty sc("command line", j + 1);
            nstring text(sc.substitute_p(cid.get_pp(), arg[j].get_ref()));
            fwrite(text.c_str(), 1, text.size(), stdout);
        }
    }
    else
    {
        for (size_t j = 0; j < arg.size(); ++j)
        {
            if (j)
                putchar(' ');
            sub_context_ty sc("command line", j + 1);
            nstring text(sc.substitute(cid.get_cp(), arg[j].get_ref()));
            fwrite(text.c_str(), 1, text.size(), stdout);
        }
    }

    if (!arg.empty())
        putchar('\n');
    trace(("}\n"));
}


//
// NAME
//      aesub
//
// SYNOPSIS
//      void aesub(void);
//
// DESCRIPTION
//      The aesub function is used to
//      dispatch the 'aegis -RePorT' command to the relevant functionality.
//      Where it goes depends on the command line.
//

int
main(int argc, char **argv)
{
    resource_limits_init();
    os_become_init_mortal();
    arglex2_init(argc, argv);
    env_initialize();
    language_init();
    switch (arglex())
    {
    default:
        aesub_main();
        break;

    case arglex_token_help:
        aesub_help();
        break;

    case arglex_token_version:
        version_copyright();
        break;
    }
    quit(0);
    return 0;
}


// vim: set ts=8 sw=4 et :
