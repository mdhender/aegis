//
//      aegis - project change supervisor
//      Copyright (C) 1992-2000, 2002-2008, 2011, 2012 Peter Miller
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

#ifndef AEGIS_USER_H
#define AEGIS_USER_H

#include <common/ac/shared_ptr.h>

#include <common/nstring.h>
#include <libaegis/output.h>
#include <libaegis/uconf.fmtgen.h>
#include <libaegis/ustate.fmtgen.h>

class project; // forward
class output_ty; // forward

/**
  * The user_ty class represents a user (usually, the operating system's
  * idea of a user) an information about that user, and her preferences.
  */
class user_ty
{
public:
    /**
      * The pointer typedef is to be used whenevr pointing to a user_ty
      * object.  This allows the pointer type to be changed to a smart
      * pointer in the future.
      */
    typedef aegis_shared_ptr<user_ty> pointer;

    /**
      * The destructor.
      *
      * It is not virtual, thou shalt not derived from this class.
      */
    ~user_ty();

    /**
      * The create class method is used to create new heap instances
      * of users.  The user in this case being the user who executeed
      * this program.
      */
    static user_ty::pointer create();

private:
    /**
      * The default constructor.
      */
    user_ty();

public:
    /**
      * The create class method is used to create new heap instances of
      * users.  The default group is derived from the password file.
      *
      * @param uid
      *     The user ID involved.
      */
    static user_ty::pointer create(int uid);

private:
    /**
      * The constructor.
      *
      * @param uid
      *     The user ID involved.
      */
    user_ty(int uid);

public:
    /**
      * The create class method is used to create a user structure for
      * the aegis/gonzo.cc file.  It has a known uid and gid, and will
      * succeed, even if there are no relvant entries in system tables.
      *
      * @param uid
      *     system user id
      * @param gid
      *     system group id
      */
    static user_ty::pointer create(int uid, int gid);

private:
    /**
      * The constrctor.
      *
      * @param uid
      *     system user id
      * @param gid
      *     system group id
      */
    user_ty(int uid, int gid);

public:
    /**
      * The create class method is used to create a user instance from
      * a login name.
      *
      * The login name is mapped to a uid.  The password file is
      * searched from beginning to end, so the cannonical name is the
      * first found in the password file.  The cannonical name is used
      * even for user structures created with this function.
      *
      * @param login_name
      *     user's login name
      */
    static user_ty::pointer create(const nstring &login_name);

    /**
      * The create_by_directory class method is used to create a user
      * from the ownership of a directory.
      *
      * @param path
      *     The absolute path pf the directory of interest.
      * @returns
      *     pointer to new user_ty instance
      */
    static user_ty::pointer create_by_directory(const nstring &path);

    /**
      * The progress_option_set class method is called by command line
      * processing to indicate that a --progress command line option has
      * been seen.
      *
      * @param usage
      *     The function to call if a fail via a usage message is
      *     required.
      */
    static void progress_option_set(void (*usage)(void));

    /**
      * The progress_option_clear class method is called by command line
      * processing to indicate that a --no-progress command line option
      * has been seen.
      *
      * @param usage
      *     The function to call if a fail via a usage message is
      *     required.
      */
    static void progress_option_clear(void (*usage)(void));

    /**
      * The progress_get method is used to determine whether or not
      * progress messages should be displayed.  It consults the command
      * line settings and also the user's progress attribute.  Being in
      * the background is not considered.  The --verbose option is not
      * considered.  The default is to return false.
      *
      * Example:
      *     attribute =
      *     [
      *         { name = "progress-preference"; value = "true"; }
      *     ];
      */
    bool progress_get();

    /**
      * The name method is used to obtain the login name of this user.
      */
    nstring name() const;

    /**
      * The full_name method is used to obtain the human readable name
      * of this user, as recorded in the password file.
      */
    nstring full_name() const;

    /**
      * The full_name class method is used to obtain the full name of a
      * user, given the login name.
      *
      * @param login
      *     The login name of the user
      * @returns
      *     the full name of the user, or the empty string if it doesn't
      *     exist.
      */
    static nstring full_name(const nstring &login);

