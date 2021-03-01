//
//	aegis - project change supervisor
//	Copyright (C) 1992-1999, 2001-2008 Peter Miller
//      Copyright (C) 2007 Walter Franzini
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

#ifndef PROJECT_H
#define PROJECT_H

#include <common/ac/time.h>

#include <libaegis/pattr.h>
#include <libaegis/pstate.h>
#include <libaegis/view_path.h>
#include <libaegis/change.h>

class string_list_ty; // forward
class sub_context_ty; // forward
class fstate_src_ty; // forward
class itab_ty; // forward

/**
  * The project_ty class is used to remember the state of a project.
  *
  * Eventually, it will be discouraged to use this class directly.  The
  * preferred method will be to access it via a project class (see
  * below) smart pointer.  This will be able to guarantee the reference
  * counting semantics.
  *
  * This class is presently in transition from being a C struct.  One by
  * one the access functions will be turned into access methids, and the
  * instance variables will become private.
  */
class project_ty
{
public:
    /**
      * The destructor.
      *
      * It is not virtual; thou shalt not derive fron this class.
      *
      * @note
      *     Eventually this method will be private, to ensure the
      *     reference counting always works.  The correct method is to
      *     call the project_free function.
      */
    ~project_ty();

    /**
      * The constructor.
      *
      * @param name
      *     The name of the project.
      */
    project_ty(string_ty *name);

    // NOTE: methods are sorted alphabetically.  To have methods
    // grouped, make sure they change their suffix rather than their
    // prefix.  E.g. thingumy_get and thingumy_set rather than
    // get_thingumy and set_thingumy.

    /**
      * The attribute_get method is used to obtain the value of an
      * attribute of this project.
      *
      * The project attributes are kept in the <tt>project_specific</tt>
      * field of the project config file (<tt>aegis.conf</tt>) which is
      * a project source file.  Only use this method when you want the
      * <b>baseline</b> attributes, use change::pconf_attributes_find
      * within a change set.
      *
      * @param name
      *     The name of the attribute.
      *     Attribute names are not case sensitive.
      * @returns
      *     the string value of the attribute, or the empty string
      *     if not found.
      */
    nstring attribute_get(const nstring &name);

    /**
      * The attribute_get_boolean method is used to obtain the value of
      * an attribute of this project, as a true/false value.
      *
      * The project attributes are kept in the <tt>project_specific</tt>
      * field of the project config file (<tt>aegis.conf</tt>)
      * which is a project source file.  Only use this method
      * when you want the <b>baseline</b> attributes, use
      * change::pconf_attributes_find_boolean within a change set.
      *
      * @param name
      *     The name of the attribute.
      *     Attribute names are not case sensitive.
      * @returns
      *     the boolean value of the attribute, or false if the
      *     attribute is not found or is not interpretable as a
      *     boolean.
      */
    bool attribute_get_boolean(const nstring &name);

    /**
      * The baseline_path_get method is used to obtain the absolure path
      * of the project's (branch's) baseline.
      *
      * @param resolve
      *     Whether or not to resolve all of the symlinks in the path.
      */
    string_ty *baseline_path_get(bool resolve = false);

    /**
      * The bind_branch method is used to bind a branch of this project
      * into a new project object.
      *
      * @param bp
      *     The change corresponding to the project of interest.
      */
    project_ty *bind_branch(change::pointer bp);

    /**
      * The bind_existing method is used to bind a newly created project
      * object to an existing project.
      *
      * @note
      *     This method does not return if there is an error.  A fatal
      *     error message will be produced in this case.
      */
    void bind_existing();

    /**
      * The bind_existing_errok method is used to bind a newly created
      * project object to an existing project.
      *
      * @returns
      *     bool; true if bound successfully, false if not
      */
    bool bind_existing_errok();

    /**
      * The bind_keep method is used to bind a new project to an
      * existing directory, as needed by the "aenpr -keep" option.
      *
      * @param path
      *     The directory containing the project.
      */
    void bind_keep(const nstring &path);

