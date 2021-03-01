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
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/arglex2.h>
#include <libaegis/help.h>
#include <libaegis/project.h>
#include <libaegis/project/identifi_sub/plain.h>
#include <libaegis/user.h>


project_identifier_subset_plain::~project_identifier_subset_plain()
{
    trace(("~project_identifier_subset_plain()\n{\n"));
    pp = 0;
    if (project_name)
    {
        str_free(project_name);
        project_name = 0;
    }
    trace(("}\n"));
}


project_identifier_subset_plain::project_identifier_subset_plain() :
    project_name(0),
    pp(0)
{
    trace(("project_identifier_subset_plain()\n"));
}


project *
project_identifier_subset_plain::get_pp()
{
    if (!pp)
    {
        //
        // locate project data
        //
        assert(!pp);
        if (!project_name)
        {
            nstring n = user_ty::create()->default_project();
            project_name = str_copy(n.get_ref());
        }
        pp = project_alloc(project_name);
        pp->bind_existing();
        assert(pp);
    }
    return pp;
}


user_ty::pointer
project_identifier_subset_plain::get_up()
{
    if (!up)
    {
        up = user_ty::create();
        assert(up);
    }
    return up;
}

void
project_identifier_subset_plain::set_user_by_name(nstring &login)
{
    //
    // If the user was already set, it will be silently replaced.
    // There are no known cases where an error message would be more
    // appropriate.
    //
    up = user_ty::create(login);
}


void
project_identifier_subset_plain::command_line_parse(void (*usage)(void))
{
    trace(("project_identifier_subset_plain::command_line_parse()\n{\n"));
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
    case arglex_token_trunk:
    case arglex_token_grandparent:
        bad_argument(usage);
        // NOTREACHED

    case arglex_token_project:
        arglex();
        // fall through...

    case arglex_token_string:
        arglex_parse_project(&project_name, usage);
        trace(("}\n"));
        return;
    }
    arglex();
    trace(("}\n"));
}


void
project_identifier_subset_plain::command_line_check(void (*)(void))
{
}


void
project_identifier_subset_plain::parse_change_with_branch(long &change_number,
    usage_t usage)
{
    arglex_parse_change(&project_name, &change_number, usage);
}


void
project_identifier_subset_plain::parse_change_with_branch(long &change_number,
    const char *&branch, usage_t usage)
{
    arglex_parse_change_with_branch
    (
        &project_name,
        &change_number,
        &branch,
        usage
    );
}


bool
project_identifier_subset_plain::set()
    const
{
    return !!project_name;
}


// vim: set ts=8 sw=4 et :
