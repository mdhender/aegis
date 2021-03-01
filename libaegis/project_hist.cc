//
//      aegis - project change supervisor
//      Copyright (C) 1995-1998, 2001-2008, 2011, 2012 Peter Miller
//      Copyright (C) 2008, 2009 Walter Franzini
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

#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/change/branch.h>
#include <libaegis/project/history.h>
#include <libaegis/zero.h>


bool
project_history_delta_validate(project *pp, long delta_number)
{
    change::pointer cp;

    cp = pp->change_get();
    return change_history_delta_validate(cp, delta_number);
}


time_t
project_history_delta_to_timestamp(project *pp, long delnum)
{
    return change_history_delta_to_timestamp(pp, delnum);
}


long
project_history_timestamp_to_delta(project *pp, time_t when)
{
    return change_history_timestamp_to_delta(pp, when);
}


long
project_history_delta_latest(project *pp)
{
    change::pointer cp;

    cp = pp->change_get();
    return change_history_delta_latest(cp);
}


long
project_history_delta_by_name(project *pp, string_ty *delta_name, int errok)
{
    change::pointer cp;

    cp = pp->change_get();
    return change_history_delta_by_name(cp, delta_name, errok);
}


long
project_history_change_by_name(project *pp, string_ty *delta_name, int errok)
{
    change::pointer cp;

    cp = pp->change_get();
    return change_history_change_by_name(cp, delta_name, errok);
}


long
project_history_change_by_timestamp(project *pp, time_t when)
{
    return change_history_change_by_timestamp(pp, when);
}


long
project_history_change_by_delta(project *pp, long delta_number)
{
    change::pointer cp;

    cp = pp->change_get();
    return change_history_change_by_delta(cp, delta_number);
}


void
project_history_delta_name_delete(project *pp, string_ty *delta_name)
{
    change::pointer cp;

    cp = pp->change_get();
    change_history_delta_name_delete(cp, delta_name);
}


void
project_history_delta_name_add(project *pp, long delta_number,
    string_ty *delta_name)
{
    change::pointer cp;

    cp = pp->change_get();
    change_history_delta_name_add(cp, delta_number, delta_name);
}


long
project_current_integration_get(project *pp)
{
    change::pointer cp;
    long            result;

    trace(("project_current_integration_get(pp = %p)\n{\n", pp));
    cp = pp->change_get();
    result = change_current_integration_get(cp);
    trace(("return %ld;\n", result));
    trace(("}\n"));
    return result;
}


void
project_current_integration_set(project *pp, long change_number)
{
    change::pointer cp;

    trace(("project_current_integration_set(pp = %p, n = %ld)\n{\n",
        pp, change_number));
    assert(change_number >= 0 || change_number == MAGIC_ZERO);
    cp = pp->change_get();
    change_current_integration_set(cp, change_number);
    trace(("}\n"));
}


long
project_next_delta_number(project *pp)
{
    change::pointer cp;

    cp = pp->change_get();
    return change_branch_next_delta_number(cp);
}


void
project_copyright_years_merge(project *pp, change::pointer cp2)
{
    change::pointer cp1;

    cp1 = pp->change_get();
    change_copyright_years_merge(cp1, cp2);
}


void
project_copyright_years_get(project *pp, int *ary, int ary_len_max,
    int *ary_len)
{
    change::pointer cp;

    //
    // when we have the real thing, will need to get the years
    // from parent branches back to the trunk
    //
    cp = pp->change_get();
    change_copyright_years_get(cp, ary, ary_len_max, ary_len);
}


void
project_copyright_year_append(project *pp, int yyyy)
{
    change::pointer cp;

    cp = pp->change_get();
    change_copyright_year_append(cp, yyyy);
}


void
project_history_new(project *pp, long delta_number, long change_number,
    string_ty *uuid, time_t when, bool is_a_branch)
{
    change::pointer cp;

    trace(("project_history_new(pp = %p, delta_number = %ld, "
        "change_number = %ld)\n{\n", pp, delta_number, change_number));
    cp = pp->change_get();
    change_branch_history_new
    (
        cp,
        delta_number,
        change_number,
        uuid,
        when,
        is_a_branch
    );
    trace(("}\n"));
}


