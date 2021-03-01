//
// aegis - project change supervisor
// Copyright (C) 1995-2009, 2011, 2012 Peter Miller
// Copyright (C) 2007, 2008 Walter Franzini
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef LIBAEGIS_CHANGE_H
#define LIBAEGIS_CHANGE_H

#include <libaegis/cstate.fmtgen.h>
#include <libaegis/fstate.fmtgen.h>
#include <libaegis/pconf.fmtgen.h>
#include <libaegis/view_path.h>
#include <libaegis/zero.h>

class nstring_list; // forward

//
// Name of the project configuration file,
// relative to the baseline/devdir root.
//
#define THE_CONFIG_FILE_OLD "config"
#define THE_CONFIG_FILE_NEW "aegis.conf"

#include <libaegis/user.h>

//
// Define a magic number to represent the trunk transaction
// of the project.  It has an exceptional file name.
//
#define TRUNK_CHANGE_NUMBER ((long)((~(unsigned long)0) >> 1))

#define TIME_NOT_SET (time_t)0
#define UUID_NOT_SET (string_ty*)0

struct string_list_ty; // forward
struct sub_context_ty; // forward
struct symtab_ty; // forward
class project; // forward

class change
{
public:
    /**
      * The pointer typedef is provided so that clients of this
      * interface may use change::pointer everywhere they need to point
      * at a change instance, and only this typedef needs to be edited
      * to use a variety of smart pointer instead.
      */
    typedef change *pointer;

    long                reference_count;
    project          *pp;
    long                number;
    cstate_ty           *cstate_data;
    string_ty           *cstate_filename;
    int                 cstate_is_a_new_file;
    fstate_ty           *fstate_data;
    symtab_ty           *fstate_stp;
    symtab_ty           *fstate_uuid_stp;
    string_ty           *fstate_filename;
    fstate_ty           *pfstate_data;
    symtab_ty           *pfstate_stp;
    symtab_ty           *pfstate_uuid_stp;
    string_ty           *pfstate_filename;
    int                 fstate_is_a_new_file;
    string_ty           *top_path_unresolved;
    string_ty           *top_path_resolved;
    string_ty           *development_directory_unresolved;
    string_ty           *development_directory_resolved;
    string_ty           *integration_directory_unresolved;
    string_ty           *integration_directory_resolved;
    string_ty           *logfile;
    pconf_ty            *pconf_data;
    long                lock_magic;
    int                 bogus;
    string_ty           *architecture_name;
    string_list_ty      *file_list[view_path_MAX];
    bool                project_specific_setenv_performed;

    // NOTE: methods are sorted alphabetically.  To have methods
    // grouped, make sure they change their suffix rather than their
    // prefix.  E.g. thingumy_get and thingumy_set rather than
    // get_thingumy and set_thingumy.

    // ----------  A  ------------------------------------------------------

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

    // ----------  B  ------------------------------------------------------

    /**
      * The brief_description_get method is used to obtain the brief
      * description of this change set.
      */
    nstring brief_description_get(void);

    // ----------  C  ------------------------------------------------------

    /**
      * The copyright_years_get_string is used to get a string
      * containing the list fo copyrigth years, a union of the project's
      * copyright years and this change set's copyright years.
      */
    nstring copyright_years_get_string(void);

    /**
      * The completion_timestamp method is used to obtain the time that
      * the change set completed integration, or "now" if it is not yet
      * complete.  use the #is_completed method to distinguish the two cases.
      */
    time_t completion_timestamp(void);

    /**
      * The creator_name method is used to obtain the Unix name of the
      * user that created this change.
      *
      * @returns
      *     The string value of the Unix name or NULL if not defined.
      *     DO NOT str_free it when you are done with it, it is cached.
      */
    string_ty *creator_name(void);

    /**
      * The cstate_filename_get method is used to obtain the name of
      * the file containing cstate meta-data of this change.
      *
      * @returns
      *     a string.  DO NOT str_free it when you are done with it,
      *     it is cached.
      */
    string_ty *cstate_filename_get(void);

    /**
      * The cstate_get method may be used to obtain the cstate meta-data
      * of this change.
      */
    cstate_ty *cstate_get(void);

    /**
      * THe cstate_write method may be used to write the cstate
      * meta-data of this change
      */
    void cstate_write(void);

    // ----------  D  ------------------------------------------------------

    /**
     * The delta_number_get method is used to get the delta number of a
     * change, or zero if the change is not yet completed.
     */
    long delta_number_get(void);

    /**
      * The description_get method is used to obtain the long
      * description of this change set.
      */
    nstring description_get(void);

    /**
      * The developer_name method is used to obtain the Unix name of
      * the developer of this change.
      *
      * @returns
      *     The string value of the Unix name or NULL if not defined.
      *     DO NOT str_free it when you are done with it, it is
      *     cached.
      */
    string_ty *developer_name(void);

    /**
      * The developer_user_get method is used to obtain a user_ty
      * instance representing the (most recent) developer of this change
      * set.
      */
    user_ty::pointer developer_user_get(void);

    // ----------  E  ------------------------------------------------------

    // ----------  F  ------------------------------------------------------

