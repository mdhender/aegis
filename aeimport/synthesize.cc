//
//	aegis - project change supervisor
//	Copyright (C) 2001-2004 Peter Miller;
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
// MANIFEST: functions to manipulate synthesizes
//

#include <change.h>
#include <change/branch.h>
#include <change/file.h>
#include <change/verbose.h>
#include <change_set.h>
#include <commit.h>
#include <cstate.h>
#include <error.h>
#include <fstate.h>
#include <lock.h>
#include <project.h>
#include <project/history.h>
#include <project/file.h>
#include <sub.h>
#include <synthesize.h>
#include <trace.h>
#include <user.h>


static string_ty *
first_line(string_ty *s)
{
    string_ty	    *s1;
    string_ty	    *s2;

    trace(("s = \"%s\"\n", s->str_text));
    s1 = str_field(s, '\n', 0);
    trace(("s1 = \"%s\"\n", s1->str_text));
    s2 = str_trim(s1);
    trace(("s2 = \"%s\"\n", s2->str_text));
    str_free(s1);
    return s2;
}


static int
extract_year(time_t t)
{
    struct tm *the_time = localtime(&t);
    return the_time->tm_year + 1900;
}


static cstate_history_ty *
change_history_fake(change_ty *cp, string_ty *who, time_t when)
{
    cstate_ty	    *cstate_data;
    cstate_history_ty *history_data;
    cstate_history_ty **history_data_p;
    type_ty	    *type_p;

    trace(("change_history_fale(cp = %08lX)\n{\n", (long)cp));
    assert(cp->reference_count>=1);
    cstate_data = change_cstate_get(cp);
    assert(cstate_data->history);
    history_data_p =
	(cstate_history_ty **)
	cstate_history_list_type.list_parse(cstate_data->history, &type_p);
    assert(type_p==&cstate_history_type);
    history_data = (cstate_history_ty *)cstate_history_type.alloc();
    *history_data_p = history_data;
    history_data->when = when;
    history_data->who = str_copy(who);
    trace(("return %8.8lX;\n", (long)history_data));
    trace(("}\n"));
    return history_data;
}


void
synthesize(string_ty *project_name, change_set_ty *csp)
{
    project_ty	    *pp;
    long	    change_number;
    change_ty	    *cp;
    cstate_ty	    *cstate_data;
    user_ty	    *up;
    fstate_src_ty   *c_src_data;
    fstate_src_ty   *p_src_data;
    size_t	    j;
    cstate_history_ty *history_data;

    //
    // Take some locks.
    //
    trace(("synthesize()\n{\n"));
    pp = project_alloc(project_name);
    project_bind_existing(pp);
    project_pstate_lock_prepare(pp);
    project_history_lock_prepare(pp);
    lock_take();

    //
    // Now create a change so we can pretend we created the
    // change set just like any other change.
    //
    change_number = project_next_change_number(pp, 1);
    cp = change_alloc(pp, change_number);
    change_bind_new(cp);
    cstate_data = change_cstate_get(cp);

    //
    // Set change attributes.
    //
    cstate_data->description = str_copy(csp->description);
    cstate_data->brief_description = first_line(csp->description);
    cstate_data->cause = change_cause_internal_enhancement;
    cstate_data->test_exempt = true;
    cstate_data->test_baseline_exempt = true;
    cstate_data->regression_test_exempt = true;
    change_copyright_year_append(cp, extract_year(csp->when));

    //
    // add to history for change creation
    //
    up = project_user(pp);
    cstate_data->state = cstate_state_awaiting_development;
    history_data = change_history_fake(cp, csp->who, csp->when - 5);
    history_data->what = cstate_history_what_new_change;

    //
    // Add the change to the list of existing changes.
    //
    project_change_append(pp, change_number, 0);

    //
    // add to history for develop begin
    //
    cstate_data->state = cstate_state_being_developed;
    history_data = change_history_fake(cp, csp->who, csp->when - 4);
    history_data->what = cstate_history_what_develop_begin;

    //
    // Add the files to the change.
    //
    for (j = 0; j < csp->file.length; ++j)
    {
	change_set_file_ty *csfp;

	csfp = csp->file.item + j;
	trace(("%s\n", csfp->filename->str_text));
	c_src_data = change_file_new(cp, csfp->filename);

	c_src_data->action = file_action_modify;
	switch (csfp->action)
	{
	case change_set_file_action_create:
	    c_src_data->action = file_action_create;
	    break;

	case change_set_file_action_modify:
	    break;

	case change_set_file_action_remove:
	    c_src_data->action = file_action_remove;
	    break;
	}

	c_src_data->usage = file_usage_source;
	c_src_data->edit = (history_version_ty *)history_version_type.alloc();
	c_src_data->edit->revision = str_copy(csfp->edit);
	p_src_data = project_file_find(pp, csfp->filename, view_path_extreme);
	if (!p_src_data)
	    p_src_data = project_file_new(pp, csfp->filename);
	p_src_data->action = file_action_create;
	switch (csfp->action)
	{
	case change_set_file_action_create:
	case change_set_file_action_modify:
	    break;

	case change_set_file_action_remove:
	    p_src_data->action = file_action_remove;
	    p_src_data->deleted_by = change_number;
	    break;
	}
	p_src_data->usage = c_src_data->usage;
	if (p_src_data->edit)
	    history_version_type.free(p_src_data->edit);
	else
	    p_src_data->edit_origin = history_version_copy(c_src_data->edit);
	p_src_data->edit = history_version_copy(c_src_data->edit);
    }

    //
    // add to history for develop end
    //
    cstate_data->state = cstate_state_being_reviewed;
    history_data = change_history_fake(cp, csp->who, csp->when - 3);
    history_data->what = cstate_history_what_develop_end;

    //
    // add to history for review pass
    //
    cstate_data->state = cstate_state_awaiting_integration;
    history_data = change_history_fake(cp, csp->who, csp->when - 2);
    history_data->what = cstate_history_what_review_pass;

    //
    // add to history for integrate begin
    //
    cstate_data->state = cstate_state_being_integrated;
    history_data = change_history_fake(cp, csp->who, csp->when - 1);
    history_data->what = cstate_history_what_integrate_begin;

    cstate_data->delta_number = project_next_delta_number(pp);

    //
    // add to history for integrate pass
    //
    cstate_data->state = cstate_state_completed;
    history_data = change_history_fake(cp, csp->who, csp->when);
    history_data->what = cstate_history_what_integrate_pass;

    //
    // add to project history
    //
    project_history_new(pp, cstate_data->delta_number, change_number);
    for (j = 0; j < csp->tag.nstrings; ++j)
    {
	project_history_delta_name_add
	(
	    pp,
	    cstate_data->delta_number,
	    csp->tag.string[j]
	);
    }

    //
    // merge copyright years
    //
    project_copyright_years_merge(pp, cp);

    //
    // Write stuff back out.
    //
    change_cstate_write(cp);
    project_pstate_write(pp);
    commit();
    lock_release();

    //
    // Release resources.
    //
    change_verbose_new_change_complete(cp);
    change_free(cp);
    project_free(pp);
    trace(("}\n"));
}