int
project_history_nth(project *pp, long n, long *cnp, long *dnp,
    string_list_ty *name)
{
    change::pointer cp;
    int             result;

    trace(("project_history_nth(pp = %p, n = %ld)\n{\n", pp, n));
    cp = pp->change_get();
    result = change_branch_history_nth(cp, n, cnp, dnp, name);
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}


bool
project_administrator_query(project *pp, const nstring &usrnam)
{
    trace(("project_administrator_query(pp = %p, usrnam = %s)\n{\n",
        pp, usrnam.quote_c().c_str()));
    change::pointer cp = pp->change_get();
    bool result = change_branch_administrator_query(cp, usrnam.get_ref());
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}


bool
project_administrator_query(project *pp, string_ty *usrnam)
{
    trace(("project_administrator_query(pp = %p, usrnam = \"%s\")\n{\n",
        pp, usrnam->str_text));
    bool result = project_administrator_query(pp, nstring(usrnam));
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}


void
project_administrator_add(project *pp, const nstring &usrnam)
{
    trace(("project_administrator_add(pp = %p, usrnam = %s)\n{\n",
        pp, usrnam.quote_c().c_str()));
    change::pointer cp = pp->change_get();
    change_branch_administrator_add(cp, usrnam.get_ref());
    trace(("}\n"));
}


void
project_administrator_add(project *pp, string_ty *usrnam)
{
    trace(("project_administrator_add(pp = %p, usrnam = \"%s\")\n{\n",
        pp, usrnam->str_text));
    project_administrator_add(pp, nstring(usrnam));
    trace(("}\n"));
}


void
project_administrator_remove(project *pp, const nstring &usrnam)
{
    trace(("project_administrator_remove(pp = %p, usrnam = %s)\n{\n",
        pp, usrnam.quote_c().c_str()));
    change::pointer cp = pp->change_get();
    change_branch_administrator_remove(cp, usrnam.get_ref());
    trace(("}\n"));
}


void
project_administrator_remove(project *pp, string_ty *usrnam)
{
    trace(("project_administrator_remove(pp = %p, usrnam = \"%s\")\n{\n",
        pp, usrnam->str_text));
    project_administrator_remove(pp, nstring(usrnam));
    trace(("}\n"));
}


string_ty *
project_administrator_nth(project *pp, long n)
{
    change::pointer cp;
    string_ty       *result;

    trace(("project_administrator_nth(pp = %p, n = %ld)\n{\n",
        pp, n));
    cp = pp->change_get();
    result = change_branch_administrator_nth(cp, n);
    trace(("return %p;\n", result));
    trace(("}\n"));
    return result;
}


bool
project_developer_query(project *pp, const nstring &usrnam)
{
    trace(("project_developer_query(pp = %p, usrnam = %s)\n{\n",
        pp, usrnam.quote_c().c_str()));
    change::pointer cp = pp->change_get();
    bool result = change_branch_developer_query(cp, usrnam.get_ref());
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}


bool
project_developer_query(project *pp, string_ty *usrnam)
{
    trace(("project_developer_query(pp = %p, usrnam = \"%s\")\n{\n",
        pp, usrnam->str_text));
    bool result = project_developer_query(pp, nstring(usrnam));
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}


void
project_developer_add(project *pp, const nstring &usrnam)
{
    trace(("project_developer_add(pp = %p, usrnam = %s)\n{\n",
        pp, usrnam.quote_c().c_str()));
    change::pointer cp = pp->change_get();
    change_branch_developer_add(cp, usrnam.get_ref());
    trace(("}\n"));
}


void
project_developer_add(project *pp, string_ty *usrnam)
{
    trace(("project_developer_add(pp = %p, usrnam = \"%s\")\n{\n",
        pp, usrnam->str_text));
    project_developer_add(pp, nstring(usrnam));
    trace(("}\n"));
}


void
project_developer_remove(project *pp, const nstring &usrnam)
{
    trace(("project_developer_remove(pp = %p, usrnam = \"%s\")\n{\n",
        pp, usrnam.quote_c().c_str()));
    change::pointer cp = pp->change_get();
    change_branch_developer_remove(cp, usrnam.get_ref());
    trace(("}\n"));
}


void
project_developer_remove(project *pp, string_ty *usrnam)
{
    trace(("project_developer_remove(pp = %p, usrnam = \"%s\")\n{\n",
        pp, usrnam->str_text));
    project_developer_remove(pp, nstring(usrnam));
    trace(("}\n"));
}