    /**
      * The file_exists method is used to determine whether or not a
      * file exists in the change set.  (view_path == view_path_first)
      *
      * @param filename
      *     The base-relative name of the file to search for.
      */
    bool file_exists(const nstring &filename);

    /**
      * The file_find method is used to locate a change file similar to
      * the file named.
      *
      * @param filename
      *     The name of the file to be found.
      * @param vp
      *     The style of view path to be used.
      */
    fstate_src_ty *file_find(const nstring &filename, view_path_ty vp);

    /**
      * The file_find method is used to locate a change file similar to
      * the one indicated.  If possible it uses the UUID otherwise (for
      * backwards compatibility) it uses the file name.
      *
      * @param src
      *     The meta-data of the file to be found.
      * @param vp
      *     The style of view path to be used.
      */
    fstate_src_ty *file_find(fstate_src_ty *src, view_path_ty vp);

    /**
      * The file_find method is used to locate a change file similar to
      * the one indicated.  If possible it uses the UUID otherwise (for
      * backwards compatibility) it uses the file name.
      *
      * @param src
      *     The meta-data of the file to be found.
      * @param vp
      *     The style of view path to be used.
      */
    fstate_src_ty *file_find(cstate_src_ty *src, view_path_ty vp);

    /**
      * The file_find method is used to locate a change file similar to
      * the file named, in the change set itself
      * (view_path == view_path_first).
      *
      * @param filename
      *     The name of the file to be found.
      */
    fstate_src_ty *file_find(const nstring &filename);

    /**
      * The file_find_fuzzy method is used to locate a change file with
      * a name similar to the one given.
      *
      * @param filename
      *     The name of the file to be found.
      * @param vp
      *     The style of view path to be used.
      */
    fstate_src_ty *file_find_fuzzy(const nstring &filename, view_path_ty vp);

    /**
      * The file_find_uuid method is used to find a source file given
      * the UUID.
      *
      * @param uuid
      *     The UUID to search for.
      * @param view_path
      *     The style and depth of search for the file.
      * @returns
      *     a pointer to the file information, or NULL if no file has the
      *     specified UUID.
      */
    fstate_src_ty *file_find_uuid(string_ty *uuid, view_path_ty view_path);

    /**
      * The file_find_uuid method is used to find a source file given
      * the UUID.
      *
      * @param uuid
      *     The UUID to search for.
      * @param view_path
      *     The style and depth of search for the file.
      * @returns
      *     a pointer to the file information, or NULL if no file has the
      *     specified UUID.
      */
    fstate_src_ty *file_find_uuid(const nstring &uuid, view_path_ty view_path);

    /**
      * The file_new method is used to add a new file to a change set's
      * file manifest.  No checking is poerformed, it is up to the caller
      * to ensure that the name is unique.
      *
      * @param file_name
      *     The name of the file to be created.
      * @returns
      *     A newly allocated file, with the name set, and everything
      *     else blank.
      */
    fstate_src_ty *file_new(string_ty *file_name);

    /**
      * The file_new method is used to add a new file to a change
      * set's file manifest.  No checking is poerformed, it is up to
      * the caller to ensure that the file name and UUID are unique.
      * Typically this is used when adding a project file to a change
      * set for modification or removal.
      *
      * @param meta
      *     The meta-data of the file to be created.
      * @returns
      *     A newly allocated file, with the name set, and everything
      *     else blank.
      */
    fstate_src_ty *file_new(fstate_src_ty *meta);

    /**
      * The file_path method is used to obtain the absolute path to
      * the given change file.
      *
      * @param file_name
      *     The name of the file in question.
      * @returns
      *     a string containing the absolute path, or NULL if the file is
      *     not a change source file.
      */
    string_ty *file_path(string_ty *file_name);

    /**
      * The file_path method is used to obtain the absolute path to
      * the given change file.
      *
      * @param file_name
      *     The name of the file in question.
      * @returns
      *     a string containing the absolute path, or the empty string
      *     if the file is not a change source file.
      */
    nstring file_path(const nstring &file_name);

    /**
      * The file_path method is used to obtain the absolute path to
      * the given change file.
      *
      * @param src
      *     The meta-data of the file in question.
      * @returns
      *     a string containing the absolute path, or NULL if the file is
      *     not a change source file.
      */
    string_ty *file_path(fstate_src_ty *src);

    /**
      * The file_path_by_uuid method is used to obtain the absolute
      * path to the given change file.
      *
      * @param uuid
      *     The UUID of the file in question.
      * @returns
      *     a string containing the absolute path, or NULL if the file is
      *     not a change source file.
      */
    string_ty *file_path_by_uuid(string_ty *uuid);

    /**
      * The file_path_by_uuid method is used to obtain the absolute
      * path to the given change file.
      *
      * @param uuid
      *     The UUID of the file in question.
      * @returns
      *     a string containing the absolute path, or NULL if the file is
      *     not a change source file.
      */
    nstring file_path_by_uuid(const nstring &uuid);

