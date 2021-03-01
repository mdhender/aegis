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

#ifndef LIBAEGIS_CHANGE_IDENTIFIER_H
#define LIBAEGIS_CHANGE_IDENTIFIER_H

#include <libaegis/change/identifi_sub.h>
#include <libaegis/file_revision.h>
#include <libaegis/project/identifi_sub/plain.h>
#include <libaegis/project/identifi_sub/branch.h>

/**
  * The change_identifier class is used to represent a change and its
  * context, as identified by all the different command line options used
  * to specify a change set.
  *
  * This class is a compound of the change_identifier_subset class and
  * the project_identifier_subset class, because they are most commonly
  * used together.  The various methods are proxied to the appropriate
  * instance variables.
  */
class change_identifier
{
public:
    /**
      * The destructor.
      * It isn't virtual; thou shalt not derive from this class.
      */
    ~change_identifier();

    /**
      * The default constructor.
      */
    change_identifier();

    /**
      * The set method is used to determine if this change ID has been
      * set yet (via any of several command line options).
      */
    bool set() const { return change_id.set(); }

    /**
      * The project_set method is used to determine if the project name
      * has been set yet (via any of several command line options).
      */
    bool project_set() const { return project_id.set(); }

    /**
      * The command_line_parse method is used to parse command line
      * options (via the arglex() function) to set the change ID.  The
      * current token is expected to be meaningful for identifying a
      * change.  The position will be advanced past all relevant tokens.
      *
      * It understands all of the following command line options:
      *     -BaseLine
      *     [ -Change] number
      *     -BRanch number
      *     -TRunk
      *     -GrandParent
      *     -DELta number
      *     -DELta name
      *     -DELta_Date when
      *     -Developmen_Directory
      *     [ -Project ] string
      *
      * \note
      *     There is no need to pass all of these command line options to
      *     this function for processing.  Only pass those options which
      *     make sense.  If is often the case that there are actually
      *     *two* changes being identified, and they will split the
      *     arguments between them.
      */
    void
    command_line_parse(void (*usage)(void))
    {
	change_id.command_line_parse(usage);
    }

    /**
      * The command_line_parse_rest method is used to parse the rest of
      * the command line.  It will call arglex(), #command_line_parse,
      * #command_line_check, and generic_argument() to perform this
      * processing.
      */
    void command_line_parse_rest(void (*usage)(void));

    bool get_devdir() { return change_id.get_devdir(); }

    /**
      * The command_line_check method is used to verify that sensible
      * command line options have been specified, once the parse has
      * completed.
      */
    void
    command_line_check(void (*usage)(void))
    {
	change_id.command_line_check(usage);
    }

    /**
      * The set_baseline method is used to specify that the baseline is
      * the change being identified.
      */
    void set_baseline() { change_id.set_baseline(); }

    /**
      * The get_baseline method is used to determine whether the
      * --baseline option has been specified.
      */
    bool get_baseline() { return change_id.get_baseline(); }

    /**
      * The get_file_revision is used to determine the path to the given
      * file at the time specified by the change ID.  It must be called
      * <i>after</i> the set_change method has been called.
      *
      * @param filename
      *     the name of the file of interest
      * @param bad_state
      *     what to do if the change is not in an appropriate state
      */
    file_revision
    get_file_revision(const nstring &filename, change_functor &bad_state)
    {
	return change_id.get_file_revision(filename, bad_state);
    }

    /**
      * The get_file_revision is used to determine the path to the given
      * file at the time specified by the change ID.  It must be called
      * <i>after</i> the set_change method has been called.
      *
      * @param src
      *     The meta data identifying the file (revison data, if set,
      *     will be ignored)
      * @param bad_state
      *     what to do if the change is not in an appropriate state
      */
    file_revision
    get_file_revision(fstate_src_ty *src, change_functor &bad_state)
    {
	return change_id.get_file_revision(src, bad_state);
    }

    /**
      * The get_project_file_names method is used to obtain the list of
      * project file names, taking into account and --delta* options.
      */
    void
    get_project_file_names(nstring_list &results)
    {
	change_id.get_project_file_names(results);
    }

    /**
      * The get_project_file method is used to obtain the details about
      * a named file, taking any --delta* options into account.
      *
      * \param file_name
      *     The name of the file of interest.
      * \returns
      *     pointer to file details, or NULL of the file does not exist.
      */
    fstate_src_ty *
    get_project_file(const nstring &file_name)
    {
	return change_id.get_project_file(file_name);
    }

    /**
      * The get_historian method is used to obtain the location of the
      * historical file records reconstruction.
      *
      * \note
      *     This function is a failure of the API to conceal this.
      *     Eventually it would be nice if all the users of this could
      *     be refactored to hide it again.
      */
    project_file_roll_forward *
    get_historian()
    {
	return change_id.get_historian();
    }

    /**
      * The get_change_version_string method is used to get the version
      * string for the change.
      */
    nstring
    get_change_version_string()
    {
	return change_id.get_change_version_string();
    }

    /**
      * The get_pp method is used to get the project pointer for the
      * change identified.
      */
    project_ty *
    get_pp()
    {
	return branch_id.get_pp();
    }

    /**
      * The get_up method is used to get the user pointer for the
      * change identified.
      */
    user_ty::pointer
    get_up()
    {
	return branch_id.get_up();
    }

    /**
      * The set_user_by_name method is used to set the user name by
      * name.  This is useful for the small set of command which accept
      * a user name on the command line.
      *
      * @param login
      *     The login name of the user.
      */
    void set_user_by_name(nstring &login) { branch_id.set_user_by_name(login); }

    /**
      * The get_cp method is used to get the change pointer for the
      * change identified.
      */
    change::pointer
    get_cp()
    {
	return change_id.get_cp();
    }

    /**
      * The get_change_number method is used to obtain the change
      * number for the identified change.
      *
      * \note
      *     There is no need to call magic_zero_decode().
      */
    long
    get_change_number()
    {
	return change_id.get_change_number();
    }

    /**
      * The error_if_no_explicit_change_number is used to emit a
      * fatal_intl error if no change number was specified on the
      * command line.
      */
    void
    error_if_no_explicit_change_number()
    {
	change_id.error_if_no_explicit_change_number();
    }

    /**
      * The invalidate_change_meta_data method is used to discard cached
      * information about the change.  This is usually necessary when a
      * sub-command is run and that subcommand would update the change
      * meta-data.
      */
    void invalidate_change_meta_data() { change_id.invalidate_meta_data(); }

private:
    /**
      * The branch_id instance variable is used to remember the project subset
      * of identifying a change.
      */
    project_identifier_subset_plain project_id;

    /**
      * The branch_id instance variable is used to remember the branch subset
      * of identifying a change.
      */
    project_identifier_subset_branch branch_id;

    /**
      * The change_id instance variable is used to remember the change subset
      * of identifying a change.
      */
    change_identifier_subset change_id;

    /**
      * The copy constructor.  Do not use.
      */
    change_identifier(const change_identifier &);

    /**
      * The assignment operator.  Do not use.
      */
    change_identifier &operator=(const change_identifier &);
};

#endif // LIBAEGIS_CHANGE_IDENTIFIER_H