    /**
      * The get_uid method is used to obtain the operating system user
      * ID associated with this user.
      */
    int get_uid() const;

    /**
      * The check_uid method is used to verify that this user has an
      * acceptable level of privilege.
      */
    bool check_uid() const;

    /**
      * The get_gid method is used to obtain the operating system group
      * ID associated with this user.
      */
    int get_gid() const;

    /**
      * The check_gid method is used to verify that this user has an
      * acceptable level of privilege.
      */
    bool check_gid() const;

    /**
      * The get_group_name method is used to obtain the name of the
      * operating system group attached to this user.
      */
    nstring get_group_name() const;

    /**
      * The set_gid method is used to set the group ID of this user.
      *
      * @param gid
      *     The group ID to use.
      * @param weak
      *     true if it is OK that the group doesn't exists, false
      *     (default) if it is an error if the group desn't exist.
      */
    void set_gid(int gid, bool weak = false);

    /**
      * The umask_set method is used to set the file creation mode mosk.
      *
      * @param um
      *     The new mask.
      */
    void umask_set(int um);

    /**
      * The umask_get method is used to obtain the new file creation
      * permissions mode mask for this user.
      */
    int umask_get() const;

    /**
      * The get_home method is used to obtain the absolute path of this
      * user's home directory.
      */
    nstring get_home() const;

    /**
      * The get_email_address method is used to read the user's
      * preferences for her email address.  Defaults to full name from
      * /etc/passwd, then login name at hostname (not generally useful).
      */
    nstring get_email_address();

    /**
      * The ustate_write method is used to write any modified ustate
      * file contents back to disk.
      */
    void ustate_write();

    /**
      * The user_own_add function is used to add a change to the user's
      * list of owned changes.
      *
      * @param pp
      *     project of the change
      * @param chanum
      *     the number of the change
      */
    void own_add(project *pp, long chanum);

    /**
      * The user_own_nth function is used to fetch the n'th
      * change owned by a user.
      * The project name is derived from the user structure.
      *
      * @param pp
      *     project of the change
      * @param n
      *     selector
      * @param chanum
      *     reference to where to put number of the change
      * @returns
      *     true on sucess, false if no such n.
      */
    bool own_nth(project *pp, long n, long &chanum);

    /**
      * The user_own_remove function is used to
      * remove a change from the user's owned change list.
      *
      * @param pp
      *     project of the change
      * @param chanum
      *     number of the change.
      *     The change is assumed to be unique.
      */
    void own_remove(project *pp, long chanum);

    /**
      * The default_change method is used to find the default change
      * number.  It is a fatal error if there isn't one.
      *
      * @param pp
      *     The project of interest.
      * @returns
      *     the change number.
      */
    long default_change(project *pp);

    /**
      * The default_project method is used to determine the default
      * project of the specified user.  It is a fatal error if there is
      * no default project name for the user.
      */
    nstring default_project();

    /**
      * The default_development_directory method is used to determine
      * the absolute path of the user's default development directory.
      *
      * If the user has not explicitly set one, and the project does not
      * have one set, the user's home directory will be returned.
      *
      * @param pp
      *     The project to consult for its default development
      *     directory, as wel.
      */
    nstring default_development_directory(project *pp);

    /**
      * The default_project_directory method is used to determine the
      * absolute path for where to place new projects.
      *
      * if the user has not explicitly set one, the user's home
      * directory will be returned.
      */
    nstring default_project_directory();

    /**
      * The become_begin method is used to set the effective uid and
      * gid to be those of this user.  In this way, the code can
      * impersonate a variety of users, while still enforcing strict
      * secure semantics.
      */
    void become_begin();

    /**
      * The become_end method is used to reverse the effects of the
      * become_begin method.
      *
      * These two methods <b>shall always</b> be used in pairs.  This is
      * simplified further if you use the user_ty::become class.
      */
    void become_end();