    /**
      * The file_promote method is used to check whether or not recent
      * integrations have change the actions the change files must
      * perform.
      *
      * If two changes are creating the same file, the first one integrated
      * means that the second one must update its action to "modify".
      *
      * If two changes are removing the same file, the first one integrated
      * means that the second one needs to drop the file from its list.
      *
      * If one change is removing a file, and a second change is modifying
      * the same file, after the first change is integrated, the second
      * change must update its action to "create".
      *
      * @returns
      *     true if anything changed, false if nothing changed.
      */
    bool file_promote(void);

    /**
      * The file_resolve_name method is used to resolve an arbitrary
      * UNIX pathname (relative or absolute) into a base relative path
      * within a change set's search path.
      *
      * @param up
      *     The user invoking the program.
      * @param file_name
      *     The file name to be resolved.
      * @returns
      *     The resolved base relative file name.  Use str_free when you are
      *     done with it.
      */
    string_ty *file_resolve_name(user_ty::pointer up, string_ty *file_name);

    /**
      * The file_resolve_name method is used to resolve an arbitrary
      * UNIX pathname (relative or absolute) into a base relative path
      * within a change set's search path.
      *
      * @param up
      *     The user invoking the program.
      * @param file_name
      *     The file name to be resolved.
      * @returns
      *     The resolved base relative file name.  Use str_free when you are
      *     done with it.
      */
    nstring file_resolve_name(user_ty::pointer up, const nstring &file_name);

    /**
      * The file_resolve_names method is used to resolve arbitrary UNIX
      * pathnames (relative or absolute) into base relative paths within
      * a change set's search path.
      *
      * @param up
      *     The user invoking the program.
      * @param file_names
      *     The file names to be resolved.
      *     This parameter ISN'T const because this will be done in situ.
      */
    void file_resolve_names(user_ty::pointer up, string_list_ty &file_names);

    /**
      * The file_unchanged method is used to determine whether a source
      * file is unchanged compared to the file in the baseline.
      *
      * @param src_data
      *     The file in question.
      * @param up
      *     The user to perform file actions as.
      * @returns
      *     bool; true if the file is unchanged, false if the file has
      *     changed, and false if the comparison isn't meaningful.
      */
    bool file_unchanged(fstate_src_ty *src_data, user_ty::pointer up);

    /**
      * The file_is_config method is used to determine whether or not
      * the named file is a configuration file.
      *
      * @param name
      *     The name of the file of interest
      * @returns
      *     true if it is a config file, false if not.
      */
    bool file_is_config(string_ty *name);

    /**
      * The download_files_acessable method may be used to to determine
      * whether or not the calling user has permission to access the
      * change set's download files.  This usually means read and search
      * premission on the development directory and/or the integration
      * director, unless the change set is in the completed state.
      */
    bool download_files_accessable(void);

private:
    /**
      * The fimprove method is used to cope with previous versions
      * of the file meta data, and rewrite them to conform to later
      * expectations.
      *
      * @param fstate_data
      *     The file meta data to be massaged.
      */
    void fimprove(fstate_ty *fstate_data);

public:
    /**
      * The fstate_get method is used to obtain a current value for the
      * change file state symbol table.
      *
      * @note
      *     This would be static to aegis/change_file.c if only
      *     aegis/aer/value/fstate.c did not need it.  No other place
      *     should access this directly.
      */
    fstate_ty *fstate_get(void);

    // ----------  G  ------------------------------------------------------

    /**
      * The gid_get method is used to obtain the Unix group id of the
      * change owner.
      */
    int gid_get() const;

    // ----------  H  ------------------------------------------------------

    // ----------  I  ------------------------------------------------------

    /**
      * The integration_directory_get method may be used to obtain the
      * change's integration directory path.  It is an error if the
      * change is not in the 'being integrated' state.
      *
      * @param resolve
      *     whether or not to resolve symlinks in the path.
      */
    nstring integration_directory_get(bool resolve);

    /**
      * The integrator_name method is used to obtain the Unix group
      * name of the integrator.
      *
      * @returns
      *     The string value of the Unix name or NULL if not defined.
      *     DO NOT str_free it when you are done with it, it is
      *     cached.
      */
    string_ty *integrator_name(void);

    /**
      * The is_a_branch method is used to determine whether or not this
      * change set is an active branch.  If you want to know if it was
      * ever an active branch (e.g. now completed) use the #was_a_branch
      * method.
      */
    bool is_a_branch(void);

    /**
      * The is_awaiting_development method returns true if the given
      * change is in the awaiting development state, and false if it is
      * not.
      */
    bool is_awaiting_development(void);

    /**
      * The is_being_developed method returns true if the given change
      * is in the being developed state, and false if it is not.
      */
    bool is_being_developed(void);

    /**
      * The is_being_integrated method is used to determine whether or
      * not this is in the being integrated state.
      *
      * @returns
      *     bool; true if this is in the being integrated state, and
      *     false if it is not.
      */
    bool is_being_integrated(void);

    /**
      * The change_is_completed function returns true (non-zero) if this
      * change is in the completed state, and false (zero) if it is not.
      */
    bool is_completed(void);

    // ----------  J  ------------------------------------------------------

    // ----------  K  ------------------------------------------------------

    // ----------  L  ------------------------------------------------------

    // ----------  M  ------------------------------------------------------

