//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
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

#ifndef LIBAEGIS_CHANGE_IDENTIFI_SUB_H
#define LIBAEGIS_CHANGE_IDENTIFI_SUB_H

#include <libaegis/change.h>
#include <libaegis/file_revision.h>
#include <libaegis/project/identifi_sub/branch.h>

class change_functor; // forward
struct fstate_src_ty; // forward
class nstring_list; // forward
class project_file_roll_forward; // forward
struct project_ty; // forward
struct user_ty; // forward

/**
  * The change_identifier_subset class is used to represent a change
  * and its context, as identified by all the diffent command line
  * options used to specify a change set.  It only operates on the
  * change portion of the identification; the project portion is a
  * separate class and instance.
  */
class change_identifier_subset
{
public:
    /**
      * The destructor.
      * It isn't virtual; thou shalt not derive from this class.
      */
    ~change_identifier_subset();

    /**
      * The constructor.
      */
    change_identifier_subset(project_identifier_subset &arg);

    /**
      * The set method is used to determine if this chaneg ID has been
      * set yet (via any one of several command line options).
      */
    bool set() const;

    /**
      * The command_line_parse method is used to parse command line
      * options (via the arglex() fucntion) to set the change ID.  The
      * current token is expected to be meaningful for identifying a
      * change.  The position will be advanced past all relevant tokens.
      *
      * It understands all of the following command line options:
      *     -BaseLine
      *     [ -Change] number
      *     -Development_Directory
      *     -BRanch number
      *     -TRunk
      *     -GrandParent
      *     -DELta number
      *     -DELta name
      *     -DELta_Date when
      *     [ -Project ] string
      *
      * \note
      *     There is no need to pass all of these command line options to
      *     this function for processing.  Only pass those options which
      *     make sense.  If is often the case that threr are actually
      *     *two* changes being identified, and they will split the
      *     arguments between them.
      */
    void command_line_parse(void (*usage)(void));

    /**
      * The command_line_check method is used to verify that sensable
      * command line options have been specified, once the parse has
      * completed.
      */
    void command_line_check(void (*usage)(void));

    /**
      * The set_baseline method is used to specify that the baseline is
      * the change being identified.
      */
    void set_baseline() { baseline = true; }

    /**
      * The get_baseline method is used to determine whether the
      * --baseline option has been specified.
      */
    bool get_baseline() { return baseline; }

    /**
      * The get_devdir method is used to determine of the
      * --Development-Directory option has been given, or is implied.
      */
    bool get_devdir();

    /**
      * The set_project method is used <i>after</i> the
      * command_line_check method has been called.  It is used to bind
      * to the appropriate project for the change identified.
      */
    void set_project();

    /**
      * The set_change method is used <i>after</i> the set_project
      * method has been called.  It is used to bind to the appropriate
      * change.
      */
    void set_change();

    /**
      * The get_file_revision is used to determine the path to the given
      * file at the time specified by the change ID.  It must be called
      * <i>after</i> the set_change method has been called.
      *
      * @param filename
      *     The name of the file in question
      * @param bad_state
      *     What to do if the change is in an inappropriate state for
      *     this operation.
      */
    file_revision get_file_revision(const nstring &filename,
	change_functor &bad_state);

    /**
      * The get_file_revision is used to determine the path to
      * the given file at the time specified by the change ID.  It must
      * be called <i>after</i> the set_change method has been called.
      *
      * @param src
      *     The file meta data (revions, if set, will be ignored)
      * @param bad_state
      *     What to do if the change is in an inappropriate state for
      *     this operation.
      */
    file_revision get_file_revision(fstate_src_ty *src,
	change_functor &bad_state);

    /**
      * The get_project_files method is used to obtain a list of the
      * project's file, taking --delta options into account if specified.
      *
      * \param results
      *     The string list to which the file names are appended.
      *
      * \note
      *     You can't specify a view path, it defaults to view_path_simple.
      */
    void get_project_file_names(nstring_list &results);