    /**
      * The user_ty::become class is used to take care of becoming a
      * user, and then ceasing to become that user at the end of the
      * scope.  For example:
      *
      *     {
      *         user_ty::become scoped(up);
      *         ...glue...
      *         if (something)
      *             return;
      *         ...execute...
      *     }
      *
      * No matter how the flow of execution leaves the scope (break,
      * continue, return, exceptions) the become_end will always occur,
      * because the destructor is automatically called by the compiler.
      *
      * This is an example of the Resoure Acquisition Is Initialisation
      * (RAII) design pattern.
      */
    class become
    {
    public:
        /**
          * The constructor.  Start impersonating the user.
          */
        become(user_ty::pointer a_up) :
            up(a_up)
        {
            up->become_begin();
        }

        /**
         * The destructor.  Stop impersonating the user.
         */
        ~become()
        {
            up->become_end();
        }

    private:
        /**
          * The up instance variable is used to remember which user is
          * being impersonated.
          */
        user_ty::pointer up;

        /**
          * The copy constructor.  Do not use.
          */
        become(const become &);

        /**
          * The assignment operator.  Do not use.
          */
        become &operator=(const become &);
    };

    /**
      * The delete_file_query method is used to determine whether a file
      * should be deleted or not.
      *
      * @param filename
      *     The name of the file to be deleted
      * @param isdir
      *     whether the file is a directory (true) or a regular file (false).
      * @param default_preference
      *     If the user gave no preference on the command line, use this
      *     instead.  1 -> true, 0 -> false, -1 -> look in .aegisrc
      * @returns
      *     true if the file should be deleted, or false if it should not.
      */
    bool delete_file_query(const nstring &filename, bool isdir,
        int default_preference);

    /**
      * The delete_file_argument class method is used to process --keep
      * and --no-keep command line options.
      *
      * @param usage
      *     Exit via this function on error.
      */
    static void delete_file_argument(void (*usage)(void));

    /**
      * The ustate_lock_prepare method is used to notify the lock
      * manager that a ustate lock will be required.
      */
    void ustate_lock_prepare();

    /**
      * The lock_wait_argument class method is used to process --wait
      * and --no-wait command line arguments.
      *
      * @param usage
      *     Exit via this function on error.
      */
    static void lock_wait_argument(void(*usage)(void));

    /**
      * The lock_wait method may be used if this user wants to wait for
      * locks or not.
      */
    bool lock_wait();

    /**
      * The editor_command method is used to read the user's preferences
      * for editor to use with aegis.  Defaults to EDITOR environment
      * variable, or else "ed".
      */
    nstring editor_command();

    /**
      * The visual_command method is used to read the user's preferences
      * for a visual editor to use with aegis.  Defaults to VISUAL
      * environment variable, or else the EDITOR environment variable,
      * or else "vi".
      */
    nstring visual_command();

    /**
      * The diff_preference method is ised to obtain this user's aed
      * preference.
      *
      * @returns
      *     The choices are always merge, never merge, and automatic
      *     (default).
      */
    uconf_diff_preference_ty diff_preference();

    /**
      * The pager_preference method is used to obtain this user's pager
      * preference.
      *
      * @returns
      *     true if should use pager, false if should not use pager.
      */
    bool pager_preference();

    /**
      * The pager_command method is used to read the user's preferences
      * for pager to use with aegis.  Defaults to PAGER environment
      * variable.  If $PAGER is not set then "more" is used.
      */
    nstring pager_command();

    /**
      * The persevere_preference method is used to determine whether or not
      * aet (and friends) should persevere in the face of errors.
      *
      * @param dflt
      *     The default to use, should the not have set a preference.
      */
    bool persevere_preference(bool dflt);

    /**
      * The persevere_argument class method is used to process
      * --presevere and -no-persevere command line arguments.
      *
      * @param usage
      *     Exit via this function on error.
      */
    static void persevere_argument(void(*usage)(void));

    /**
      * The log_file_preference method may be used to obtain this user's
      * log file preference.
      *
      * @param dflt
      *     If the user has not specified one, this argument is the
      *     default to be returned (varies with command).
      * @returns
      *     the choices are: none, replace, append
      */
    uconf_log_file_preference_ty log_file_preference(
        uconf_log_file_preference_ty dflt);

