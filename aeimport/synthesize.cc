//
//	aegis - project change supervisor
//	Copyright (C) 2001-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/error.h>
#include <common/trace.h>
#include <common/uuidentifier.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/change.h>
#include <libaegis/change/verbose.h>
#include <libaegis/commit.h>
#include <libaegis/cstate.h>
#include <libaegis/fstate.h>
#include <libaegis/lock.h>
#include <libaegis/project/file.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>

#include <aeimport/change_set.h>
#include <aeimport/synthesize.h>


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
change_history_fake(change::pointer cp, string_ty *who, time_t when)
{
    cstate_ty	    *cstate_data;
    cstate_history_ty *history_data;
    cstate_history_ty **history_data_p;
    meta_type *type_p = 0;

    trace(("change_history_fale(cp = %08lX)\n{\n", (long)cp));
    assert(cp->reference_count>=1);
    cstate_data = cp->cstate_get();
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
    change::pointer cp;
    cstate_ty	    *cstate_data;
    fstate_src_ty   *c_src_data;
    fstate_src_ty   *p_src_data;
    size_t	    j;
    cstate_history_ty *history_data;

    //
    // Take some locks.
    //
    trace(("synthesize()\n{\n"));
    pp = project_alloc(project_name);
    pp->bind_existing();
    pp->pstate_lock_prepare();
    project_history_lock_prepare(pp);
    lock_take();

    //
    // Now create a change so we can pretend we created the
    // change set just like any other change.
    //
    change_number = project_next_change_number(pp, 1);
    cp = change_alloc(pp, change_number);
    change_bind_new(cp);
    cstate_data = cp->cstate_get();

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
    for (j = 0; j < csp->file.size(); ++j)
    {
	change_set_file_ty *csfp = csp->file[j];
	trace(("%s\n", csfp->filename->str_text));
	c_src_data = cp->file_new(csfp->filename);

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
	    p_src_data = pp->file_new(csfp->filename);
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
    cstate_data->delta_uuid = universal_unique_identifier();

    //
    // add to history for integrate pass
    //
    cstate_data->state = cstate_state_completed;
    cstate_data->uuid = universal_unique_identifier();
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
    pp->pstate_write();
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
