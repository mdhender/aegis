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
// MANIFEST: implementation of the change_id class
//

#pragma implementation "change_id"

#include <arglex2.h>
#include <arglex/change.h>
#include <change.h>
#include <change/branch.h>
#include <change/file.h>
#include <change_id.h>
#include <error.h> // assert
#include <help.h>
#include <gettime.h>
#include <now.h>
#include <project.h>
#include <project/history.h>
#include <project/file.h>
#include <project/file/roll_forward.h>
#include <sub.h>
#include <trace.h>
#include <user.h>


change_id::~change_id()
{
    trace(("change_id::~change_id()\n{\n"));
    if (cp)
    {
	change_free(cp);
	cp = 0;
    }
    pp = 0;
    if (project_name);
    {
	str_free(project_name);
	project_name = 0;
    }
    trace(("}\n"));
}


change_id::change_id() :
    baseline(false),
    branch(0),
    change_number(0),
    delta_date(NO_TIME_SET),
    delta_number(-1),
    grandparent(false),
    trunk(false),
    project_name(0),
    pp(0),
    cp(0)
{
}


bool
change_id::set()
    const
{
    trace(("change_id::set()\n"));
    return
	(
	    baseline
	||
	    change_number != 0
	||
	    delta_date != NO_TIME_SET
	||
	    !delta_name.empty()
	||
	    delta_number != -1
	);
}


void
change_id::command_line_parse(void (*usage)(void))
{
    trace(("change_id::command_line_parse()\n{\n"));
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

    case arglex_token_baseline:
	if (baseline)
	    duplicate_option(usage);
	baseline = true;
	break;

    case arglex_token_change:
    case arglex_token_delta_from_change:
	arglex();
	// fall through...

    case arglex_token_number:
	arglex_parse_change_with_branch
	(
	    &project_name,
	    &change_number,
	    &branch,
	    usage
	);
	break;

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

    case arglex_token_delta:
	if (delta_number >= 0 || delta_name)
	    duplicate_option(usage);
	switch (arglex())
	{
	default:
	    option_needs_number(arglex_token_delta, usage);
	    // NOTREACHED

	case arglex_token_number:
	    delta_number = arglex_value.alv_number;
	    if (delta_number < 0)
	    {
		sub_context_ty *scp = sub_context_new();
		sub_var_set_long(scp, "Number", delta_number);
		fatal_intl(scp, i18n("delta $number out of range"));
		// NOTREACHED
		sub_context_delete(scp);
	    }
	    break;

	case arglex_token_string:
	    delta_name = arglex_value.alv_string;
	    break;
	}
	break;

    case arglex_token_delta_date:
	if (delta_date != NO_TIME_SET)
	    duplicate_option(usage);
	if (arglex() != arglex_token_string)
	{
	    option_needs_string(arglex_token_delta_date, usage);
	    // NOTREACHED
	}
	delta_date = date_scan(arglex_value.alv_string);
	if (delta_date == NO_TIME_SET)
	    fatal_date_unknown(arglex_value.alv_string);
	break;
    }
    arglex();
    trace(("}\n"));
}


void
change_id::command_line_check(void (*usage)(void))
{
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
    if
    (
	(
	    !!baseline
	+
	    (!delta_name.empty() || delta_number >= 0)
	+
	    !!change_number
	+
	    (delta_date != NO_TIME_SET)
	)
    >
	1
    )
    {
	mutually_exclusive_options3
	(
	    arglex_token_baseline,
	    arglex_token_change,
	    arglex_token_delta,
	    usage
	);
    }
}


void
change_id::set_project_name(string_ty *arg)
{
    if (!project_name)
	project_name = str_copy(arg);
}


void
change_id::set_project(void)
{
    //
    // locate project data
    //
    if (!project_name)
	project_name = user_default_project();
    pp = project_alloc(project_name);
    project_bind_existing(pp);
    if (branch)
        pp = project_find_branch(pp, branch);
}