    // ----------  N  ------------------------------------------------------

    // ----------  O  ------------------------------------------------------

    // ----------  P  ------------------------------------------------------

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
      * The pconf_attributes_get_boolean method is used to obtain the
      * value of an attribute of the project, from the project
      * configuration file.  It could be in this change, or it could
      * be in a baseline or an ancestor baseline.
      *
      * @param name
      *     The name of the attribute.
      *     Attribute names are not case sensitive.
      * @param dflt
      *     The default value to return if no attribute available.
      * @returns
      *     the string value of the attribute, or the empty string
      *     if not found.
      */
    bool pconf_attributes_get_boolean(const nstring &name, bool dflt = false);

    /**
      * The pconf_copyright_owner_get method is used to obtain the
      * name of the copyright holder of a project.
      */
    nstring pconf_copyright_owner_get(void);

    /**
      * The pfstate_get method is used to obtain the project file state
      * information for a change set.  This is all of the project files,
      * not just one.
      */
    fstate_ty *pfstate_get(void);

    /**
      * The project_specific_setenv_performed_set method is used to
      * remember if the environment variable(s), specified as
      * project_specific attributes, has been already exported.
      *
      * @returns
      *     Void.
      */
    void project_specific_setenv_performed_set(void);

    /**
      * The project_specific_setenv_performed_get method is used to
      * know if the environment variable(s), specified as
      * project_specific attributes, has been already exported.
      *
      * @returns
      *     True if the variables has been already exported, false
      *     otherwise.
      */
    bool project_specific_setenv_performed_get() const;

    // ----------  Q  ------------------------------------------------------

    // ----------  R  ------------------------------------------------------

    /**
      * The reviewer_name method is used to determine the Unix name of
      * the (last) reviewer.
      *
      * @returns
      *     The string representation of the Unix name or NULL if not
      *     defined. DO NOT str_free it when you are done with it, it
      *     is cached.
      */
    string_ty *reviewer_name(void);

    /**
      * The run_project_file_command_needed method may be used to
      * determine whether or not the project_file_command needstosed in
      * recent integration history.
      *
      * @returns
      *     true if need to run it (see next method), false if not.
      */
    bool run_project_file_command_needed(void);

    /**
      * The run_project_file_command method is used to run the command
      * configured in the aegis.conf file's project_file_command field.
      *
      * @param up
      *     The user to run the command as.
      *
      * @note
      *     this method MUST be called outside the change set lock
      */
    void run_project_file_command(const user_ty::pointer &up);

    /**
      * The run_project_file_command_done method is used to remember
      * that the project_file comman has been (is about to be) called
      * recently.
      *
      * @note
      *     this method MUST be called INSIDE the change set lock
      */
    void run_project_file_command_done(void);

    /**
      * The run_new_file_command method is used to run the
      * new_file_command field of the aegis.conf file.
      *
      * @param slp
      *     The list of new file names
      * @param up
      *     Th euser to run the command as.
      */
    void run_new_file_command(string_list_ty *slp, const user_ty::pointer &up);

    /**
      * The run_new_file_undo_command method is used to run the command
      * in the new_file_undo_command field of the aegis.conf file.
      *
      * @param slp
      *     The list of filenames affected.
      * @param up
      *     the user to run the program as
      */
    void run_new_file_undo_command(string_list_ty *slp,
        const user_ty::pointer &up);

    /**
      * The run_new_test_command method is used to run the command in
      * the new_test_command field of the aegis.conf file.
      *
      * @param slp
      *     The list of filenames affected.
      * @param up
      *     the user to run the program as
      */
    void run_new_test_command(string_list_ty *slp,
        const user_ty::pointer &up);

    /**
      * The run_new_test_undo_command method is used to run the command
      * in the new_test_undo_command field of the aegis.conf file.
      *
      * @param slp
      *     The list of filenames affected.
      * @param up
      *     the user to run the program as
      */
    void run_new_test_undo_command(string_list_ty *slp,
        const user_ty::pointer &up);

    /**
      * The run_copy_file_command method is used to run the command in
      * the copy_file_command field of the aegis.conf file.
      *
      * @param slp
      *     The list of filenames affected.
      * @param up
      *     the user to run the program as
      */
    void run_copy_file_command(string_list_ty *slp, const user_ty::pointer &up);

    /**
      * The run_copy_file_undo_command method is used to run the command
      * in the copy_file_undo_command field of the aegis.conf file.
      *
      * @param slp
      *     The list of filenames affected.
      * @param up
      *     the user to run the program as
      */
    void run_copy_file_undo_command(string_list_ty *slp,
        const user_ty::pointer &up);

    /**
      * The run_remove_file_command method is used to run the command in
      * the remove_file_command field of the aegis.conf file.
      *
      * @param slp
      *     The list of filenames affected.
      * @param up
      *     the user to run the program as
      */
    void run_remove_file_command(string_list_ty *slp,
        const user_ty::pointer &up);

    /**
      * The run_remove_file_undo_command method is used to run the
      * command in the remove_file_undo_command field of the aegis.conf
      * file.
      *
      * @param slp
      *     The list of filenames affected.
      * @param up
      *     the user to run the program as
      */
    void run_remove_file_undo_command(string_list_ty *slp,
        const user_ty::pointer &up);

