//
//	aegis - project change supervisor
//	Copyright (C) 1995-2006 Peter Miller;
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
// MANIFEST: interface definition for libaegis/change.c
//

#ifndef LIBAEGIS_CHANGE_H
#define LIBAEGIS_CHANGE_H

#include <common/main.h>
#include <libaegis/cstate.h>
#include <libaegis/fstate.h>
#include <libaegis/pconf.h>
#include <libaegis/view_path.h>
#include <libaegis/zero.h>

class nstring_list; // forward

//
// Name of the project configuration file,
// relative to the baseline/devdir root.
//
#define THE_CONFIG_FILE_OLD "config"
#define THE_CONFIG_FILE_NEW "aegis.conf"

//
// Define a magic number to represent the trunk transaction
// of the project.  It has an exceptional file name.
//
#define TRUNK_CHANGE_NUMBER ((long)((~(unsigned long)0) >> 1))

struct string_list_ty; // existence
struct user_ty; // existence
struct sub_context_ty; // existence
struct symtab_ty; // existence

struct change_ty
{
	long		reference_count;
	struct project_ty *pp;
	long		number;
	cstate_ty       *cstate_data;
	string_ty	*cstate_filename;
	int		cstate_is_a_new_file;
	fstate_ty	*fstate_data;
	symtab_ty       *fstate_stp;
	symtab_ty       *fstate_uuid_stp;
	string_ty	*fstate_filename;
	string_ty	*pfstate_filename;
	int		fstate_is_a_new_file;
	string_ty	*top_path_unresolved;
	string_ty	*top_path_resolved;
	string_ty	*development_directory_unresolved;
	string_ty	*development_directory_resolved;
	string_ty	*integration_directory_unresolved;
	string_ty	*integration_directory_resolved;
	string_ty	*logfile;
	pconf_ty	*pconf_data;
	long		lock_magic;
	int		bogus;
	string_ty	*architecture_name;
	struct string_list_ty *file_list[view_path_MAX];

	/**
          * The attributes_get method is used to obtain the value of an
          * attribute of this change set.
	  *
	  * @param name
	  *     The name of the attribute.
	  *     Attribute names are not case sensitive.
	  * @returns
          *     the string value of the attribute, or the empty string
          *     if not found.
	  */
	nstring attributes_get(const nstring &name);

	/**
          * The attributes_get_boolean method is used to obtain the
          * value of an attribute of this change set, as a true/false
          * attribute.
	  *
	  * @param name
	  *     The name of the attribute.
	  *     Attribute names are not case sensitive.
	  * @returns
          *     the boolean value of the attribute, or false if the
          *     attribute is not found or is not interpretable as a
          *     boolean.
	  */
	bool attributes_get_boolean(const nstring &name);

	/**
          * The pconf_attributes_get method is used to obtain the
          * value of an attribute of the project, from the project
          * configuration file.  It could be in this change, or it could
          * be in a baseline or an ancestor baseline.
	  *
	  * @param name
	  *     The name of the attribute.
	  *     Attribute names are not case sensitive.
	  * @returns
          *     the string value of the attribute, or the empty string
          *     if not found.
	  */
	nstring pconf_attributes_find(const nstring &name);

	/**
          * The pconf_copyright_owner_get method is used to obtain the
          * name of the copyright holder of a project.
	  */
	nstring pconf_copyright_owner_get();

	//
	// if you add to this structure, don't forget to update
	// change_alloc()     in libaegis/change/alloc.cc
	// change_free()      in libaegis/change/free.cc
	// change_lock_sync() in libaegis/change/lock_sync.cc
	//
};