    /**
      * The bind_new method is used to bind a new project object to a
      * new name.  This will create all of the necessary data structures
      * and (eventually) files, once the project directory has been set.
      */
    void bind_new();

    /**
      * The change_get method is used to obtain a pointer to the change
      * object representing this project branch.
      *
      * @returns
      *     pointer to change object;
      *     DO NOT change_free() or delete it.
      */
    change::pointer change_get();

    /**
      * The change_get_raw method is used to obtain a pointer to the change
      * object representing this project branch or NULL if no such object
      * is in the project.
      *
      * @returns
      *     pointer to change object;
      *     DO NOT change_free() or delete it.
      */
    change::pointer change_get_raw() { return pcp; }

    /**
      * The change_path_get method is used to obtain the absolute path
      * of the meta-data of a specific change set.
      *
      * @param change_number
      *     The number of the change for which the path is desired.
      * @returns
      *     a string; use str_free when you are done with.
      */
    string_ty *change_path_get(long change_number);

    void
    change_reset()
    {
        change::pointer tmp = pcp;
        pcp = 0;
        change_free(tmp);
    }

    /**
      * The changes_path_get method is used to obtain the absolute path
      * of the directory containing change set meta-data.
      *
      * @returns
      *     a string; do not str_free or delete it, because it is cached.
      */
    string_ty *changes_path_get();

public: // during transition, then private
    /**
      * The convert_to_new_format method is used to convert Aegis 2.3
      * project meta-data into Aegis 3.0 (and later) project meta-data.
      */
    void convert_to_new_format();

public:
    /**
      * The copy_the_owner method is used to copy the owner from another
      * project.  It is only ever used by aenrls, which sort-of clones
      * another project.  Don't use this method.
      *
      * Note: this is only a transient requirement for new projects.
      * Existing projects take their uid and gid from the Unix uid and
      * gid of the project directory.
      */
    void copy_the_owner(project_ty *pp);

    /**
      * the copyright_years_slurp method is used to determine the range
      * of copyright years covered by this project and all its ancrestor
      * projects back to the trunk.
      *
      * @param a
      *     The array to store the results in
      * @param amax
      *     The maximum number of distinct years which can be stored in
      *     the array (see SIZEOF macro).
      * @alen_p
      *     This is a pointer to the array length used to date.
      */
    void copyright_years_slurp(int *a, int amax, int *alen_p);

    /**
      * The file_find_by_uuid method is used to find the state
      * information of a file within the project, given the file's UUID.
      * It will search the immediate branch, and then any ancestor
      * branches until the file is found.
      *
      * \param uuid
      *     The UUID of the file to search for.
      * \param vp
      *     If this is true, apply viewpath rules to the file (i.e. if
      *     it is removed, return a null pointer) if false return first
      *     instance found.
      */
    fstate_src_ty *file_find_by_uuid(string_ty *uuid, view_path_ty vp);

    /**
      * The file_find_fuzzy method is used to find the state information
      * for a project file when the project_file_find function fails.
      * It uses fuzzy string matching, which is significantly slower
      * than exact searching, but can provide very useful error messages
      * for users.
      *
      * \param pp
      *     The project to search.
      * \param filename
      *     The base-relative name of the file to search for.
      * \param as_view_path
      *     If this is true, apply viewpath rules to the file (i.e. if
      *     it is removed, return a null pointer) if false return first
      *     instance found.
      */
    fstate_src_ty *file_find_fuzzy(string_ty *filename,
	view_path_ty as_view_path);

private:
    /**
      * The file_list_get method is used to obtain a list of file names.
      * The lists are calculated on demand and cached.
      *
      * \param as_view_path
      *     The view path style to use when calculating the list of
      *     project files.
      * \returns
      *     Pointer to a string list, do not delete it, it is cached.
      */
    struct string_list_ty *file_list_get(view_path_ty as_view_path);

public:
    /**
      * The file_list_invalidate method is used to clear the cahced
      * project file information when it becomes stale.
      */
    void file_list_invalidate();