    /**
      * The run_make_transparent_command method is used to run the
      * command in the make_transparent_command field of the aegis.conf
      * file.
      *
      * @param slp
      *     The list of filenames affected.
      * @param up
      *     the user to run the program as
      */
    void run_make_transparent_command(string_list_ty *slp,
        const user_ty::pointer &up);

    /**
      * The run_make_transparent_undo_comman method is used to run
      * the command in the make_transparent_undo_command field of the
      * aegis.conf file.
      *
      * @param slp
      *     The list of filenames affected.
      * @param up
      *     the user to run the program as
      */
    void run_make_transparent_undo_command(string_list_ty *slp,
        const user_ty::pointer &up);

    /**
      * The run_forced_develop_begin_notify_command method is used to
      * run the command in the forced_develop_begin_notify_command field
      * of the aegis.conf file.
      *
      * @param up
      *     the user to run the program as
      */
    void run_forced_develop_begin_notify_command(const user_ty::pointer &up);

    /**
      * The run_develop_end_notify_command method is used to run
      * the command in the develop_end_notify_command field of the
      * aegis.conf file.
      */
    void run_develop_end_notify_command(void);

    /**
      * The run_develop_end_undo_notify_command method is used to run
      * the command in the develop_end_undo_notify_command field of the
      * aegis.conf file.
      */
    void run_develop_end_undo_notify_command(void);

    /**
      * The run_review_begin_notify_command method is used to run
      * the command in the review_begin_notify_command field of the
      * aegis.conf file.
      */
    void run_review_begin_notify_command(void);

    /**
      * The run_review_begin_undo_notify_command method is used to run
      * the command in the review_begin_undo_notify_command field of the
      * aegis.conf file.
      */
    void run_review_begin_undo_notify_command(void);

    /**
      * The run_review_pass_notify_command method is used to run
      * the command in the review_pass_notify_command field of the
      * aegis.conf file.
      */
    void run_review_pass_notify_command(void);

    /**
      * The run_review_pass_undo_notify_command method is used to run
      * the command in the review_pass_undo_notify_command field of the
      * aegis.conf file.
      */
    void run_review_pass_undo_notify_command(void);

    /**
      * The run_review_fail_notify_command method is used to run
      * the command in the review_fail_notify_command field of the
      * aegis.conf file.
      */
    void run_review_fail_notify_command(void);

    /**
      * The run_integrate_pass_notify_command method is used to run
      * the command in the integrate_pass_notify_command field of the
      * aegis.conf file.
      */
    void run_integrate_pass_notify_command(void);

    /**
      * The run_integrate_fail_notify_command method is used to run
      * the command in the integrate_fail_notify_command field of the
      * aegis.conf file.
      */
    void run_integrate_fail_notify_command(void);

    /**
      * The run_develop_begin_early_command method is used to run the
      * develop_begin_early_command from the project configuration file.
      * This is used by the aedb command, just after the development
      * directory has been created.
      *
      * @param up
      *     The user to run the command as.
      */
    void run_develop_begin_early_command(user_ty::pointer up);

    /**
      * The run_develop_begin_command method is used to run the
      * develop_begin_command from the projetc configuration file.  This
      * is used by the aedb command, just all other actions performed by
      * aedb.
      *
      * @param up
      *     The user to run the command as.
      */
    void run_develop_begin_command(user_ty::pointer up);

    // ----------  S  ------------------------------------------------------

    /**
      * The search_path_get method may be used to obtain the search path
      * (list of progressively more out-of-date ancestor branches)
      * within which to search for files.
      *
      * @param result
      *     Where to append the additional directories.
      * @param resolve
      *     Whether or not to expand synbolic links within paths.
      */
    void search_path_get(string_list_ty *result, bool resolve);

    /**
      * The search_path_get method may be used to obtain the search path
      * (list of progressively more out-of-date ancestor branches)
      * within which to search for files.
      *
      * @param result
      *     Where to append the additional directories.
      * @param resolve
      *     Whether or not to expand synbolic links within paths.
      */
    void search_path_get(nstring_list &result, bool resolve);

    // ----------  T  ------------------------------------------------------

    time_t time_limit_get(void);

    /**
      * The umask method is used to obtain the umask for this change.
      *
      * @returns
      *     An int representing the umask value.
      */
    int umask_get() const;

    // ----------  U  ------------------------------------------------------

    /**
     * The uuid_get method is used to obtain the UUID of this change set.
     *
     * @returns
     *     The uuid of the change set, or the empty string of this
     *     change set doesn't have a UUID.
     */
    nstring uuid_get(void);

    /**
     * The uuid_get_list method is used to obtain all of the UUIDs
     * associated with this change set.  This includes the change set's
     * UUID, if it has one, and all of the aoriginal-uuid attributes.
     *
     * @param uuids
     *     Where to put all of the UUIDs associated with this change set.
     */
    void uuid_get_list(nstring_list &uuids);

    // ----------  V  ------------------------------------------------------