change_ty *change_alloc(struct project_ty *, long);
void change_free(change_ty *);
change_ty *change_copy(change_ty *);
void change_bind_existing(change_ty *);
int change_bind_existing_errok(change_ty *);
void change_bind_new(change_ty *);
change_ty *change_bogus(struct project_ty *);
cstate_ty *change_cstate_get(change_ty *);
void change_cstate_write(change_ty *);
cstate_history_ty *change_history_new(change_ty *, struct user_ty *);
string_ty *change_creator_name(change_ty *);
string_ty *change_developer_name(change_ty *);
string_ty *change_reviewer_name(change_ty *);
string_ty *change_integrator_name(change_ty *);
void change_top_path_set(change_ty *, string_ty *);
void change_top_path_set(change_ty *, const nstring &);
void change_development_directory_set(change_ty *, string_ty *);
void change_integration_directory_set(change_ty *, string_ty *);
string_ty *change_top_path_get(change_ty *, int);
string_ty *change_development_directory_get(change_ty *, int);
string_ty *change_integration_directory_get(change_ty *, int);
string_ty *change_logfile_basename(void);
string_ty *change_logfile_get(change_ty *);
void change_cstate_lock_prepare(change_ty *);
void change_error(change_ty *, struct sub_context_ty *, const char *);
void change_fatal(change_ty *, struct sub_context_ty *, const char *) NORETURN;
void change_verbose(change_ty *, struct sub_context_ty *, const char *);

/**
  * The change_warning function is used to issue a warning message
  * specific to a change.  The message will be substituted accouring
  * to aesub(5) and any additional substitutions provided in the
  * substitution context.
  */
void change_warning(change_ty *, struct sub_context_ty *, const char *);

/**
  * The change_warning_obsolete_field function is used to warn about the
  * use of obsolete fields (in user suppied files, Aegis will take care
  * of quietly cleaning up the meta-data files all by itself).
  */
void change_warning_obsolete_field(change_ty *cp, string_ty *errpos,
    const char *old_field, const char *new_field);

pconf_ty *change_pconf_get(change_ty *, int);
void change_run_new_file_command(change_ty *, struct string_list_ty *,
	struct user_ty *);
void change_run_new_file_undo_command(change_ty *,
	struct string_list_ty *, struct user_ty *);
void change_run_new_test_command(change_ty *, struct string_list_ty *,
	struct user_ty *);
void change_run_new_test_undo_command(change_ty *,
	struct string_list_ty *, struct user_ty *);
void change_run_copy_file_command(change_ty *, struct string_list_ty *,
	struct user_ty *);
void change_run_copy_file_undo_command(change_ty *,
	struct string_list_ty *, struct user_ty *);
void change_run_remove_file_command(change_ty *, struct string_list_ty *,
	struct user_ty *);
void change_run_remove_file_undo_command(change_ty *,
	struct string_list_ty *, struct user_ty *);
void change_run_make_transparent_command(change_ty *, struct string_list_ty *,
	struct user_ty *);
void change_run_make_transparent_undo_command(change_ty *,
	struct string_list_ty *, struct user_ty *);
int change_run_project_file_command_needed(change_ty *);
void change_run_project_file_command(change_ty *, struct user_ty *);
void change_run_forced_develop_begin_notify_command(change_ty *,
	struct user_ty *);
void change_run_develop_end_notify_command(change_ty *);
void change_run_develop_end_undo_notify_command(change_ty *);
void change_run_review_begin_notify_command(change_ty *);
void change_run_review_begin_undo_notify_command(change_ty *);
void change_run_review_pass_notify_command(change_ty *);
void change_run_review_pass_undo_notify_command(change_ty *);
void change_run_review_fail_notify_command(change_ty *);
void change_run_integrate_pass_notify_command(change_ty *);
void change_run_integrate_fail_notify_command(change_ty *);
void change_run_history_get_command(change_ty *cp, fstate_src_ty *src,
	string_ty *output_file, struct user_ty *up);
void change_run_history_create_command(change_ty *cp, fstate_src_ty *);
void change_run_history_put_command(change_ty *cp, fstate_src_ty *);

