/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995 Peter Miller;
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
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: definitions of command line tokens
 */

#ifndef ARGLEX2_H
#define ARGLEX2_H

#include <arglex.h>

enum
{
	arglex_token_anticipate,
	arglex_token_automatic,
	arglex_token_baseline,
	arglex_token_build,
	arglex_token_change,
	arglex_token_change_attributes,
	arglex_token_change_directory,
	arglex_token_change_owner,
	arglex_token_copy_file,
	arglex_token_copy_file_undo,
	arglex_token_defaults,
	arglex_token_delta,
	arglex_token_develop_begin,
	arglex_token_develop_begin_undo,
	arglex_token_develop_end,
	arglex_token_develop_end_undo,
	arglex_token_development_directory,
	arglex_token_difference,
	arglex_token_directory,
	arglex_token_edit,
	arglex_token_file,
	arglex_token_force,
	arglex_token_independent,
	arglex_token_integrate_begin,
	arglex_token_integrate_begin_undo,
	arglex_token_integrate_fail,
	arglex_token_integrate_pass,
	arglex_token_interactive,
	arglex_token_keep,
	arglex_token_library,
	arglex_token_list,
	arglex_token_log,
	arglex_token_major,
	arglex_token_manual,
	arglex_token_merge_automatic,
	arglex_token_merge_not,
	arglex_token_merge_only,
	arglex_token_minimum,
	arglex_token_minor,
	arglex_token_move_file,
	arglex_token_new_administrator,
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
	arglex_token_no_keep,
	arglex_token_nolog,
	arglex_token_output,
	arglex_token_overwriting,
	arglex_token_page_length,
	arglex_token_page_width,
	arglex_token_project,
	arglex_token_project_attributes,
	arglex_token_regression,
	arglex_token_remove_administrator,
	arglex_token_remove_developer,
	arglex_token_remove_file,
	arglex_token_remove_file_undo,
	arglex_token_remove_integrator,
	arglex_token_remove_project,
	arglex_token_remove_reviewer,
	arglex_token_report,
	arglex_token_review_fail,
	arglex_token_review_pass,
	arglex_token_review_pass_undo,
	arglex_token_tab_width,
	arglex_token_terse,
	arglex_token_test,
	arglex_token_unchanged,
	arglex_token_unformatted,
	arglex_token_user,
	arglex_token_verbose
};

#endif /* ARGLEX2_H */