string_ty *
project_developer_nth(project *pp, long n)
{
    change::pointer cp;
    string_ty       *result;

    trace(("project_developer_nth(pp = %p, n = %ld)\n{\n",
        pp, n));
    cp = pp->change_get();
    result = change_branch_developer_nth(cp, n);
    trace(("return %p;\n", result));
    trace(("}\n"));
    return result;
}


bool
project_reviewer_query(project *pp, const nstring &usrnam)
{
    trace(("project_reviewer_query(pp = %p, usrnam = %s)\n{\n",
        pp, usrnam.quote_c().c_str()));
    change::pointer cp = pp->change_get();
    bool result = change_branch_reviewer_query(cp, usrnam.get_ref());
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}


bool
project_reviewer_query(project *pp, string_ty *usrnam)
{
    trace(("project_reviewer_query(pp = %p, usrnam = \"%s\")\n{\n",
        pp, usrnam->str_text));
    bool result = project_reviewer_query(pp, nstring(usrnam));
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}


void
project_reviewer_add(project *pp, const nstring &usrnam)
{
    trace(("project_reviewer_add(pp = %p, usrnam = %s)\n{\n",
        pp, usrnam.quote_c().c_str()));
    change::pointer cp = pp->change_get();
    change_branch_reviewer_add(cp, usrnam.get_ref());
    trace(("}\n"));
}


void
project_reviewer_add(project *pp, string_ty *usrnam)
{
    trace(("project_reviewer_add(pp = %p, usrnam = \"%s\")\n{\n",
        pp, usrnam->str_text));
    project_reviewer_add(pp, nstring(usrnam));
    trace(("}\n"));
}


void
project_reviewer_remove(project *pp, const nstring &usrnam)
{
    trace(("project_reviewer_remove(pp = %p, usrnam = %s)\n{\n",
        pp, usrnam.quote_c().c_str()));
    change::pointer cp = pp->change_get();
    change_branch_reviewer_remove(cp, usrnam.get_ref());
    trace(("}\n"));
}


void
project_reviewer_remove(project *pp, string_ty *usrnam)
{
    trace(("project_reviewer_remove(pp = %p, usrnam = \"%s\")\n{\n",
        pp, usrnam->str_text));
    project_reviewer_remove(pp, nstring(usrnam));
    trace(("}\n"));
}


string_ty *
project_reviewer_nth(project *pp, long n)
{
    change::pointer cp;
    string_ty       *result;

    trace(("project_reviewer_nth(pp = %p, n = %ld)\n{\n",
        pp, n));
    cp = pp->change_get();
    result = change_branch_reviewer_nth(cp, n);
    trace(("return %p;\n", result));
    trace(("}\n"));
    return result;
}


bool
project_integrator_query(project *pp, const nstring &usrnam)
{
    trace(("project_integrator_query(pp = %p, usrnam = %s)\n{\n",
        pp, usrnam.quote_c().c_str()));
    change::pointer cp = pp->change_get();
    bool result = change_branch_integrator_query(cp, usrnam.get_ref());
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}


bool
project_integrator_query(project *pp, string_ty *usrnam)
{
    trace(("project_integrator_query(pp = %p, usrnam = \"%s\")\n{\n",
        pp, usrnam->str_text));
    bool result = project_integrator_query(pp, nstring(usrnam));
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}


void
project_integrator_add(project *pp, const nstring &usrnam)
{
    trace(("project_integrator_add(pp = %p, usrnam = %s)\n{\n",
        pp, usrnam.quote_c().c_str()));
    change::pointer cp = pp->change_get();
    change_branch_integrator_add(cp, usrnam.get_ref());
    trace(("}\n"));
}


void
project_integrator_add(project *pp, string_ty *usrnam)
{
    trace(("project_integrator_add(pp = %p, usrnam = \"%s\")\n{\n",
        pp, usrnam->str_text));
    project_integrator_add(pp, nstring(usrnam));
    trace(("}\n"));
}


void
project_integrator_remove(project *pp, const nstring &usrnam)
{
    trace(("project_integrator_remove(pp = %p, usrnam = %s)\n{\n",
        pp, usrnam.quote_c().c_str()));
    change::pointer cp = pp->change_get();
    change_branch_integrator_remove(cp, usrnam.get_ref());
    trace(("}\n"));
}