/**
  * The change_run_history_query_command function is used to obtain the
  * head revision number of the history of the given source file.
  *
  * @param cp
  *     The change to operate within.
  * @param src
  *     The source file meta-data of the file of interest.
  * @returns
  *     Pointer to string containing the version.  Use str_free() when
  *     you are done with it.
  */
string_ty *change_run_history_query_command(change_ty *cp, fstate_src_ty *src);

void change_run_history_label_command(change_ty *cp, fstate_src_ty *,
	string_ty *label);

/**
  * The change_run_history_transaction_begin_command function is
  * used to run the history_transaction_begin_command in the project
  * configuration file.  This is used by the aeipass(1) command before
  * any history put or create commands.
  *
  * @param cp
  *     The change to operate within.
  */
void change_run_history_transaction_begin_command(change_ty *cp);

/**
  * The change_run_history_transaction_end_command function is
  * used to run the history_transaction_end_command in the project
  * configuration file.  This is used by the aeipass(1) command after
  * any history put or create commands.
  *
  * @param cp
  *     The change to operate within.
  */
void change_run_history_transaction_end_command(change_ty *cp);

/**
  * The change_run_history_transaction_abort_command function is
  * used to run the history_transaction_abort_command in the project
  * configuration file.  This is used by the aeipass(1) command if
  * a history transaction needs to be aborted.
  *
  * @param cp
  *     The change to operate within.
  */
void change_run_history_transaction_abort_command(change_ty *cp);

void change_history_trashed_fingerprints(change_ty *,
    struct string_list_ty *);
void change_run_diff_command(change_ty *cp, struct user_ty *up,
	string_ty *original, string_ty *input, string_ty *output);
void change_run_diff3_command(change_ty *cp, struct user_ty *up,
	string_ty *original, string_ty *most_recent, string_ty *input,
	string_ty *output);
void change_run_merge_command(change_ty *cp, struct user_ty *up,
	string_ty *original, string_ty *most_recent, string_ty *input,
	string_ty *output);
void change_run_patch_diff_command(change_ty *cp, struct user_ty *up,
	string_ty *original, string_ty *input, string_ty *output,
	string_ty *index_name);
void change_run_annotate_diff_command(change_ty *cp, struct user_ty *up,
	string_ty *original, string_ty *input, string_ty *output,
	string_ty *index_name, const char *diff_option);
int change_has_merge_command(change_ty *);
void change_run_integrate_begin_command(change_ty *);
void change_run_integrate_begin_undo_command(change_ty *);
void change_run_develop_begin_command(change_ty *, struct user_ty *);
void change_run_develop_begin_undo_command(change_ty *, struct user_ty *);
int change_run_test_command(change_ty *cp, struct user_ty *up, string_ty *,
	string_ty *, int, int, const nstring_list &variable_assignments);
int change_run_development_test_command(change_ty *cp, struct user_ty *up,
	string_ty *, string_ty *, int, int,
	const nstring_list &variable_assignments);
void change_run_build_command(change_ty *);
void change_run_build_time_adjust_notify_command(change_ty *);
void change_run_development_build_command(change_ty *, struct user_ty *,
	struct string_list_ty *);

/**
  * The change_run_develop_end_policy_command function is used to
  * run the develop_end_policy_command specified in the project
  * configuration file.
  *
  * @param cp
  *     The change in question.
  * @param up
  *     The developer of the change.
  * @note
  *     This function does not return if the command exits with a
  *     non-zero exit status.
  */
void change_run_develop_end_policy_command(change_ty *cp, user_ty *up);

