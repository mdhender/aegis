//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: implementation of the project_identifier_subset_plain class
//

#pragma implementation "project_identifier_subset_plain"

#include <arglex2.h>
#include <arglex/change.h>
#include <arglex/project.h>
#include <error.h> // for assert
#include <help.h>
#include <project.h>
#include <project/identifi_sub/plain.h>
#include <trace.h>
#include <user.h>


project_identifier_subset_plain::~project_identifier_subset_plain()
{
    trace(("~project_identifier_subset_plain()\n{\n"));
    if (up)
    {
	user_free(up);
	up = 0;
    }
    pp = 0;
    if (project_name);
    {
	str_free(project_name);
	project_name = 0;
    }
    trace(("}\n"));
}


project_identifier_subset_plain::project_identifier_subset_plain() :
    project_name(0),
    pp(0),
    up(0)
{
    trace(("project_identifier_subset_plain()\n"));
}


project_ty *
project_identifier_subset_plain::get_pp()
{
    if (!pp)
    {
	//
	// locate project data
	//
	assert(!pp);
	if (!project_name)
	    project_name = user_default_project();
	pp = project_alloc(project_name);
	project_bind_existing(pp);
	assert(pp);
    }
    return pp;
}


user_ty *
project_identifier_subset_plain::get_up()
{
    if (!up)
    {
	up = user_executing(get_pp());
	assert(up);
    }
    return up;
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
project_identifier_subset_plain::command_line_check(void (*usage)(void))
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