void
change_id::set_change(void (*usage)(void))
{
    trace(("change_id::set_change()\n{\n"));
    if (baseline)
    {
	cp = project_change_get(pp);
	trace(("baseline\n"));
	trace(("}\n"));
	return;
    }

    //
    // it is an error if the delta does not exist
    //
    if (!delta_name.empty())
    {
	change_number =
	    project_history_change_by_name(pp, delta_name.get_ref(), 0);
	trace(("delta \"%s\" == change %ld\n", delta_name.c_str(),
	    change_number));
	delta_name = "";
    }
    if (delta_date != NO_TIME_SET)
    {
	//
	// If the time is in the future, you could get a different
	// answer for the same input at some point in the future.
	//
	// This is the "time safe" quality first described by
	// Damon Poole <damon@ede.com>
	//
	if (delta_date > now())
	    project_error(pp, 0, i18n("date in the future"));

	//
    	// Now find the change number corresponding.
	//
	change_number = project_history_change_by_timestamp(pp, delta_date);
	trace(("delta date %ld == change %ld\n", (long)delta_date,
	    change_number));
    }
    if (delta_number >= 0)
    {
	// does not return if no such delta number
	change_number = project_history_change_by_delta(pp, delta_number);
	trace(("delta %ld == change %ld\n", delta_number, change_number));
	delta_number = 0;
    }

    //
    // locate change data
    //
    if (!change_number)
    {
	user_ty *up = user_executing(pp);
	change_number = user_default_change(up);
	trace(("change %ld\n", change_number));
    }
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);
    trace(("}\n"));
}


file_version
change_id::get_file_version(const nstring &filename)
{
    //
    // Check the change state.
    //
    trace(("change_id::get_file_version(\"%s\")\n{\n", filename.c_str()));
    cstate_ty *cstate_data = change_cstate_get(cp);
    fstate_src_ty *src = 0;
    switch (cstate_data->state)
    {
#ifndef DEBUG
    default:
#endif
    case cstate_state_awaiting_development:
	change_fatal(cp, 0, i18n("bad patch send state"));

    case cstate_state_completed:
	{
	    //
	    // Need to reconstruct the appropriate file histories.
	    //
	    trace(("project = \"%s\"\n", project_name_get(pp)->str_text));
	    project_file_roll_forward
		historian
		(
		    pp,
		    (
			delta_date != NO_TIME_SET
		    ?
			delta_date
		    :
			change_completion_timestamp(cp)
		    ),
		    0
		);
	    file_event_ty *fep = historian.get_last(filename.get_ref());
	    if (!fep)
	    {
		//
		// The file doesn't exist yet at this
		// delta.  Omit it.
		//
	       	trace(("}\n"));
		return file_version("/dev/null", false);
	    }
	    src =
		change_file_find(fep->cp, filename.get_ref(), view_path_first);
	}
	break;

    case cstate_state_being_integrated:
    case cstate_state_awaiting_integration:
    case cstate_state_being_reviewed:
    case cstate_state_awaiting_review:
    case cstate_state_being_developed:
	trace(("mark\n"));
	src = change_file_find(cp, filename.get_ref(), view_path_simple);
	if (!src)
	{
	    sub_context_ty *scp = sub_context_new();
	    src = change_file_find_fuzzy(cp, filename.get_ref());
	    if (!src)
	    {
		src =
		    project_file_find_fuzzy
		    (
			pp,
			filename.get_ref(),
			view_path_extreme
		    );
	    }
	    sub_var_set_string(scp, "File_Name", filename.get_ref());
	    if (src)
	    {
		sub_var_set_string(scp, "Guess", src->file_name);
		change_fatal(cp, scp, i18n("no $filename, closest is $guess"));
	    }
	    else
	    {
		change_fatal(cp, scp, i18n("no $filename"));
	    }
	}
	break;
    }
    trace(("src = %08lX\n", (long)src));
    assert(src);
    switch (src->action)
    {
    case file_action_remove:
	//
	// The file had been removed at this
	// delta.  Omit it.
	//
	trace(("}\n"));
	return file_version("/dev/null", false);

    case file_action_create:
    case file_action_modify:
    case file_action_insulate:
    case file_action_transparent:
	break;
    }
    int from_unlink = 0;
    trace(("edit = %s\n", src->edit->revision->str_text));
    nstring from = change_file_version_path(cp, src, &from_unlink);
    trace(("}\n"));
    return file_version(from, from_unlink);
}