string_ty *change_file_whiteout(change_ty *, string_ty *);
void change_file_whiteout_write(change_ty *, string_ty *, struct user_ty *);
void change_become(change_ty *);
void change_become_undo(void);
void change_developer_become(change_ty *);
void change_developer_become_undo(void);
int change_umask(change_ty *);
void change_development_directory_clear(change_ty *);
void change_integration_directory_clear(change_ty *);
void change_architecture_clear(change_ty *);
void change_architecture_add(change_ty *, string_ty *);
void change_architecture_query(change_ty *);
string_ty *change_architecture_name(change_ty *, int);
string_ty *change_run_architecture_discriminator_command(change_ty *cp);
cstate_architecture_times_ty *change_architecture_times_find(change_ty *,
	string_ty *);
void change_build_time_set(change_ty *);

/**
  * The change_test_time_set function is used to set the test time for a
  * change.  The architecture of the currently executing session is used.
  *
  * @param cp
  *     The change in question.
  * @param when
  *     The time the test was performed.
  */
void change_test_time_set(change_ty *cp, time_t when);

/**
  * The change_test_time_set function is used to set the test time for a
  * change, for the given architecture variant.
  *
  * @param cp
  *     The change in question.
  * @param variant
  *     The name of the architecture of interest.
  * @param when
  *     The time the test was performed.
  */
void change_test_time_set(change_ty *cp, string_ty *variant, time_t when);

/**
  * The change_test_baseline_time_set function is used to set the
  * baseline test time for a change.  The architecture of the currently
  * executing session is used.
  *
  * @param cp
  *     The change in question.
  * @param when
  *     The time the test was performed.
  */
void change_test_baseline_time_set(change_ty *cp, time_t when);

/**
  * The change_test_baseline_time_set function is used to set the test
  * time for a change, for the given architecture variant.
  *
  * @param cp
  *     The change in question.
  * @param variant
  *     The name of the architecture of interest.
  * @param when
  *     The time the test was performed.
  */
void change_test_baseline_time_set(change_ty *cp, string_ty *variant,
    time_t when);

/**
  * The change_regression_test_time_set function is used to set (or
  * clear) the regression test time stamp of the chanegh set.
  *
  * @param cp
  *     The change set in question.
  * @param when
  *     The time the test was performed, or zero to clear the time.
  * @param arch_name
  *     The name of the architecture, or the NULL pointer to mean "the
  *     current one".
  */
void change_regression_test_time_set(change_ty *cp, time_t when,
    string_ty *arch_name = 0);

void change_test_times_clear(change_ty *);
void change_build_times_clear(change_ty *);
void change_architecture_from_pconf(change_ty *);

const char *change_outstanding_builds(change_ty *, time_t);
const char *change_outstanding_tests(change_ty *, time_t);
const char *change_outstanding_tests_baseline(change_ty *, time_t);
const char *change_outstanding_tests_regression(change_ty *, time_t);

int change_pathconf_name_max(change_ty *);
string_ty *change_filename_check(change_ty *, string_ty *);

void change_create_symlinks_to_baseline(change_ty *, struct user_ty *,
    const struct work_area_style_ty &);

/**
  * The change_maintain_symlinks_to_baseline is used to repair the
  * symbolic links (etc) as dictated by the development_directory_style
  * field of the project configuration file.
  *
  * The "symlink" in the name is an historical accident, it also
  * maintains the hard links and copies as well.
  *
  * \note
  *      This function is NOT to be called by aeb, because it needs
  *      different logic.
  * \note
  *     This function may only be called when the change is in the
  *     "being developed" state.
  */
void change_maintain_symlinks_to_baseline(change_ty *, struct user_ty *);

void change_remove_symlinks_to_baseline(change_ty *, struct user_ty *,
    const struct work_area_style_ty &);

void change_rescind_test_exemption(change_ty *);
string_ty *change_cstate_filename_get(change_ty *);

/**
  * The change_fstate_filename_get function is used to obtain the
  * absolute path of the file which holds the change's file state.
  *
  * @param cp
  *     The change being operated on.
  * @returns
  *     a string.  DO NOT str_free it when you are done with it,
  *     it is cached.
  */
string_ty *change_fstate_filename_get(change_ty *cp);