    /**
      * The version_debian_get method is used to get the version
      * of this change set, in Debian format.  In particular, it
      * makes being-developed change sets look like they are "release
      * candidates" for the next project delta, otherwise business as
      * usual.
      */
    nstring version_debian_get(void);

    /**
      * The version_rpm_get method is used to get the version of
      * this change set, in RPM format.  In particular, it makes
      * being-developed change sets look like they are "branched from"
      * for the current project delta, otherwise business as usual.
      */
    nstring version_rpm_get(void);

    /**
      * The version_get method is used to get the version of this change
      * set.  This will consist of the branch name (1.2) combined with
      * ether a delta number (.D123) or, for incomplete changes, a
      * change number (.C123).
      */
    nstring version_get(void);

    // ----------  W  ------------------------------------------------------

    /**
      * The was_a_branch method is used to determine whether or not this
      * change set is, or was ever, an active branch.  If you want to
      * know if it is a currently active branch (beging_developed) use
      * the #is_a_branch method.
      */
    bool was_a_branch(void);

    // ----------  X  ------------------------------------------------------

    // ----------  Y  ------------------------------------------------------

    // ----------  Z  ------------------------------------------------------

    // ---------------------------------------------------------------------
    // PLEASE keep the above method declarations in alphabetical order.

    //
    // If you add instance variables to this class, don't forget to update
    // change_alloc()     in libaegis/change/alloc.cc
    // change_free()      in libaegis/change/free.cc
    // change_lock_sync() in libaegis/change/lock_sync.cc
    //
};

change::pointer change_alloc(project *, long);
void change_free(change::pointer );
change::pointer change_copy(change::pointer );
void change_bind_existing(change::pointer );
int change_bind_existing_errok(change::pointer );
void change_bind_new(change::pointer );
change::pointer change_bogus(project *);

cstate_history_ty *change_history_new(change::pointer , user_ty::pointer );

void change_top_path_set(change::pointer , string_ty *);
void change_top_path_set(change::pointer , const nstring &);
void change_development_directory_set(change::pointer , string_ty *);
void change_integration_directory_set(change::pointer , string_ty *);
string_ty *change_top_path_get(change::pointer , int);
string_ty *change_development_directory_get(change::pointer , int);
string_ty *change_integration_directory_get(change::pointer , int);
string_ty *change_logfile_basename(void);
string_ty *change_logfile_get(change::pointer );
void change_cstate_lock_prepare(change::pointer );
void change_error(change::pointer , sub_context_ty *, const char *);
void change_fatal(change::pointer , sub_context_ty *, const char *) NORETURN;
void change_verbose(change::pointer , sub_context_ty *, const char *);

/**
 * The change_warning function is used to issue a warning message
 * specific to a change.  The message will be substituted accouring
 * to aesub(5) and any additional substitutions provided in the
 * substitution context.
 */
void change_warning(change::pointer , sub_context_ty *, const char *);

/**
 * The change_warning_obsolete_field function is used to warn about the
 * use of obsolete fields (in user suppied files, Aegis will take care
 * of quietly cleaning up the meta-data files all by itself).
 */
void change_warning_obsolete_field(change::pointer cp, string_ty *errpos,
    const char *old_field, const char *new_field);

pconf_ty *change_pconf_get(change::pointer , int);

// use cp->run_new_file_command instead
void change_run_new_file_command(change::pointer cp, string_list_ty *slp,
    user_ty::pointer up) DEPRECATED;

// use cp->run_new_file_undo_command instead
void change_run_new_file_undo_command(change::pointer cp, string_list_ty *slp,
    user_ty::pointer up) DEPRECATED;

// use cp->run_new_test_command instead
void change_run_new_test_command(change::pointer cp, string_list_ty *slp,
    user_ty::pointer up) DEPRECATED;

// use cp->run_new_test_undo_command instead
void change_run_new_test_undo_command(change::pointer cp, string_list_ty *slp,
    user_ty::pointer up) DEPRECATED;

// use cp->run_copy_file_command instead
void change_run_copy_file_command(change::pointer cp, string_list_ty *slp,
    user_ty::pointer up) DEPRECATED;

// use cp->run_copy_file_undo_command instead
void change_run_copy_file_undo_command(change::pointer cp, string_list_ty *slp,
    user_ty::pointer up) DEPRECATED;

// use cp->run_remove_file_command instead
void change_run_remove_file_command(change::pointer cp, string_list_ty *slp,
    user_ty::pointer up) DEPRECATED;

// use cp->run_remove_file_undo_command instead
void change_run_remove_file_undo_command(change::pointer cp,
    string_list_ty *slp, user_ty::pointer up) DEPRECATED;

// use cp->run_make_transparent_command instead
void change_run_make_transparent_command(change::pointer cp,
    string_list_ty *slp, user_ty::pointer up) DEPRECATED;

// use cp->run_make_transparent_undo_command instead
void change_run_make_transparent_undo_command(change::pointer cp,
    string_list_ty *slp, user_ty::pointer up) DEPRECATED;

// use vp->run_project_file_command_needed() instead
int change_run_project_file_command_needed(change::pointer cp) DEPRECATED;

// use cp->run_project_file_command(up) instead
void change_run_project_file_command(change::pointer cp, user_ty::pointer up)
    DEPRECATED;