    /**
      * The relative_filename_preference_argument class method is used
      * to process --base-relative and --not-base-relative command line
      * options.
      *
      * @param usage
      *     Exit via this function on error.
      */
    static void relative_filename_preference_argument(void(*usage)(void));

    /**
      * The relative_filename_preference method may be used to obtain
      * this user's preference for whethger or not relative file names
      * on the command line are relative to the development diorectory
      * root, or relative to the current directory.
      *
      * @param dflt
      *     The default to use if the user has not selected on (varies
      *     with command).
      * @returns
      *     the choices are: base, current
      */
    uconf_relative_filename_preference_ty relative_filename_preference(
        uconf_relative_filename_preference_ty dflt);

    /**
      * The symlink_pref_argument is used to process the
      * --symbolic-links and --no-symbolic-links command line options.
      *
      * @param usage
      *     Exit via this function on error.
      */
    static void symlink_pref_argument(void(*usage)(void));

    /**
      * The symlink_pref method may be used to obtain this user's
      * symbolic link maintenance preference.  Some development
      * directory styles do not have to be maintained.
      *
      * If your development directory style uses hard links or copies,
      * this method also controls them.
      *
      * @param proj_files_changed
      *     true if any project files have changed recently
      * @returns
      *     true to update the links, false to not update the links
      */
    bool symlink_pref(bool proj_files_changed);

    /**
      * The symlink_pref_argument is used to process the --whiteout and
      * --no-whiteout command line options.
      *
      * @param usage
      *     Exit via this function on error.
      */
    static void whiteout_argument(void (*usage)(void));

    /**
      * The whiteout mwthod may be used to determine this user's
      * whiteout file preference.
      *
      * @param dflt
      *     The default the comman wants (true (0) or false (1))
      *     or -1 if the user's .aegisrc file is to be consulted.
      * @returns
      *     true if whiteout files are to be used, false if no whiteout
      *     files are to be generated.
      */
    bool whiteout(int dflt = -1);

    /**
      * The uconf_write_xml method may be used to write this user's
      * uconf data out as XML.
      *
      * @param op
      *     Where to write the XML.
      */
    void uconf_write_xml(output::pointer op);

    /**
      * The uconf_get method is used to get the uconf data corresponding
      * to the specified user.  It is cached.  This should be used
      * sparingly, if at all.  It is preferable to use one of the above
      * methods if at all possible.
      *
      * @returns
      *     pointer to uconf structure in dynamic memory
      * @note
      *     this method will eventually be PRIVATE
      */
    uconf_ty *uconf_get();

private:
    /**
      * The login_name instance variable is used to remember the login
      * name of this user.
      */
    nstring login_name;

    /**
      * The login_full_name instance variable is used to remember the
      * full name of this user, as recorded in the password entry.
      */
    nstring login_full_name;

    /**
      * The home instance variable is used to remember the absolute path
      * of this user's home directory.
      */
    nstring home;

    /**
      * The group_name instance variable is used to remember the name
      * of the operating system group attached to this user.
      */
    nstring group_name;

    /**
      * The user_id instance variable is used to remeber the operating
      * system user ID associated with this user.
      */
    int user_id;

    /**
      * The group_id instance variable is used to remeber the operating
      * system group ID associated with this user.
      */
    int group_id;

    /**
      * The umask instance variable is used to remember this user's
      * preferred new file mode creation mask.
      */
    int umask;

    /**
      * The ustate_path instance variable is used to remember where the
      * ustate file is that holds this user's index of owned project.
      *
      * In some ways this is not ideal, since the path depends on which
      * gonzo holds the project record.  There is more than one.
      */
    nstring ustate_path;

    /**
      * The ustate_data instance variable is used to remember the
      * location of the cached ustate data for this user.
      *
      * In some ways this is not ideal, since the data depends on which
      * gonzo holds the project record.  There is more than one.
      */
    ustate_ty *ustate_data;

    /**
      * The ustate_is_new instance variable is used to remember whether
      * or not a new ustate file is to be created.
      */
    bool ustate_is_new;