/**
  * The change_pfstate_filename_get function is used to obtain the
  * absolute path of the file which holds the delta's cache of the
  * project file state.
  *
  * @param cp
  *     The change being operated on.
  * @returns
  *     a string.  DO NOT str_free it when you are done with it,
  *     it is cached.
  */
string_ty *change_pfstate_filename_get(change_ty *cp);

void change_rescind_test_exemption_undo(change_ty *);
void change_force_regression_test_exemption(change_ty *);
void change_force_regression_test_exemption_undo(change_ty *);

void change_check_architectures(change_ty *);
string_ty *change_new_test_filename_get(change_ty *, long, int);
string_ty *change_development_directory_template(change_ty *,
	struct user_ty *);
string_ty *change_metrics_filename_pattern_get(change_ty *);

/**
  * The change_is_awaiting_development function returns true if the
  * given change is in the awaiting development state, and false if it
  * is not.
  */
bool change_is_awaiting_development(change_ty *);

/**
  * The change_is_being_developed function returns true (non-zero) if
  * the given change is in the being developed state, and false (zero)
  * if it is not.
  */
int change_is_being_developed(change_ty *);

/**
  * The change_is_being_integrated function is used to determine whether
  * or not a change is in the being integrated state.
  *
  * @param cp
  *     The change in question.
  * @returns
  *     bool; true if the given change is in the being integarted state,
  *     and false if it is not.
  */
bool change_is_being_integrated(change_ty *cp);

/**
  * The change_is_completed function returns true (non-zero) if the given
  * change is in the completed state, and false (zero) if it is not.
  */
int change_is_completed(change_ty *);

/**
  * The change_delta_number_get function is used to get the delta number
  * of a change, or zero if the change is not yet completed.
  */
long change_delta_number_get(change_ty *);

/**
  * The change_brief_description_get function may be used to get the
  * brief_descriotion field of the change attributes.
  */
string_ty *change_brief_description_get(change_ty *);

/**
  * The change_uuid_set function is used to set a change's UUID,
  * if it has not been set already.
  */
void change_uuid_set(change_ty *cp);

/**
  * The change_uuid_clear function is used to clear a change's UUID,
  * if it has been set in the past.
  */
void change_uuid_clear(change_ty *cp);

/**
  * The change_reviewer_list function is used to get the list of
  * reviewers since the last develop_end.  There can be more than one
  * if the review_policy_command filed of the project configuration is
  * being used.
  *
  * \param cp
  *     The change to be consulted.
  * \param result
  *     where to put the answers.
  */
void change_reviewer_list(change_ty *cp, string_list_ty &result);

/**
  * The change_reviewer_already function is used to determine if a user
  * has already reviewed a change, since the last develop end.
  *
  * \param cp
  *     The change to be consulted.
  * \param login
  *     The login name of the user in question.
  * \returns
  *     bool; true if the user has reviewed already, false if not.
  */
bool change_reviewer_already(change_ty *cp, string_ty *login);

/**
  * The change_when_get function is used to obtain the last (most
  * recent) time for the given state transition.
  *
  * @param cp
  *     The change in qiestion.
  * @param what
  *     The state transition to look for.
  * @returns
  *     time_t; the time of the event, or 0 if not found.
  */
time_t change_when_get(change_ty *cp, cstate_history_what_ty what);

/**
  * The change_diff_required function is used to see of the project (and
  * this specific change set) need the diff command to be run.
  *
  * @param cp
  *     The change in question.
  * @returns
  *     bool; true if diff command needs to be run, false if not.
  */
bool change_diff_required(change_ty *cp);

/**
  * The change_build_required function is used to see of the project (and
  * this specific change set) need the build command to be run.
  *
  * @param cp
  *     The change in question.
  * @returns
  *     bool; true if build command needs to be run, false if not.
  */
bool change_build_required(change_ty *cp);

#endif // LIBAEGIS_CHANGE_H