void
project_integrator_remove(project *pp, string_ty *usrnam)
{
    trace(("project_integrator_remove(pp = %p, usrnam = \"%s\")\n{\n",
        pp, usrnam->str_text));
    project_integrator_remove(pp, nstring(usrnam));
    trace(("}\n"));
}


string_ty *
project_integrator_nth(project *pp, long n)
{
    change::pointer cp;
    string_ty       *result;

    trace(("project_integrator_nth(pp = %p, n = %ld)\n{\n",
        pp, n));
    cp = pp->change_get();
    result = change_branch_integrator_nth(cp, n);
    trace(("return %p;\n", result));
    trace(("}\n"));
    return result;
}


void
project_change_add(project *pp, long change_number, int is_a_branch)
{
    change::pointer cp;

    trace(("project_change_add(pp = %p, change_number = %ld)\n{\n",
        pp, change_number));
    cp = pp->change_get();
    change_branch_change_add(cp, change_number, is_a_branch);
    trace(("}\n"));
}


void
project_change_remove(project *pp, long change_number)
{
    change::pointer cp;

    trace(("project_change_remove(pp = %p, change_number = %ld)\n{\n",
        pp, change_number));
    cp = pp->change_get();
    change_branch_change_remove(cp, change_number);
    trace(("}\n"));
}


int
project_change_nth(project *pp, long n, long *cnp)
{
    change::pointer cp;
    int             result;

    trace(("project_change_nth(pp = %p, n = %ld)\n{\n", pp, n));
    cp = pp->change_get();
    result = change_branch_change_nth(cp, n, cnp);
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}


long
project_next_change_number(project *pp, int skip)
{
    change::pointer cp;
    long            change_number;

    trace(("project_next_change_number(pp = %p)\n{\n", pp));
    cp = pp->change_get();
    change_number = change_branch_next_change_number(cp, skip);
    trace(("return %ld;\n", change_number));
    trace(("}\n"));
    return change_number;
}


long
project_last_change_integrated(project *pp)
{
    change::pointer cp;
    long            n;

    //
    // This function is used to determine if the project file list
    // has (potentially) changed.  Hash all of the ancestors
    // together.
    //
    n = 0;
    while (pp)
    {
        cp = pp->change_get();
        n = n * 29 + change_history_last_change_integrated(cp);
        pp = (pp->is_a_trunk() ? 0 : pp->parent_get());
    }
    return n;
}


void
project_description_set(project *pp, string_ty *s)
{
    change::pointer cp;
    cstate_ty       *cstate_data;

    //
    // set the trunk change state file
    //
    trace(("project_description_set(pp = %p, s = \"%s\")\n{\n",
        pp, s->str_text));
    cp = pp->change_get();
    cstate_data = cp->cstate_get();
    if (cstate_data->brief_description)
        str_free(cstate_data->brief_description);
    cstate_data->brief_description = str_copy(s);
    trace(("}\n"));
}


nstring
project_description_get(project *pp)
{
    change::pointer cp;
    cstate_ty       *cstate_data;

    cp = pp->change_get();
    cstate_data = cp->cstate_get();
    return nstring(cstate_data->brief_description);
}


string_ty *
project_version_previous_get(project *pp)
{
    change::pointer cp;
    cstate_ty       *cstate_data;

    cp = pp->change_get();
    cstate_data = cp->cstate_get();
    return cstate_data->version_previous;
}


void
project_version_previous_set(project *pp, string_ty *s)
{
    change::pointer cp;
    cstate_ty       *cstate_data;

    cp = pp->change_get();
    cstate_data = cp->cstate_get();
    if (cstate_data->version_previous)
        str_free(cstate_data->version_previous);
    if (s)
        cstate_data->version_previous = str_copy(s);
    else
        cstate_data->version_previous = 0;
}


void
project_umask_set(project *pp, int n)
{
    change_branch_umask_set(pp->change_get(), n);
}


int
project_umask_get(project *pp)
{
    return pp->umask_get();
}


int
project::umask_get()
{
    if (off_limits)
        return 027;
    return change_branch_umask_get(change_get());
}


void
project_developer_may_review_set(project *pp, bool n)
{
    change_branch_developer_may_review_set(pp->change_get(), n);
}