    /**
      * The file_new method is used to create a new project file by
      * name, and add it to the project file manifest.  No validation is
      * done, the called must guarantee that the file name is unique.
      *
      * \param file_name
      *     The base-relative name of the file to search for.
      */
    fstate_src_ty *file_new(string_ty *file_name);

    /**
      * The file_new method is used to create a new project file from
      * the meta data of an existing (usually change set) file, and add
      * it to the project file manifest.  No validation is done, the
      * caller must guarantee that the file name and UUID are unique.
      *
      * \param meta
      *     The meta data, including the name and UUID, of the file to
      *     be created.
      */
    fstate_src_ty *file_new(fstate_src_ty *meta);

    /**
      * The file_nth method is used to get the 'n'th file from the list
      * of project files.
      *
      * \param n
      *     The file number to obtain (zero based).
      * \param as_view_path
      *     The style of view path to use when calculating the list.
      * \returns
      *     pointer to file mete-data, or NULL if beyond end of list
      */
    fstate_src_ty *file_nth(size_t n, view_path_ty as_view_path);

    /**
      * The find_branch method is used to locate the branch with the
      * given number in this project.
      *
      * @param number
      *     The branch number to locate.
      * @returns
      *     Pointer to valid project object.
      */
    project_ty *find_branch(const char *number);

private:
    /**
      * The get_the_owner method is used to determine the Unix pid and
      * gid for the project.
      */
    void get_the_owner();

public:
    /**
      * The get_user method may be used to obtain a pointer to the user
      * (and group) ownership date for this project.
      */
    user_ty::pointer get_user() const;

    /**
      * The gid_get method is used to obtain the Unix group id of the
      * project owner.
      */
    int gid_get();

    /**
      * The change_completion_timestamp is used to
      * determine the completion timestamp of a change.
      *
      * @param change_number
      *    the number of the change.
      *
      * @returns
      *    the completion timestamp of the change.
      *
      * @note
      *    this method caches its results for speed.
      */
    time_t change_completion_timestamp(long change_number);

    /**
      * The history_path_get method is used to determine the top-level
      * directory of the tree which is used to hold the project's
      * history files.
      *
      * @returns
      *     a pointer to a string.  Do NOT free this string when you are
      *     done with it, because it is cached.
      */
    string_ty *history_path_get();

    /**
      * The home_path_get method is used to get the "home" directory
      * of a project; the directory which contains trunk's delta
      * directories and the baseline, and the "info" meta-data
      * directory.
      *
      * @returns
      *     a string containing the absolute path.
      *     DO NOT str_free or delete it.
      *
      * @note
      *     It is a bug to call this method for anything other than a
      *     trunk project.  Calling if for a branch will result in an
      *     assert failure.
      */
    string_ty *home_path_get();

    /**
      * The home_path_set method is used when creating a new project to
      * remeber the directory the project beaselines and meta-data are
      * stored within.
      *
      * @param dir
      *     The absolute path of the directory in which to keep the
      *     project.  To cope with automounters, directories are stored
      *     as given, or are derived from the home directory in the
      *     passwd file.  Within Aegis, pathnames have their symbolic
      *     links resolved, and any comparison of paths is done on this
      *     "system idea" of the pathname.
      */
    void home_path_set(string_ty *dir);
    void home_path_set(const nstring &dir);

    /**
      * The info_path_get method is used to obtain the absolute path of
      * the meta-data file for this project (branch).
      *
      * @returns
      *     a string; do not str_free or delete it, because it is cached.
      * @note
      *     it is a bug to call this for anything but a top-level
      *     (trunk) project.
      */
    string_ty *info_path_get();

    /**
      * The is_a_trunk method is used to determine whether a project
      * is a trunk branch (is the deepest ancestor) or a normal nested
      * branch (has at least one ancestor branch).
      *
      * @returns
      *     bool; true if is a trunk, false if is a branch
      */
    bool is_a_trunk() const { return (parent == 0); }

