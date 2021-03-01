//
// aegis - project change supervisor
// Copyright (C) 2006-2008, 2011, 2012 Peter Miller
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

#include <common/error.h>
#include <common/progname.h>
#include <common/trace.h>
#include <libaegis/attribute.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/help.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/project/file.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>

#include <ae-repo-ci/arglex3.h>
#include <ae-repo-ci/checkin.h>
#include <ae-repo-ci/repository.h>


void
checkin_usage()
{
    const char *progname = progname_get();
    fprintf(stderr, "Usage: %s [ <option>... ] <filename>\n", progname);
    fprintf(stderr, "       %s --list [ <option>... ]\n", progname);
    fprintf(stderr, "       %s --help\n", progname);
    exit(1);
}


static string_ty *
project_specific_find(change_identifier &cid, const char *name)
{
    pconf_ty *pconf_data = change_pconf_get(cid.get_cp(), 0);
    assert(pconf_data);
    attributes_ty *psp =
        attributes_list_find(pconf_data->project_specific, name);
    if (!psp)
        return 0;
    assert(psp->value);
    return psp->value;
}


void
checkin()
{
    trace(("checkin()\n{\n"));
    repository *rp = 0;
    change_identifier cid;
    arglex();
    nstring module;
    nstring directory;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(checkin_usage);
            continue;

        case arglex_token_project:
        case arglex_token_string:
        case arglex_token_change:
        case arglex_token_number:
            cid.command_line_parse(checkin_usage);
            continue;

        case arglex_token_module:
            if (!module.empty())
                duplicate_option(checkin_usage);
            if (arglex() != arglex_token_string)
                option_needs_string(arglex_token_module, checkin_usage);
            module = arglex_value.alv_string;
            break;

        case arglex_token_directory:
            if (!directory.empty())
                duplicate_option(checkin_usage);
            if (arglex() != arglex_token_string)
                option_needs_dir(arglex_token_directory, checkin_usage);
            directory = arglex_value.alv_string;
            break;

        case arglex_token_repository:
            if (rp)
                duplicate_option(checkin_usage);
            if (arglex() != arglex_token_string)
                option_needs_string(arglex_token_repository, checkin_usage);
            rp = repository::factory(arglex_value.alv_string);
            break;
        }
        arglex();
    }
    if (!rp)
        error_raw("no repository type specified");

    //
    // reject illegal combinations of options
    //
    cid.command_line_check(checkin_usage);

    if (!cid.get_cp()->is_completed())
    {
        fatal_raw("change wrong state");
    }
    trace(("change number = %ld\n", cid.get_change_number()));
    rp->change_specific_attributes(cid.get_cp());

    //
    // Default the module name to the name of the project
    // without version (branch) numbers.
    //
    if (module.empty())
    {
        module = nstring(cid.get_pp()->parent_get()->name_get());
    }
    trace(("module = %s\n", module.quote_c().c_str()));

    //
    // create a work area that we can make the duplicate changes to
    //
    os_become_orig();
    rp->checkout(module, directory);
    os_become_undo();

    //
    // apply each file to the work area
    //
    for (size_t j = 0; ; ++j)
    {
        fstate_src_ty *src = change_file_nth(cid.get_cp(), j, view_path_first);
        if (!src)
            break;
        rp->file_wrapper(cid, src);
    }

    //
    // build a commit message
    //
    string_ty *msg1 = project_specific_find(cid, "ae-repo-ci:commit-message");
    if (!msg1)
        msg1 = str_from_c("$version - ${change brief_description}");
    sub_context_ty sc;
    string_ty *msg2 = sc.substitute(cid.get_cp(), msg1);
    str_free(msg1);
    msg1 = 0;
    nstring message(msg2);
    msg2 = 0;

    //
    // commit all the duplicate changes to the duplicate repository
    //
    os_become_orig();
    rp->commit(message);
    delete rp;
    rp = 0;
    os_become_undo();

    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