bool
project_developer_may_review_get(project *pp)
{
    return change_branch_developer_may_review_get(pp->change_get());
}


void
project_developer_may_integrate_set(project *pp, bool n)
{
    change_branch_developer_may_integrate_set(pp->change_get(), n);
}


bool
project_developer_may_integrate_get(project *pp)
{
    return change_branch_developer_may_integrate_get(pp->change_get());
}


void
project_reviewer_may_integrate_set(project *pp, bool n)
{
    change_branch_reviewer_may_integrate_set(pp->change_get(), n);
}


bool
project_reviewer_may_integrate_get(project *pp)
{
    return change_branch_reviewer_may_integrate_get(pp->change_get());
}


void
project_developers_may_create_changes_set(project *pp, bool n)
{
    change_branch_developers_may_create_changes_set(pp->change_get(), n);
}


bool
project_developers_may_create_changes_get(project *pp)
{
    return
        change_branch_developers_may_create_changes_get(pp->change_get());
}


void
project_forced_develop_begin_notify_command_set(project *pp, string_ty *s)
{
    change_branch_forced_develop_begin_notify_command_set
    (
        pp->change_get(),
        s
    );
}


string_ty *
project_forced_develop_begin_notify_command_get(project *pp)
{
    return
        change_branch_forced_develop_begin_notify_command_get
        (
            pp->change_get()
        );
}


void
project_develop_end_notify_command_set(project *pp, string_ty *s)
{
    change_branch_develop_end_notify_command_set(pp->change_get(), s);
}


string_ty *
project_develop_end_notify_command_get(project *pp)
{
    return change_branch_develop_end_notify_command_get(pp->change_get());
}


void
project_develop_end_undo_notify_command_set(project *pp, string_ty *s)
{
    change_branch_develop_end_undo_notify_command_set
    (
        pp->change_get(),
        s
    );
}


string_ty *
project_develop_end_undo_notify_command_get(project *pp)
{
    return
        change_branch_develop_end_undo_notify_command_get
        (
            pp->change_get()
        );
}


void
project_review_pass_notify_command_set(project *pp, string_ty *s)
{
    change_branch_review_pass_notify_command_set(pp->change_get(), s);
}


string_ty *
project_review_pass_notify_command_get(project *pp)
{
    return change_branch_review_pass_notify_command_get(pp->change_get());
}


void
project_review_pass_undo_notify_command_set(project *pp, string_ty *s)
{
    change_branch_review_pass_undo_notify_command_set
    (
        pp->change_get(),
        s
    );
}


string_ty *
project_review_pass_undo_notify_command_get(project *pp)
{
    return
        change_branch_review_pass_undo_notify_command_get
        (
            pp->change_get()
        );
}


void
project_review_fail_notify_command_set(project *pp, string_ty *s)
{
    change_branch_review_fail_notify_command_set(pp->change_get(), s);
}


string_ty *
project_review_fail_notify_command_get(project *pp)
{
    return change_branch_review_fail_notify_command_get(pp->change_get());
}


void
project_integrate_pass_notify_command_set(project *pp, string_ty *s)
{
    change_branch_integrate_pass_notify_command_set(pp->change_get(), s);
}


string_ty *
project_integrate_pass_notify_command_get(project *pp)
{
    return
        change_branch_integrate_pass_notify_command_get(pp->change_get());
}


void
project_integrate_fail_notify_command_set(project *pp, string_ty *s)
{
    change_branch_integrate_fail_notify_command_set(pp->change_get(), s);
}


string_ty *
project_integrate_fail_notify_command_get(project *pp)
{
    return
        change_branch_integrate_fail_notify_command_get(pp->change_get());
}


void
project_default_development_directory_set(project *pp, string_ty *s)
{
    change_branch_default_development_directory_set(pp->change_get(), s);
}

string_ty *
project_default_development_directory_get(project *pp)
{
    return
        change_branch_default_development_directory_get(pp->change_get());
}


void
project_default_test_exemption_set(project *pp, bool n)
{
    change_branch_default_test_exemption_set(pp->change_get(), n);
}


bool
project_default_test_exemption_get(project *pp)
{
    return change_branch_default_test_exemption_get(pp->change_get());
}


// vim: set ts=8 sw=4 et :