    /**
      * The list_inner method is used to append the branch names to the
      * result list.  This is one step of building a complete list of
      * projects.
      *
      * @param result
      *     Append all project branch names to this list.
      */
    void list_inner(string_list_ty &result);

    /**
      * The lock_prepare_everything method is used to take a resd-only
      * lock of everything in the project: pstate, baseline, changes,
      * and recurse into all active branches.
      */
    void lock_prepare_everything();

private:
    /**
      * The lock_sync method is used to invalidate the project state
      * data if the lock has been released and taken again.
      *
      * @note
      *     it is a bug to call this for anything but a top-level
      *     (trunk) project.
      */
    void lock_sync();

public:
    /**
      * The name_get method is used to get the name of the project.
      *
      * @returns
      *     a string; the name of the project.
      *     DO NOT str_free it.
      */
    string_ty *name_get() const;

    /**
      * The parent_branch_number_get method is used to get the branch
      * (change) number of this branch in the parent branch.
      *
      * @note
      *     This method is not meaningful for trunk projects (branches).
      */
    long parent_branch_number_get() const { return parent_bn; }

    /**
      * The parent_get method is used to obtain the parent brach of this
      * project.
      *
      * @returns
      *     pointer to project; do not delete or project_free
      */
    project_ty *parent_get() { return (parent ? parent : this); }

    /**
      * The pstate_get method is used to obtain the project state data.
      *
      * @returns
      *     pointer to pstate_ty data; do not delete, it is cached.
      * @note
      *     it is a bug to call this for anything but a top-level
      *     (trunk) project.
      */
    pstate_ty *pstate_get();

    /**
      * The pstate_lock_prepare method is used to prepare to take a
      * project pstate lock for this project, prior to lock_take() being
      * called.
      */
    void pstate_lock_prepare();

    /**
      * The pstate_path_get method is used to obtain the absolute path
      * of the pstate (project state) file.
      *
      * @returns
      *     a string; do not str_free or delete it, because it is cached.
      * @note
      *     it is a bug to call this for anything but a top-level
      *     (trunk) project.
      */
    string_ty *pstate_path_get();

    /**
      * The pstate_write method is used to write out the project state
      * data into the meta-data directory tree.
      */
    void pstate_write();

    /**
      * The trunk_get method is used to find the trunk project of this
      * project, then top-level branch of this project.
      */
    project_ty *trunk_get();

    /**
      * The uid_get method is used to obtain the Unix user id of the
      * project owner.
      */
    int uid_get();

    /**
      * The umask_get method is used to obtain the file creation mode
      * mask for this project.
      */
    int umask_get();

public: // during transition
    long            reference_count;

private:
    /**
      * The name instance variable is used to remember the name of this
      * project.
      */
    string_ty *name;

    /**
      * The home_path instance variable is used to remember the "home"
      * directory of a project; the directory which contains trunk's
      * delta directories and the baseline, and the "info" meta-data
      * directory.
      *
      * @note
      *     Never access this instance variable directly, always go via
      *     home_path_get() in case it hasn't been calculated yet.
      */
    string_ty *home_path;

    /**
      * The baseline_path_unresolved instance variable is used to
      * remember the absolute path of this project's (branch's) baseline
      * directory.  It has not had is symbolic links resolved.  All
      * access is via the baseline_path_get method, which takes care of
      * calculating it on demand.
      */
    string_ty *baseline_path_unresolved;

    /**
      * The baseline_path_unresolved instance variable is used to
      * remember the absolute path of this project's (branch's) baseline
      * directory.  It has not had is symbolic links resolved.  All
      * access is via the baseline_path_get method, which takes care of
      * calculating it on demand.
      */
    string_ty *baseline_path;

    /**
      * The change2time_stp instance variable is used to remember for
      * each change the completion timestamp.  All access are via the
      * change_completion_timestamp_maybe_cached method, which take
      * care of calculating it on demand.
      */
    itab_ty *change2time_stp;

    /**
      * The history_path is used to remember the absolute path of the
      * distory directory.  The calculation is deferred until needed.
      * Always use the history_path_get method, never access this
      * instance variable directly.
      */
    string_ty *history_path;

