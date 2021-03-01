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
// MANIFEST: functions to manipulate config_files
//

#include <change.h>
#include <change/branch.h>
#include <change/file.h>
#include <change/verbose.h>
#include <commit.h>
#include <config_file.h>
#include <cstate.h>
#include <error.h>
#include <format.h>
#include <io.h>
#include <lock.h>
#include <os.h>
#include <pconf.h>
#include <project.h>
#include <project/history.h>
#include <project/file.h>
#include <sub.h>
#include <trace.h>
#include <user.h>
#include <uuidentifier.h>


void
config_file(string_ty *project_name, format_ty *format, time_t when,
    string_ty *the_config_file)
{
    project_ty      *pp;
    long	    change_number;
    fstate_src_ty   *c_src_data;
    fstate_src_ty   *p_src_data;
    change_ty	    *cp;
    pconf_ty	    *pconf_data;
    string_ty	    *bl;
    string_ty	    *pconf_file_name;
    cstate_history_ty *history_data;
    user_ty	    *up;
    cstate_ty	    *cstate_data;
    cstate_ty	    *p_cstate_data;
    string_ty	    *path;
    string_ty	    *path_d;
    string_ty	    *original;

    //
    // Take some locks.
    //
    trace(("config_file()\n{\n"));
    pp = project_alloc(project_name);
    project_bind_existing(pp);
    project_pstate_lock_prepare(pp);
    project_baseline_write_lock_prepare(pp);
    project_history_lock_prepare(pp);
    lock_take();

    //
    // Create the file contents.
    //
    pconf_data = (pconf_ty *)pconf_type.alloc();
    pconf_data->development_directory_style =
	(work_area_style_ty *)work_area_style_type.alloc();
    pconf_data->development_directory_style->source_file_link = true;
    pconf_data->development_directory_style->source_file_symlink = true;
    pconf_data->development_directory_style->source_file_copy = true;
    pconf_data->build_command = str_from_c("exit 0");
    pconf_data->history_put_command = format_history_put(format);
    pconf_data->history_create_command =
	str_copy(pconf_data->history_put_command);
    pconf_data->history_get_command = format_history_get(format);
    pconf_data->history_query_command = format_history_query(format);
    pconf_data->diff_command = format_diff(format);
    pconf_data->merge_command = format_merge(format);
    pconf_data->history_put_trashes_file = pconf_history_put_trashes_file_warn;

    //
    // Write the file directly into the baseline.
    //
    bl = project_baseline_path_get(pp, 0);
    pconf_file_name = os_path_join(bl, the_config_file);
    io_comment_append(0, i18n("configuration file hint"));
    project_become(pp);
    pconf_write_file(pconf_file_name, pconf_data, 0);
    project_become_undo();

    //
    // Now create a change so we can pretend we created the config
    // file the same as any other file in any other change.
    //
    change_number = project_next_change_number(pp, 1);
    cp = change_alloc(pp, change_number);
    change_bind_new(cp);
    cstate_data = change_cstate_get(cp);

    //
    // Set change attributes.
    //
    cstate_data->description =
	str_format("Initial project `%s' file.", the_config_file->str_text);
    cstate_data->brief_description =
	str_format("%s file", the_config_file->str_text);
    cstate_data->cause = change_cause_internal_enhancement;
    cstate_data->test_exempt = true;
    cstate_data->test_baseline_exempt = true;
    cstate_data->regression_test_exempt = true;
    change_copyright_years_now(cp);

    //
    // add to history for change creation
    //
    up = project_user(pp);
    cstate_data->state = cstate_state_awaiting_development;
    history_data = change_history_new(cp, up);
    history_data->what = cstate_history_what_new_change;
    history_data->when = when;

    //
    // Add the change to the list of existing changes.
    //
    project_change_append(pp, change_number, 0);

    //
    // add to history for develop begin
    //
    cstate_data->state = cstate_state_being_developed;
    history_data = change_history_new(cp, up);
    history_data->what = cstate_history_what_develop_begin;
    history_data->when = when + 1;

    //
    // Add the file to the change.
    //
    c_src_data = change_file_new(cp, the_config_file);
    c_src_data->action = file_action_create;
    c_src_data->usage = file_usage_config;

    //
    // add to history for develop end
    //
    cstate_data->state = cstate_state_being_reviewed;
    history_data = change_history_new(cp, up);
    history_data->what = cstate_history_what_develop_end;
    history_data->when = when + 2;

    //
    // create the project file
    //
    p_src_data = project_file_new(pp, the_config_file);
    p_src_data->usage = c_src_data->usage;

    //
    // add to history for review pass
    //
    cstate_data->state = cstate_state_awaiting_integration;
    history_data = change_history_new(cp, up);
    history_data->what = cstate_history_what_review_pass;
    history_data->when = when + 3;

    //
    // add to history for integrate begin
    //
    cstate_data->state = cstate_state_being_integrated;
    history_data = change_history_new(cp, up);
    history_data->what = cstate_history_what_integrate_begin;
    history_data->when = when + 4;

    cstate_data->delta_number = project_next_delta_number(pp);
    cstate_data->delta_uuid = universal_unique_identifier();
    change_integration_directory_set(cp, bl);

    //
    // Difference the file.
    //
    path = change_file_path(cp, the_config_file);
    assert(path);
    trace_string(path->str_text);
    path_d = str_format("%s,D", path->str_text);
    trace_string(path_d->str_text);
    original = str_from_c("/dev/null");
    change_run_diff_command(cp, up, original, path, path_d);
    str_free(original);

    //
    // Fingerprint the difference file.
    //
    p_src_data->diff_file_fp = (fingerprint_ty *)fingerprint_type.alloc();
    project_become(pp);
    change_fingerprint_same(p_src_data->diff_file_fp, path_d, 0);
    project_become_undo();

    //
    // Check the config file into the history.
    //
    change_run_history_create_command(cp, c_src_data);

    //
    // Fingerprint the file.
    //
    p_src_data->file_fp = (fingerprint_ty *)fingerprint_type.alloc();
    project_become(pp);
    change_fingerprint_same(p_src_data->file_fp, path, 0);
    project_become_undo();

    //
    // Update the head revision number.
    //
    assert(c_src_data->edit);
    assert(c_src_data->edit->revision);
    p_src_data->edit = history_version_copy(c_src_data->edit);
    p_src_data->edit_origin = history_version_copy(c_src_data->edit);

    //
    // add to history for integrate pass
    //
    cstate_data->state = cstate_state_completed;
    cstate_data->uuid = universal_unique_identifier();
    history_data = change_history_new(cp, up);
    history_data->what = cstate_history_what_integrate_pass;
    history_data->when = when + 5;

    change_integration_directory_clear(cp);

    //
    // add to project history
    //
    project_history_new(pp, cstate_data->delta_number, change_number);

    //
    // Set build times.
    //
    // A more thorough job will be done later, when the head revisions
    // of the files are checkout into the baseline, once all the
    // change sets are in place.
    //
    p_cstate_data = change_cstate_get(project_change_get(pp));
    p_cstate_data->build_time = history_data->when;

    //
    // add the copyright year to the project
    //
    change_copyright_years_now(project_change_get(pp));

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
