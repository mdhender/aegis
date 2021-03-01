//
// aegis - project change supervisor
// Copyright (C) 2006-2008, 2011, 2012 Peter Miller;
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License, version 3, as
// published by the Free Software Foundation.
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
#include <common/ac/unistd.h>

#include <common/env.h>
#include <common/language.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/rsrc_limits.h>
#include <common/sizeof.h>
#include <common/trace.h>
#include <libaegis/arglex2.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/help.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/safe_freopen.h>
#include <libaegis/version.h>
#include <libaegis/view_path.h>

#include <aemakegen/debug.h>
#include <aemakegen/flavour.h>
#include <aemakegen/target.h>

enum
{
    arglex_token_debug = ARGLEX2_MAX,
    arglex_token_script,
    arglex_token_target,
    arglex_token_target_list,
    arglex_token_flavour,
    arglex_token_flavour_list,
    ARGLEX3_MAX
};

static arglex_table_ty argtab[] =
{
    { "-Debug", arglex_token_debug, },
    { "-FLavour", arglex_token_flavour, },
    { "-List_Flavour", arglex_token_flavour_list, },
    { "-List_Target", arglex_token_target_list, },
    { "-SCRipt", arglex_token_script, },
    { "-TArget", arglex_token_target, },
    ARGLEX_END_MARKER
};


static void
usage(void)
{
    const char *prog = progname_get();
    fprintf(stderr, "Usage: %s [ <option>... ] <filename>...\n", prog);
    fprintf(stderr, "       %s --version\n", prog);
    quit(1);
}


static void
main_help(void)
{
    help(0, usage);
}


static void
list_flavours(void)
{
    while (arglex_token != arglex_token_eoln)
    {
        generic_argument(usage);
        arglex();
    }
    flavour::list();
}


static void
list_targets(void)
{
    while (arglex_token != arglex_token_eoln)
    {
        generic_argument(usage);
        arglex();
    }
    target::list();
}


static void
generate(void)
{
    change_identifier cid;
    nstring_list files;
    nstring target_name;
    nstring flavour_name;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        case arglex_token_baseline:
        case arglex_token_branch:
        case arglex_token_change:
        case arglex_token_delta:
        case arglex_token_delta_date:
        case arglex_token_delta_from_change:
        case arglex_token_delta_name:
        case arglex_token_grandparent:
        case arglex_token_number:
        case arglex_token_project:
            cid.command_line_parse(usage);
            continue;

        case arglex_token_script:
            if (arglex() != arglex_token_string)
                option_needs_file(arglex_token_script, usage);
            target::script(arglex_value.alv_string);
            break;

        case arglex_token_string:
            files.push_back(arglex_value.alv_string);
            break;

        case arglex_token_flavour:
            if (arglex() != arglex_token_string)
                option_needs_string(arglex_token_flavour, usage);
            flavour_name = arglex_value.alv_string;
            break;

        case arglex_token_target:
            if (arglex() != arglex_token_string)
                option_needs_string(arglex_token_target, usage);
            target_name = arglex_value.alv_string;
            break;

        case arglex_token_output:
            if (arglex() != arglex_token_string)
                option_needs_file(arglex_token_output, usage);
            safe_freopen(arglex_value.alv_string, "w", stdout);
            break;

        case arglex_token_trace:
            for (;;)
            {
                if (arglex() != arglex_token_string)
                    break;
                trace_enable(arglex_value.alv_string);
            }
            trace(("enabled\n"));
            continue;

        case arglex_token_debug:
            ++debug;
            break;

        default:
            generic_argument(usage);
        }
        arglex();
    }
    cid.command_line_check(usage);

    //
    // Create the processing instance.  This is how we differentiate
    // Makefile.in files from RPM .spec files, etc.
    //
    target::pointer tp = target::create(target_name, cid);

    //
    // Set up the filtering.
    // It is an error to fail to call this method.
    //
    tp->set_flavour(flavour_name);

    //
    // Add all the change set's files to the list of files to be
    // processed.  This way, only generated files need to be added to
    // the command line.
    //
    change::pointer cp = cid.get_cp();
    for (size_t j = 0; ; ++j)
    {
        fstate_src_ty *src = change_file_nth(cp, j, view_path_extreme);
        if (!src)
            break;
        files.push_back(nstring(src->file_name));
    }

    //
    // Run the processing selected.
    //
    if (files.empty())
        usage();
    files.sort();
    tp->process(files);
}


int
main(int argc, char **argv)
{
    resource_limits_init();
    os_become_init_mortal();
    arglex2_init3(argc, argv, argtab);
    env_initialize();
    language_init();

    static const arglex_dispatch_ty choices[] =
    {
        { arglex_token_help,         main_help,     0 },
        { arglex_token_version,      version,       0 },
        { arglex_token_flavour_list, list_flavours, 0 },
        { arglex_token_target_list,  list_targets,  0 },
    };

    arglex();
    arglex_dispatch(choices, SIZEOF(choices), generate);
    return 0;
}


// vim: set ts=8 sw=4 et :