    /**
      * The info_path instance variable is used to remember the location
      * of the meta-data file for this project (branch).  Never access
      * the info_path variable directly, always go via the info_path_get
      * method.
      */
    string_ty *info_path;

    /**
      * The pstate_path instance variable is used to remember the
      * absolute path of the pstate (project state) file.  It is
      * calculated as needed.  Never access this instance variable
      * directly, always go via the pstate_path_get method.
      */
    string_ty *pstate_path;

    /**
      * The changes_path instance variable is used to remember the
      * absolute path of the directory containing change set meta-data.
      * It is calculated as needed.  Never access this instance variable
      * directly, always go via the changes_path_get method.
      */
    string_ty *changes_path;

    /**
      * The pstate_data instance variable is used to remember the
      * project state data.  It is calculated as needed.  Never
      * access this instance variable directly, always go via the
      * pstate_get method.
      */
    pstate_ty *pstate_data;

    /**
      * The is_a_new_file instance variable is used to remember whether
      * the pstate file is a new file which has to be created.
      */
    bool is_a_new_file;

    /**
      * The lock_magic instance variable is used to remember the last
      * time the lock was taken.  If the lock_sync function returns
      * different, it means we have to invalidate all our project data
      * and read it in again, because something could have changed.
      * Only every accessed by the lock_sync ethod.
      */
    long lock_magic;

    /**
      * The pcp instance variable is used to remember the change object
      * representing this branch's state.
      *
      * @note
      *     Never access this instance variable directly, event from a
      *     project_ty method, always go via the change_get() method in
      *     case it has not been calculated yet.
      */
    change::pointer pcp;

    /**
      * The uid instance variable is used to remember the Unix user id
      * for the project owner.  This is set by the get_the_owner method.
      */
    int uid;

    /**
      * The gid instance variable is used to remember the Unix group id
      * for the project owner.  This is set by the get_the_owner method.
      */
    int gid;

    /**
      * The parent instance variable is used to remember the project
      * which this project is a branch of.  If it is NULL, then this is
      * a trunk project.
      */
    project_ty *parent;

    /**
      * The parent_bn instance variable is used to remember the change
      * (branch) number of this project relative to the parent branch.
      * If it is TRUNK_BRANCH_NUMER, then this is a trunk project.
      */
    long parent_bn;

    /**
      * The file_list instance variable is used to remember the list
      * of files in the project, one list for each style of view path.
      * Never access this instance variable directly, always go via the
      * file_list_get method, they are calculated on demand.
      */
    struct string_list_ty *file_list[view_path_MAX];

    /**
      * The file_by_uuid instance variable is used to remember a symbol
      * table of files indexed by UUID (or by file name if your project
      * has any files without a UUID for backwards compatibility).
      *
      * This instance variable shall always be accessed via the
      * find_file_by_uuid method.  The tables are calculated on demand
      * and cached.
      */
    struct symtab_ty *file_by_uuid[view_path_MAX];

    //
    // If you add an instance variable to this class, make sure you
    // dispose of it in the destructor and initialize in in the
    // constructor.  Both may be found in libaegis/project.cc
    //


private:
    /**
      * The up instance variable is used to remember the user (and
      * group) which owns this project.
      */
    user_ty::pointer up;

    /**
      * The off_limits instance variable is used to remember when
      * a project is inaccessable to the executing user.  This
      * flag is normally false, but it will be set to true by
      * project::bind_existing for projects which are inaccessable.
      */
    bool off_limits;

    /**
      * The default constructor.
      *
      * Do not use this method.  It is not defined.  Projects will
      * always have a name, it it shall always be supplied to the
      * constructor.
      */
    project_ty();

    /**
      * The copy constructor.  Do not use.
      *
      * The prefereed mechanism is to use the reference counting
      * provided the the project class (see below) smart pointer.
      */
    project_ty(const project_ty &);

    /**
      * The assignment operator.  Do not use.
      *
      * The prefereed mechanism is to use the reference counting
      * provided the the project class (see below) smart pointer.
      */
    project_ty &operator=(const project_ty &);
};

