//
//	aegis - project change supervisor
//	Copyright (C) 1991-2006, 2008 Peter Miller
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

#ifndef ARGLEX2_H
#define ARGLEX2_H

#include <common/arglex.h>

enum
{
	arglex_token_anticipate,
	arglex_token_automatic,
	arglex_token_baseline,
	arglex_token_base_relative,
	arglex_token_branch,
	arglex_token_build,
	arglex_token_change,
	arglex_token_change_attributes,
	arglex_token_change_directory,
	arglex_token_change_owner,
	arglex_token_clean,
	arglex_token_clone,
	arglex_token_compatibility,
	arglex_token_compress,
	arglex_token_compress_not,
	arglex_token_compression_algorithm,
	arglex_token_configured,
	arglex_token_configured_not,
	arglex_token_copy_file,
	arglex_token_copy_file_undo,
	arglex_token_current_relative,
	arglex_token_defaults,
	arglex_token_delta,
	arglex_token_delta_date,
	arglex_token_delta_from_change,
	arglex_token_delta_name,
	arglex_token_description_only,
	arglex_token_develop_begin,
	arglex_token_develop_begin_undo,
	arglex_token_develop_end,
	arglex_token_develop_end_undo,
	arglex_token_development_directory,
	arglex_token_difference,
	arglex_token_directory,
	arglex_token_edit,
	arglex_token_edit_bg,
	arglex_token_file,
	arglex_token_file_attributes,
	arglex_token_fix_architecture,
	arglex_token_force,
	arglex_token_grandparent,
	arglex_token_independent,
	arglex_token_integrate_begin,
	arglex_token_integrate_begin_undo,
	arglex_token_integrate_fail,
	arglex_token_integrate_pass,
	arglex_token_interactive,
	arglex_token_keep,
	arglex_token_keep_not,
	arglex_token_library,
	arglex_token_list,
	arglex_token_log,
	arglex_token_major,
	arglex_token_make_transparent,
	arglex_token_make_transparent_undo,
	arglex_token_manual,
	arglex_token_maximum,
	arglex_token_merge_automatic,
	arglex_token_merge_not,
	arglex_token_merge_only,
	arglex_token_mime_header,
	arglex_token_mime_header_not,
	arglex_token_minimum,
	arglex_token_minor,
	arglex_token_move_file,
	arglex_token_move_file_undo,
	arglex_token_new_administrator,
	arglex_token_new_branch,
	arglex_token_new_branch_undo,
	arglex_token_new_change,
	arglex_token_new_change_undo,
	arglex_token_new_developer,
	arglex_token_new_file,
	arglex_token_new_file_undo,
	arglex_token_new_integrator,
	arglex_token_new_project,
	arglex_token_new_release,
	arglex_token_new_reviewer,
	arglex_token_new_test,
	arglex_token_new_test_undo,
	arglex_token_nolog,
	arglex_token_output,
	arglex_token_overwriting,
	arglex_token_pager,
	arglex_token_pager_not,
	arglex_token_page_headers,
	arglex_token_page_headers_not,
	arglex_token_persevere,
	arglex_token_persevere_not,
	arglex_token_progress,
	arglex_token_progress_not,
	arglex_token_project,
	arglex_token_project_attributes,
	arglex_token_project_alias_create,
	arglex_token_project_alias_remove,
        arglex_token_project_recursive,
	arglex_token_regression,
	arglex_token_read_only,
	arglex_token_reason,
	arglex_token_remove_administrator,
	arglex_token_remove_developer,
	arglex_token_remove_file,
	arglex_token_remove_file_undo,
	arglex_token_remove_integrator,
	arglex_token_remove_project,
	arglex_token_remove_reviewer,
	arglex_token_report,
	arglex_token_rescind,
	arglex_token_review_begin,
	arglex_token_review_begin_undo,
	arglex_token_review_fail,
	arglex_token_review_pass,
	arglex_token_review_pass_undo,
	arglex_token_signed_off_by,
	arglex_token_signed_off_by_not,
	arglex_token_suggest,
	arglex_token_suggest_limit,
	arglex_token_suggest_noise,
	arglex_token_symbolic_links,
	arglex_token_symbolic_links_not,
	arglex_token_tab_width,
	arglex_token_template,
	arglex_token_template_not,
	arglex_token_terse,
	arglex_token_test,
	arglex_token_touch,
	arglex_token_touch_not,
	arglex_token_trunk,
	arglex_token_unchanged,
	arglex_token_unformatted,
	arglex_token_user,
        arglex_token_uuid,
	arglex_token_verbose,
	arglex_token_wait,
	arglex_token_wait_not,
	arglex_token_whiteout,
	arglex_token_whiteout_not,
	ARGLEX2_MAX
};

void arglex2_init(int, char **);
void arglex2_init3(int, char **, arglex_table_ty *);

void arglex2_retable(arglex_table_ty *);

#endif // ARGLEX2_H
