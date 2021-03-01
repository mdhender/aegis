/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001, 2002 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate config_files
 */

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
#include <pconf.h>
#include <project.h>
#include <project/history.h>
#include <project/file.h>
#include <sub.h>
#include <trace.h>
#include <user.h>


void
config_file(project_name, format)
	string_ty	*project_name;
	format_ty	*format;
{
	project_ty	*pp;
	long		change_number;
	fstate_src      c_src_data;
	fstate_src      p_src_data;
	change_ty	*cp;
	pconf		pconf_data;
	string_ty	*bl;
	string_ty	*pconf_file_name;
	cstate_history	history_data;
	user_ty		*up;
	cstate		cstate_data;
	cstate		p_cstate_data;
	string_ty	*the_config_file;
	string_ty	*path;
	string_ty	*path_d;
	string_ty	*original;

	/*
	 * Take some locks.
	 */
	trace(("config_file()\n{\n"));
	pp = project_alloc(project_name);
	project_bind_existing(pp);
	project_pstate_lock_prepare(pp);
	project_baseline_write_lock_prepare(pp);
	project_history_lock_prepare(pp);
	lock_take();

	/*
	 * Create the file contents.
	 */
	pconf_data = pconf_type.alloc();
	pconf_data->create_symlinks_before_build = 1;
	pconf_data->remove_symlinks_after_build = 0;
	pconf_data->create_symlinks_before_integration_build = 1;
	pconf_data->remove_symlinks_after_integration_build = 0;
	pconf_data->build_command = str_from_c("exit 0");
	pconf_data->history_put_command = format_history_put(format);
	pconf_data->history_create_command =
		str_copy(pconf_data->history_put_command);
	pconf_data->history_get_command = format_history_get(format);
	pconf_data->history_query_command = format_history_query(format);
	pconf_data->diff_command = format_diff(format);
	pconf_data->merge_command = format_merge(format);
	pconf_data->history_put_trashes_file =
		pconf_history_put_trashes_file_warn;

	/*
	 * Write the file directly into the baseline.
	 */
	bl = project_baseline_path_get(pp, 0);
	pconf_file_name = str_format("%S/%s", bl, THE_CONFIG_FILE);
	io_comment_append(0, i18n("config file hint"));
	project_become(pp);
	pconf_write_file(pconf_file_name, pconf_data, 0);
	project_become_undo();

	/*
	 * Now create a change so we can pretend we created the config
	 * file the same as any other file in any other change.
	 */
	change_number = project_next_change_number(pp, 1);
	cp = change_alloc(pp, change_number);
	change_bind_new(cp);
	cstate_data = change_cstate_get(cp);

	/*
	 * Set change attributes.
	 */
	cstate_data->description =
		str_format("Initial project `%s' file.", THE_CONFIG_FILE);
	cstate_data->brief_description =
		str_format("%s file", THE_CONFIG_FILE);
	cstate_data->cause = change_cause_internal_enhancement;
	cstate_data->test_exempt = 1;
	cstate_data->test_baseline_exempt = 1;
	cstate_data->regression_test_exempt = 1;
	change_copyright_years_now(cp);

	/*
	 * add to history for change creation
	 */
	up = project_user(pp);
	cstate_data->state = cstate_state_awaiting_development;
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_new_change;

	/*
	 * Add the change to the list of existing changes.
	 */
	project_change_append(pp, change_number, 0);

	/*
	 * add to history for develop begin
	 */
	cstate_data->state = cstate_state_being_developed;
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_develop_begin;

	/*
	 * Add the file to the change.
	 */
	the_config_file = str_from_c(THE_CONFIG_FILE);
	c_src_data = change_file_new(cp, the_config_file);
	c_src_data->action = file_action_create;
	c_src_data->usage = file_usage_source;

	/*
	 * add to history for develop end
	 */
	cstate_data->state = cstate_state_being_reviewed;
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_develop_end;

	/*
	 * create the project file
	 */
	p_src_data = project_file_new(pp, the_config_file);
	p_src_data->usage = c_src_data->usage;

	/*
	 * add to history for review pass
	 */
	cstate_data->state = cstate_state_awaiting_integration;
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_review_pass;

	/*
	 * add to history for integrate begin
	 */
	cstate_data->state = cstate_state_being_integrated;
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_integrate_begin;

	cstate_data->delta_number = project_next_delta_number(pp);
	change_integration_directory_set(cp, bl);

	/*
	 * Difference the file.
	 */
	path = change_file_path(cp, the_config_file);
	assert(path);
	trace_string(path->str_text);
	path_d = str_format("%S,D", path);
	trace_string(path_d->str_text);
	original = str_from_c("/dev/null");
	change_run_diff_command(cp, up, original, path, path_d);
	str_free(original);

	/*
	 * Fingerprint the difference file.
	 */
	p_src_data->diff_file_fp = fingerprint_type.alloc();
	project_become(pp);
	change_fingerprint_same(p_src_data->diff_file_fp, path_d, 0);
	project_become_undo();

	/*
	 * Check the config file into the history.
	 */
	change_run_history_create_command(cp, c_src_data);

	/*
	 * Fingerprint the file.
	 */
	p_src_data->file_fp = fingerprint_type.alloc();
	project_become(pp);
	change_fingerprint_same(p_src_data->file_fp, path, 0);
	project_become_undo();

	/*
	 * Update the head revision number.
	 */
	assert(c_src_data->edit);
	assert(c_src_data->edit->revision);
	p_src_data->edit = history_version_copy(c_src_data->edit);
	p_src_data->edit_origin = history_version_copy(c_src_data->edit);

	/*
	 * add to history for integrate pass
	 */
	cstate_data->state = cstate_state_completed;
	history_data = change_history_new(cp, up);
	history_data->what = cstate_history_what_integrate_pass;

	change_integration_directory_clear(cp);

	/*
	 * add to project history
	 */
	project_history_new(pp, cstate_data->delta_number, change_number);

	/*
	 * Set build times.
	 *
	 * A more thorough job will be done later, when the head revisions
	 * of the files are checkout into the baseline, once all the
	 * change sets are in place.
	 */
	p_cstate_data = change_cstate_get(project_change_get(pp));
	p_cstate_data->build_time = history_data->when;

	/*
	 * add the copyright year to the project
	 */
	change_copyright_years_now(project_change_get(pp));

	/*
	 * Write stuff back out.
	 */
	change_cstate_write(cp);
	project_pstate_write(pp);
	commit();
	lock_release();

	/*
	 * Release resources.
	 */
	change_verbose_new_change_complete(cp);
	change_free(cp);
	project_free(pp);
	trace(("}\n"));
}