project_ty *project_alloc(string_ty *name);

inline DEPRECATED void
project_bind_existing(project_ty *pp)
{
    pp->bind_existing();
}

inline DEPRECATED bool
project_bind_existing_errok(project_ty *pp)
{
    return pp->bind_existing_errok();
}

inline DEPRECATED project_ty *
project_bind_branch(project_ty *ppp, change::pointer bp)
{
    return ppp->bind_branch(bp);
}

inline DEPRECATED void
project_bind_new(project_ty *pp)
{
    pp->bind_new();
}

void project_list_get(struct string_list_ty *);

inline DEPRECATED void
project_list_inner(struct string_list_ty *result, project_ty *pp)
{
    pp->list_inner(*result);
}

inline DEPRECATED project_ty *
project_find_branch(project_ty *pp, const char *number)
{
    return pp->find_branch(number);
}

void project_free(project_ty *);

inline string_ty *
project_name_get(project_ty *pp)
{
    return pp->name_get();
}

project_ty *project_copy(project_ty *);

inline DEPRECATED change::pointer
project_change_get(project_ty *pp)
{
    return pp->change_get();
}

inline DEPRECATED string_ty *
project_home_path_get(project_ty *pp)
{
    return pp->home_path_get();
}

string_ty *project_Home_path_get(project_ty *);
string_ty *project_top_path_get(project_ty *, int);

/**
  * The project_rss_path_get function is used to get the path of the RSS
  * directory for the given project.
  *
  * @param pp
  *     The project in question
  * @param resolve
  *     Whether or not to resolve symlinks in the path.
  * @returns
  *     a string containing the absolute path
  */
nstring project_rss_path_get(project_ty *pp, bool resolve = false);

inline DEPRECATED void
project_home_path_set(project_ty *pp, string_ty *dir)
{
    pp->home_path_set(dir);
}

inline DEPRECATED string_ty *
project_baseline_path_get(project_ty *pp, bool resolve = false)
{
    return pp->baseline_path_get(resolve);
}

inline DEPRECATED string_ty *
project_history_path_get(project_ty *pp)
{
    return pp->history_path_get();
}

/**
  * The project_history_filename_get function is used to determine the
  * absolute path pf the file used to contain the history of the given file.
  *
  * @returns
  *     a pointer to a string.  You are required to str_free this string
  *     when you are done with it.
  */
string_ty *project_history_filename_get(project_ty *, struct fstate_src_ty *);

inline DEPRECATED string_ty *
project_info_path_get(project_ty *pp)
{
    return pp->info_path_get();
}

inline DEPRECATED string_ty *
project_changes_path_get(project_ty *pp)
{
    return pp->changes_path_get();
}

DEPRECATED
string_ty *project_change_path_get(project_ty *, long);

inline DEPRECATED string_ty *
project_pstate_path_get(project_ty *pp)
{
    return pp->pstate_path_get();
}

inline DEPRECATED pstate_ty *
project_pstate_get(project_ty *pp)
{
    return pp->pstate_get();
}

inline DEPRECATED void
project_pstate_write(project_ty *pp)
{
    pp->pstate_write();
}

void project_pstate_write_top(project_ty *);

inline DEPRECATED void
project_pstate_lock_prepare(project_ty *pp)
{
    pp->pstate_lock_prepare();
}

void project_pstate_lock_prepare_top(project_ty *);
void project_baseline_read_lock_prepare(project_ty *);
void project_baseline_write_lock_prepare(project_ty *);
void project_history_lock_prepare(project_ty *);
void project_error(project_ty *, struct sub_context_ty *, const char *);
void project_fatal(project_ty *, struct sub_context_ty *, const char *)
    NORETURN;
void project_verbose(project_ty *, struct sub_context_ty *, const char *);
void project_change_append(project_ty *, long, int);
void project_change_delete(project_ty *, long);
int project_change_number_in_use(project_ty *, long);
string_ty *project_version_short_get(project_ty *);
string_ty *project_version_get(project_ty *);