    /**
      * The ustate_modified instance variable is used to remember
      * whether or not the cahced ustate data has been modified, and
      * needs to be written out.
      */
    bool ustate_modified;

    /**
      * The user_ustate_get function is used to fetch the "ustate" file
      * for this user, caching for future reference.
      *
      * @param pp
      *     The project of interest
      */
    ustate_ty *ustate_get(project *pp);

    /**
      * The uconf_path instance variable is used to remember the
      * absolute path of this user's .aegisrc file.
      */
    nstring uconf_path;

    /**
      * The uconf_data instance variable is used to remember the base
      * address of the read-in .aegisrc file data.  It is cached for
      * efficiency.
      */
    uconf_ty *uconf_data;

    /**
      * The lock_magic instance variable is used to remember the last
      * time the locks changed, so that uconf and ustate data can be
      * invalidated, if necessary.  See the lock_sync method.
      */
    long lock_magic;

    /**
      * The lock_sync method is used to flush any out-of-date data
      * caching associated with the user structure.
      */
    void lock_sync();

    /**
      * The pool_nusers class variable is used to remember how many
      * user_ty instaces currently exist.
      */
    static size_t pool_nusers;

    /**
      * The pool_nusers_max class variable is used to remember the size
      * allocated for the pool_user array.
      *
      * assert(pool_nusers <= pool_nusers_max);
      */
    static size_t pool_nusers_max;

    /**
      * The pool_user class variable is used to remember the base
      * address of a dynamically allocated array of pointers to user
      * instances.
      *
      * assert(!pool_user == !pool_nusers_max);
      */
    static pointer *pool_user;

    /**
      * The pool_find class method is used to look into the pool of
      * user_ty instances to determine wether or not the given UID
      * corresponds to an existing user_ty instance.
      *
      * This is an O(n) search.  It is expected that there will be at
      * most 4, so the slow search isn't a problem - a faster search
      * would have a larger k - making it moot.
      *
      * @param uid
      *     The UID to search for
      * @returns
      *     pointer if found, NULL if not found
      */
    static user_ty::pointer pool_find(int uid);

    /**
      * The pool_add class method is used to add a new user_ty instance
      * to the pool.
      */
    static void pool_add(user_ty::pointer up);

    /**
      * The waiting_for_lock method is used to print a message to inform
      * the user of the progress acquiring the lock.
      */
    void waiting_for_lock();

    /**
      * The waiting_for_lock class method is used to call the
      * waiting_for_lock method.
      */
    static void waiting_for_lock(void *);
};


// use user_ty::create(int) instead
user_ty::pointer user_numeric(int uid) DEPRECATED;

// use user_ty::create(int, int) instead
user_ty::pointer user_numeric2(int uid, int gid) DEPRECATED;

// use user_ty::create(nstring) instead
user_ty::pointer user_symbolic(string_ty *login_name) DEPRECATED;

// use user_ty::create() instead
user_ty::pointer user_executing(void) DEPRECATED;

// use nothing at all
void user_free(user_ty::pointer) DEPRECATED;

// just assign it
user_ty::pointer user_copy(user_ty::pointer) DEPRECATED;

// use user_ty::name() instead
string_ty *user_name(user_ty::pointer up) DEPRECATED;

// use user_ty::full_name() instead
string_ty *user_name2(user_ty::pointer up) DEPRECATED;

// use user_ty::get_uid() instead
int user_id(user_ty::pointer) DEPRECATED;

// use user_ty::get_gid() instead
int user_gid(user_ty::pointer) DEPRECATED;

// use user_ty::umask_get() instead
int user_umask(user_ty::pointer) DEPRECATED;

// user user_ty::ustate_lock_prepare() instead
void user_ustate_lock_prepare(user_ty::pointer) DEPRECATED;

// use user_ty::get_group_name() instead
string_ty *user_group(user_ty::pointer) DEPRECATED;

// use user_ty::get_home() instead
string_ty *user_home(user_ty::pointer up) DEPRECATED;

// user user_ty::get_email_address() instead
string_ty *user_email_address(user_ty::pointer up) DEPRECATED;