// use cp->run_forced_develop_begin_notify_command instead
void change_run_forced_develop_begin_notify_command(change::pointer cp,
    user_ty::pointer up) DEPRECATED;

// use cp->run_develop_end_notify_command instead
void change_run_develop_end_notify_command(change::pointer cp) DEPRECATED;

// use cp->run_develop_end_undo_notify_command instead
void change_run_develop_end_undo_notify_command(change::pointer cp) DEPRECATED;

// use cp->run_review_begin_notify_command instead
void change_run_review_begin_notify_command(change::pointer cp) DEPRECATED;

// use cp->run_review_begin_undo_notify_command
void change_run_review_begin_undo_notify_command(change::pointer cp) DEPRECATED;

// use cp->run_review_pass_notify_command instead
void change_run_review_pass_notify_command(change::pointer cp) DEPRECATED;

// use cp->run_review_pass_undo_notify_command instead
void change_run_review_pass_undo_notify_command(change::pointer cp) DEPRECATED;

// use cp->run_review_fail_notify_command instead
void change_run_review_fail_notify_command(change::pointer cp) DEPRECATED;

// use cp->run_integrate_pass_notify_command instead
void change_run_integrate_pass_notify_command(change::pointer cp) DEPRECATED;

// use cp->run_integrate_fail_notify_command instead
void change_run_integrate_fail_notify_command(change::pointer cp) DEPRECATED;

void change_run_history_get_command(change::pointer cp, fstate_src_ty *src,
    string_ty *output_file, user_ty::pointer up);
void change_run_history_create_command(change::pointer cp, fstate_src_ty *);
void change_run_history_put_command(change::pointer cp, fstate_src_ty *);

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
string_ty *change_run_history_query_command(change::pointer cp,
    fstate_src_ty *src);