inline DEPRECATED int
project_uid_get(project_ty *pp)
{
    return pp->uid_get();
}

inline DEPRECATED int
project_gid_get(project_ty *pp)
{
    return pp->gid_get();
}

user_ty::pointer project_user(project_ty *);
void project_become(project_ty *);
void project_become_undo(project_ty *pp);
long project_next_test_number_get(project_ty *);
int project_is_readable(project_ty *);
long project_minimum_change_number_get(project_ty *);
void project_minimum_change_number_set(project_ty *, long);
bool project_reuse_change_numbers_get(project_ty *);
void project_reuse_change_numbers_set(project_ty *, bool);
long project_minimum_branch_number_get(project_ty *);
void project_minimum_branch_number_set(project_ty *, long);
bool project_skip_unlucky_get(project_ty *);
void project_skip_unlucky_set(project_ty *, bool);
bool project_compress_database_get(project_ty *);
void project_compress_database_set(project_ty *, bool);
int project_develop_end_action_get(project_ty *);
void project_develop_end_action_set(project_ty *, int);
bool project_protect_development_directory_get(project_ty *);
void project_protect_development_directory_set(project_ty *, bool);

int break_up_version_string(const char *, long *, int, int *, int);
void extract_version_from_project_name(string_ty **, long *, int, int *);
int project_name_ok(string_ty *);

pconf_ty *project_pconf_get(project_ty *);

/**
  * The project_new_branch function is used to create new branches.
  *
  * @param pp
  *     The project to create the branch below.
  * @param up
  *     The authorised user requesting the new branch.
  * @param change_number
  *     The change number to use for the new branch.
  * @param topdir
  *     The change (branch) directory, It is in the same place as the
  *     rest of the project, unless otherwise specified (i.e. not NULL).
  *     This makes it easy to collect the whole project tree, branches,
  *     info and all, in a simple tar or cpio command for archiving or
  *     moving.
  * @param reason
  *     The reason the user gave on the command line, or NULL for no
  *     reason to be attached to the change's history.
  * @returns
  *     A pointer to the new project represented by the new branch.
  */
project_ty *project_new_branch(project_ty *pp, user_ty::pointer up,
    long change_number, string_ty *topdir = 0, string_ty *reason = 0);

inline DEPRECATED void
project_file_list_invalidate(project_ty *pp)
{
    pp->file_list_invalidate();
}

string_ty *project_brief_description_get(project_ty *);

/**
  * The project_uuid_find function is used to locate a change given its UUID.
  *
  * @param pp
  *     The project to search.  Will rewind to the trunk project
  *     before the search commences.
  * @param uuid
  *     The change UUID to search for.
  * @returns
  *     a pointer to the change with th given UUID, or NULL if no change
  *     has the given UUID.
  */
change::pointer project_uuid_find(project_ty *pp, string_ty *uuid);


/**
  * The project clas sis used to represent a "smart pointer" (wich is
  * neither smart nor a pointer) to a project_ty object.
  */
class project
{
public:
    /**
      * The destructor.  It is not vurtual.
      * Thou shalt not derive from this class.
      */
    ~project();

    /**
      * The copy constructor.
      */
    project(const project &arg);

    /**
      * The assignment operator.
      */
    project &operator=(const project &arg);

    /**
      * The constructor.
      *
      * @param name
      *     The name of the project.
      */
    project(string_ty *name);

    /**
      * The constructor.
      *
      * @param name
      *     The name of the project.
      */
    project(const nstring &name);

    /**
      * The member dereference operator is used to obtain the actual
      * object to dereference.
      */
    project_ty *operator->() const { return ref; }

private:
    /**
      * The ref instance variable is used to remember the project_ty
      * object being referenced.  Reference counting is used to ensure
      * it isn't deleted too early.
      */
    project_ty *ref;

    /**
      * The default constructor.  Do not use.  (You always need a
      * project name.)
      */
    project();
};

#endif // PROJECT_H