    /**
      * The get_project_file method is used to obtain the details about
      * a named file, taking any --delta* options into account.
      *
      * \param file_name
      *     The name of the file of interest.
      * \returns
      *     pointer to file details, or NULL of the file does not exist.
      */
    fstate_src_ty *get_project_file(const nstring &file_name);

    /**
      * The set_project_name method is used to set the name of the
      * project to be accessed.
      */
    void set_project_name(string_ty *);

    /**
      * The get_change_version_string methof is used to get the version
      * string for the change.
      */
    nstring get_change_version_string();

    /**
      * The get_pp method is used to get the project pointer for the
      * change identified.
      */
    project_ty *get_pp() { return pid.get_pp(); }

    /**
      * The get_up method is used to get the user pointer for the
      * change identified.
      */
    user_ty::pointer get_up() { return pid.get_up(); }

    /**
      * The set_user_by_name method is used to set the user name by
      * name.  This is useful for the small set of command which accept
      * a user name on the command line.
      *
      * @param login
      *     The login name of the user.
      */
    void set_user_by_name(nstring &login) { pid.set_user_by_name(login); }

    /**
      * The get_cp method is used to get the change pointer for the
      * change identified.
      */
    change::pointer get_cp();

    /**
      * The get_change_number method is used to obtain the change
      * number for the identified change.
      *
      * \note
      *     There is no need to call magic_zero_decode().
      */
    long get_change_number();

    /**
      * The error_if_no_explicit_change_number is used to emit a
      * fatal_intl error if no change number was specified on the
      * command line.
      */
    void error_if_no_explicit_change_number();

    /**
      * The get_historian method is used to obtain the location of the
      * historical file reconstructor.
      *
      * \note
      *     This function is a failure of the API to conceal this.
      *     Eventually it would be nice if all the users of this could
      *     be refactored to hide it again.
      */
    project_file_roll_forward *get_historian();

    /**
      * The invalidate_meta_data method is used to discard cached
      * information about the change.  This is usually necessary when a
      * sub-command is run and that subcommand would update the change
      * meta-data.
      */
    void invalidate_meta_data();

private:
    /**
      * The pid instance variable is used to remember which project goes
      * with this change.
      */
    project_identifier_subset_branch pid;

    enum { NO_TIME_SET = -1 };

    /**
      * The baseline instance variable is used to remember wether or not
      * the --baseline command line option has been specified.
      */
    bool baseline;

    /**
      * The devdir instance variable is used to remember wether or not
      * the --devdir command line option has been specified.
      */
    bool devdir;

    /**
      * The change_number instance variable is used to remember any
      * --change command line option specified.  This could be implicit.
      */
    long change_number;

    /**
      * The delta_date instance variable is used to remember any
      * --delta-date command line option specified.  This could be
      * implicit.
      */
    long delta_date;

    /**
      * The delta_name instance variable is used to remember any
      * --delta-name command line option specified.
      */
    nstring delta_name;

    /**
      * The delta_number instance variable is used to remember any
      * --delta command line option specified.  This could be implicit.
      */
    long delta_number;

    /**
      * The cp instance variable is used to remember which change we are
      * bound to.
      */
    change::pointer cp;

    /**
      * The historian_p instance variable is used to remember the
      * location of the historical file reconstructor.  Shall only be
      * accessed by the get_historian method.
      */
    project_file_roll_forward *historian_p;

    /**
      * The need_historical_perspective method is used to determine
      * whether or not access to project files should be direct (false)
      * or via the historian (false).
      */
    bool need_historical_perspective() const;

    /**
      * The default constructor.  Do not use.
      */
    change_identifier_subset();

    /**
      * The copy constructor.  Do not use.
      */
    change_identifier_subset(const change_identifier_subset &);

    /**
      * The assignment operator.  Do not use.
      */
    change_identifier_subset &operator=(const change_identifier_subset &);
};

#endif // LIBAEGIS_CHANGE_IDENTIFI_SUB_H