// use user_ty::editor_command() instead
string_ty *user_editor_command(user_ty::pointer) DEPRECATED;

// user user_ty::visual_command() instead
string_ty *user_visual_command(user_ty::pointer) DEPRECATED;

// see user_ty::pager_command instead
string_ty *user_pager_command(user_ty::pointer) DEPRECATED;

// use user_ty::full_name(const nstring &) instead
string_ty *user_full_name(string_ty *) DEPRECATED;

// use user_ty::ustate_write() instead
void user_ustate_write(user_ty::pointer) DEPRECATED;

// use user_ty::own_add instead
void user_own_add(user_ty::pointer up, project *pp, long chanum) DEPRECATED;

// use user_ty::own_nth instead
int user_own_nth(user_ty::pointer up, project *oo, long n, long *chanum_p)
    DEPRECATED;

// use user_ty::own_remove instead
void user_own_remove(user_ty::pointer up, project *pp, long chanum)
    DEPRECATED;

// use user_ty::default_change(project *pp) instead
long user_default_change(user_ty::pointer up, project *pp) DEPRECATED;

// user user_ty::default_project() instead
string_ty *user_default_project_by_user(user_ty::pointer) DEPRECATED;

// user user_ty::default_project() instead
string_ty *user_default_project(void) DEPRECATED;

// use user_ty::default_development_directory(project *pp) instead
string_ty *user_default_development_directory(user_ty::pointer up,
    project *pp) DEPRECATED;

// user user_ty::default_project_directory() instead
string_ty *user_default_project_directory(user_ty::pointer) DEPRECATED;

// use user_ty::become_begin() instead
void user_become(user_ty::pointer) DEPRECATED;

// use user_ty::become_end() instead
void user_become_undo(void) DEPRECATED;

// use user_ty::delete_file_query instead
bool user_delete_file_query(user_ty::pointer up, string_ty *filename,
    bool isdir, int default_preference) DEPRECATED;

// use user_ty::delete_file_argument instead
void user_delete_file_argument(void (*)(void)) DEPRECATED;

// use user_ty::diff_preference() instead
int user_diff_preference(user_ty::pointer) DEPRECATED;

// use user_ty::pager_preference() instead
int user_pager_preference(user_ty::pointer) DEPRECATED;

// use user_ty::persevere_preference() instead
int user_persevere_preference(user_ty::pointer, int) DEPRECATED;

// use user_ty::persevere_argument instead
void user_persevere_argument(void(*)(void)) DEPRECATED;

// use user_ty::log_file_preference instead
uconf_log_file_preference_ty user_log_file_preference(user_ty::pointer,
    uconf_log_file_preference_ty) DEPRECATED;

// user user_ty::lock_wait_argument instead
void user_lock_wait_argument(void(*usage)(void)) DEPRECATED;

// user user_ty::lock_wait instead
int user_lock_wait(user_ty::pointer up) DEPRECATED;

// use user_ty::whiteout_argument instead
void user_whiteout_argument(void(*)(void)) DEPRECATED;

// use user_ty::whiteout instead
int user_whiteout(user_ty::pointer up, int dflt = -1) DEPRECATED;

// use user_ty::symlink_pref_argument instead
void user_symlink_pref_argument(void(*)(void)) DEPRECATED;

// use user_ty::symlink_pref instead
int user_symlink_pref(user_ty::pointer, int) DEPRECATED;

// use user_ty::relative_filename_preference_argument instead
void user_relative_filename_preference_argument(void(*)(void)) DEPRECATED;

// use user_ty::relative_filename_preference instead
uconf_relative_filename_preference_ty user_relative_filename_preference(
    user_ty::pointer, uconf_relative_filename_preference_ty) DEPRECATED;

// use user_ty::uconf_write_xml instead
void user_uconf_write_xml(user_ty::pointer, output::pointer) DEPRECATED;

// use user_ty::uconf_get() instead
uconf_ty *user_uconf_get(user_ty::pointer) DEPRECATED;

#endif // AEGIS_USER_H
// vim: set ts=8 sw=4 et :
