//
//      aegis - project change supervisor
//      Copyright (C) 2004-2008, 2011, 2012 Peter Miller
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

#include <common/ac/assert.h>

#include <common/error.h>
#include <common/trace.h>
#include <libaegis/arglex2.h>
#include <libaegis/help.h>
#include <libaegis/project.h>
#include <libaegis/project/identifi_sub/branch.h>


project_identifier_subset_branch::~project_identifier_subset_branch()
{
    trace(("~project_identifier_subset_branch()\n"));
}


project_identifier_subset_branch::project_identifier_subset_branch(
        project_identifier_subset &arg) :
    deeper(arg),
    pp_with_branch(0),
    branch(0),
    grandparent(false),
    trunk(false)
{
    trace(("project_identifier_subset_branch()\n"));
}


void
project_identifier_subset_branch::command_line_check(usage_t usage)
{
    deeper.command_line_check(usage);

    //
    // reject illegal combinations of options
    //
    if (grandparent)
    {
        if (branch)
        {
            mutually_exclusive_options
            (
                arglex_token_branch,
                arglex_token_grandparent,
                usage
            );
        }
        if (trunk)
        {
            mutually_exclusive_options
            (
                arglex_token_trunk,
                arglex_token_grandparent,
                usage
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
                usage
            );
        }
        branch = "";
    }
}


void
project_identifier_subset_branch::command_line_parse(usage_t usage)
{
    trace(("project_identifier_subset_branch::command_line_parse()\n{\n"));
    switch (arglex_token)
    {
    default:
        fatal_raw
        (
            "%s: %d: option %s not handled in switch (bug)",
            __FILE__,
            __LINE__,
            arglex_token_name(arglex_token)
        );
        // NOTREACHED

    case arglex_token_branch:
        if (branch)
            duplicate_option(usage);
        switch (arglex())
        {
        default:
            option_needs_number(arglex_token_branch, usage);

        case arglex_token_number:
        case arglex_token_string:
            branch = arglex_value.alv_string;
            break;
        }
        break;

    case arglex_token_trunk:
        if (trunk)
            duplicate_option(usage);
        trunk = true;
        break;

    case arglex_token_grandparent:
        if (grandparent)
            duplicate_option(usage);
        grandparent = true;
        break;

    case arglex_token_project:
    case arglex_token_string:
        deeper.command_line_parse(usage);
        trace(("}\n"));
        return;
    }
    arglex();
    trace(("}\n"));
}


void
project_identifier_subset_branch::parse_change_with_branch(long &change_number,
    usage_t usage)
{
    parse_change_with_branch(change_number, branch, usage);
}


void
project_identifier_subset_branch::parse_change_with_branch(long &change_number,
    const char *&branch_arg, usage_t usage)
{
    deeper.parse_change_with_branch
    (
        change_number,
        branch_arg,
        usage
    );
}


user_ty::pointer
project_identifier_subset_branch::get_up()
{
    return deeper.get_up();
}

void
project_identifier_subset_branch::set_user_by_name(nstring &login)
{
    deeper.set_user_by_name(login);
}


project *
project_identifier_subset_branch::get_pp()
{
    if (!pp_with_branch)
    {
        pp_with_branch = deeper.get_pp();
        assert(pp_with_branch);
        if (branch)
        {
            pp_with_branch = pp_with_branch->find_branch(branch);
            assert(pp_with_branch);
        }
    }
    return pp_with_branch;
}


bool
project_identifier_subset_branch::set()
    const
{
    return deeper.set();
}


// vim: set ts=8 sw=4 et :