void change_run_history_label_command(change::pointer cp, fstate_src_ty *,
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
void change_run_history_transaction_begin_command(change::pointer cp);

/**
 * The change_run_history_transaction_end_command function is
 * used to run the history_transaction_end_command in the project
 * configuration file.  This is used by the aeipass(1) command after
 * any history put or create commands.
 *
 * @param cp
 *     The change to operate within.
 */
void change_run_history_transaction_end_command(change::pointer cp);

/**
 * The change_run_history_transaction_abort_command function is
 * used to run the history_transaction_abort_command in the project
 * configuration file.  This is used by the aeipass(1) command if
 * a history transaction needs to be aborted.
 *
 * @param cp
 *     The change to operate within.
 */
void change_run_history_transaction_abort_command(change::pointer cp);

void change_history_trashed_fingerprints(change::pointer , string_list_ty *);
void change_run_diff_command(change::pointer cp, user_ty::pointer up,
    string_ty *original, string_ty *input, string_ty *output);
void change_run_diff3_command(change::pointer cp, user_ty::pointer up,
    string_ty *original, string_ty *most_recent, string_ty *input,
    string_ty *output);
void change_run_merge_command(change::pointer cp, user_ty::pointer up,
    string_ty *original, string_ty *most_recent, string_ty *input,
    string_ty *output);
void change_run_patch_diff_command(change::pointer cp, user_ty::pointer up,
    string_ty *original, string_ty *input, string_ty *output,
    string_ty *index_name);
void change_run_annotate_diff_command(change::pointer cp, user_ty::pointer up,
    string_ty *original, string_ty *input, string_ty *output,
    string_ty *index_name, const char *diff_option);
int change_has_merge_command(change::pointer );
void change_run_integrate_begin_command(change::pointer );
void change_run_integrate_begin_undo_command(change::pointer );

void change_run_develop_begin_command(change::pointer cp, user_ty::pointer up)
    DEPRECATED;

void change_run_develop_begin_undo_command(change::pointer cp,
    user_ty::pointer up);
int change_run_test_command(change::pointer cp, user_ty::pointer up,
    string_ty *, string_ty *, int, int,
    const nstring_list &variable_assignments);
int change_run_development_test_command(change::pointer cp, user_ty::pointer up,
    string_ty *, string_ty *, int, int,
    const nstring_list &variable_assignments);
void change_run_build_command(change::pointer );
void change_run_build_time_adjust_notify_command(change::pointer );
void change_run_development_build_command(change::pointer , user_ty::pointer ,
    string_list_ty *);

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
void change_run_develop_end_policy_command(change::pointer cp,
    user_ty::pointer up);

string_ty *change_file_whiteout(change::pointer , string_ty *);
void change_file_whiteout_write(change::pointer cp, string_ty *path,
    user_ty::pointer up);
void change_become(change::pointer );
void change_become_undo(change::pointer cp);
void change_developer_become(change::pointer );
void change_developer_become_undo(change::pointer cp);

void change_development_directory_clear(change::pointer );
void change_integration_directory_clear(change::pointer );
void change_architecture_clear(change::pointer );
void change_architecture_add(change::pointer , string_ty *);
void change_architecture_query(change::pointer );
string_ty *change_architecture_name(change::pointer , int);
string_ty *change_run_architecture_discriminator_command(change::pointer cp);
cstate_architecture_times_ty *change_architecture_times_find(change::pointer ,
                                                             string_ty *);
void change_build_time_set(change::pointer );

/**
 * The change_test_time_set function is used to set the test time for a
 * change.  The architecture of the currently executing session is used.
 *
 * @param cp
 *     The change in question.
 * @param when
 *     The time the test was performed.
 */
void change_test_time_set(change::pointer cp, time_t when);

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
void change_test_time_set(change::pointer cp, string_ty *variant, time_t when);

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
void change_test_baseline_time_set(change::pointer cp, time_t when);

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
void change_test_baseline_time_set(change::pointer cp, string_ty *variant,
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
void change_regression_test_time_set(change::pointer cp, time_t when,
    string_ty *arch_name = 0);

void change_test_times_clear(change::pointer );
void change_build_times_clear(change::pointer );
void change_architecture_from_pconf(change::pointer );

const char *change_outstanding_builds(change::pointer , time_t);
const char *change_outstanding_tests(change::pointer , time_t);
const char *change_outstanding_tests_baseline(change::pointer , time_t);
const char *change_outstanding_tests_regression(change::pointer , time_t);

int change_pathconf_name_max(change::pointer );
string_ty *change_filename_check(change::pointer , string_ty *);

void change_create_symlinks_to_baseline(change::pointer , user_ty::pointer ,
    const work_area_style_ty &);

/**
 * The change_maintain_symlinks_to_baseline is used to repair the
 * symbolic links (etc) as dictated by the development_directory_style
 * field of the project configuration file.
 *
 * The "symlink" in the name is an historical accident, it also
 * maintains the hard links and copies as well.
 *
 * @param cp
 *     The change to operatte on
 * @param up
 *     The user to operate as
 * @param undoing
 *     True if this call is in response to and undo operation (aecpu,
 *     aemtu) because they need special attention for file time stamps.
 *     Defaults to false if not specified.
 *
 * @note
 *      This function is NOT to be called by aeb, because it needs
 *      different logic.
 * @note
 *     This function may only be called when the change is in the
 *     "being developed" state.
 */
void change_maintain_symlinks_to_baseline(change::pointer cp,
    user_ty::pointer up, bool undoing = false);

void change_remove_symlinks_to_baseline(change::pointer , user_ty::pointer ,
    const work_area_style_ty &);

void change_rescind_test_exemption(change::pointer );

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
string_ty *change_fstate_filename_get(change::pointer cp);

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
string_ty *change_pfstate_filename_get(change::pointer cp);

void change_rescind_test_exemption_undo(change::pointer );
void change_force_regression_test_exemption(change::pointer );
void change_force_regression_test_exemption_undo(change::pointer );

void change_check_architectures(change::pointer );
string_ty *change_new_test_filename_get(change::pointer , long, int);
string_ty *change_development_directory_template(change::pointer ,
                                                 user_ty::pointer );
string_ty *change_metrics_filename_pattern_get(change::pointer );

/**
 * The change_uuid_set function is used to set a change's UUID,
 * if it has not been set already.
 */
void change_uuid_set(change::pointer cp);

/**
 * The change_uuid_clear function is used to clear a change's UUID,
 * if it has been set in the past.
 */
void change_uuid_clear(change::pointer cp);

/**
 * The change_reviewer_list function is used to get the list of
 * reviewers since the last develop_end.  There can be more than one
 * if the review_policy_command filed of the project configuration is
 * being used.
 *
 * @param cp
 *     The change to be consulted.
 * @param result
 *     where to put the answers.
 */
void change_reviewer_list(change::pointer cp, string_list_ty &result);

/**
  * The change_reviewer_already function is used to determine if a user
  * has already reviewed a change, since the last develop end.
  *
  * @param cp
  *     The change to be consulted.
  * @param login
  *     The login name of the user in question.
  * @returns
  *     bool; true if the user has reviewed already, false if not.
  */
bool change_reviewer_already(change::pointer cp, string_ty *login);

/**
  * The change_reviewer_already function is used to determine if a user
  * has already reviewed a change, since the last develop end.
  *
  * @param cp
  *     The change to be consulted.
  * @param login
  *     The login name of the user in question.
  * @returns
  *     bool; true if the user has reviewed already, false if not.
  */
bool change_reviewer_already(change::pointer cp, const nstring &login);

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
time_t change_when_get(change::pointer cp, cstate_history_what_ty what);

/**
 * The change_diff_required function is used to see of the project (and
 * this specific change set) need the diff command to be run.
 *
 * @param cp
 *     The change in question.
 * @returns
 *     bool; true if diff command needs to be run, false if not.
 */
bool change_diff_required(change::pointer cp);

/**
 * The change_build_required function is used to see of the project (and
 * this specific change set) need the build command to be run.
 *
 * @param cp
 *     The change in question.
 * @param conf_exists
 *     true if the project configuration file must exist (causing fatal
 *     error if it does not), or false if it doesn't matter.
 * @returns
 *     bool; true if build command needs to be run, false if not.
 */
bool change_build_required(change::pointer cp, bool conf_exists = true);

// vim: set ts=8 sw=4 et :
#endif // LIBAEGIS_CHANGE_H
