//
//	aegis - project change supervisor
//	Copyright (C) 1991-1997, 1999, 2001-2008 Peter Miller
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

#ifndef OS_H
#define OS_H

/** \addtogroup OS
  * \brief Access OS services
  * \ingroup AegisLibrary
  * @{
  */
#include <common/ac/time.h>

#include <common/main.h>
#include <common/nstring.h>

class string_list_ty; // forward
class nstring_list; // forward

#define OS_EXEC_FLAG_NO_INPUT 0
#define OS_EXEC_FLAG_INPUT 1
#define OS_EXEC_FLAG_ERROK 2
#define OS_EXEC_FLAG_SILENT 4

/**
  * The os_exists function is used to determine if the given path
  * exists.  It does not follow symlinks.
  *
  * \param path
  *     The path to check for existance.
  * \param eaccess_is_ok
  *     If true, the EACCES error is also cause for returning false,
  *     rather than reporting a fatal error and exiting.
  * \returns
  *     int; non-zero if the file exists, zero if the file doesn't
  *     (ENOENT or ENOTDIR)
  * \note
  *     This function does not return for any error other than ENOENT or
  *     ENOTDIR, but prints an error and exits.
  */
int os_exists(string_ty *path, bool eaccess_is_ok = false);

/**
  * The os_exists function is used to determine if the given path
  * exists.  It does not follow symlinks.
  *
  * \param path
  *     The path to check for existance.
  * \param eaccess_is_ok
  *     If true, the EACCES error is also cause for returning false,
  *     rather than reporting a fatal error and exiting.
  * \returns
  *     bool; true if the file exists, false if the file doesn't exist
  *     (ENOENT or ENOTDIR).
  * \note
  *     This function does not return for any error other than ENOENT or
  *     ENOTDIR, but prints an error and exits.
  */
bool os_exists(const nstring &path, bool eaccess_is_ok = false);

/**
  * The os_mkdir function is used to create a new directory.  It does
  * not mak intermediate directories.
  *
  * \param path
  *     The path opf the directory to be created.
  * \param mode
  *     The mode of the created directory.
  * \note
  *     This function does not return for any error, but prints an error
  *     and exits.
  */
void os_mkdir(string_ty *path, int mode);

/**
  * The os_mkdir function is used to create a new directory.  It does
  * not mak intermediate directories.
  *
  * \param path
  *     The path opf the directory to be created.
  * \param mode
  *     The mode of the created directory.
  * \note
  *     This function does not return for any error, but prints an error
  *     and exits.
  */
void os_mkdir(const nstring &path, int mode);

/**
  * The os_rmdir function is used to remove an empty directory.
  *
  * \param path
  *     The path of the directory to be removed.
  * \note
  *     This funtion does not return if the is an error; instead it
  *     prints an error message and exits.
  */
void os_rmdir(string_ty *path);

/**
  * The os_rmdir function is used to remove an empty directory.
  *
  * \param path
  *     The path of the directory to be removed.
  * \note
  *     This funtion does not return if the is an error; instead it
  *     prints an error message and exits.
  */
void os_rmdir(const nstring &path);

/**
  * The os_rmdir_errok function is used to remove an empty directory.
  * It ignores errors.
  *
  * \param path
  *     The path of the directory to be removed.
  */
void os_rmdir_errok(string_ty *path);

/**
  * The os_rmdir_errok function is used to remove an empty directory.
  * It ignores errors.
  *
  * \param path
  *     The path of the directory to be removed.
  */
void os_rmdir_errok(const nstring &path);

/**
  * The os_rmdir_bg function removes a directory tree in the background.
  *
  * \param path
  *     The directory tree to be removed.
  */
void os_rmdir_bg(string_ty *path);

/**
  * The os_rmdir_bg function removes a directory tree in the background.
  *
  * \param path
  *     The directory tree to be removed.
  */
void os_rmdir_bg(const nstring &path);

/**
  * The os_rmdir_tree function is used to remive directory trees.
  *
  * \param path
  *     The path of the directory tree to be removed.
  * \note
  *     This function will not return if there is any error removing
  *     the directory tree, instead it will report the error, abort the
  *     remove, and exit.
  */
void os_rmdir_tree(string_ty *path);

/**
  * The os_rmdir_tree function is used to remive directory trees.
  *
  * \param path
  *     The path of the directory tree to be removed.
  * \note
  *     This function will not return if there is any error removing
  *     the directory tree, instead it will report the error, abort the
  *     remove, and exit.
  */
void os_rmdir_tree(const nstring &path);

/**
  * The os_mkdir_between function is used to make intermediate
  * directories, of necessary between a root and a relative destination.
  * The final portion is not created.
  *
  * \param root
  *     The root directory.
  * \param rel
  *     The path, relative to the root, which needs to have intermediate
  *     directories created.
  * \param mode
  *     The permissions mode for any created directories.
  * \note
  *     This function will not return if there is any error creating a
  *     directory, instead it will report the error and exit.
  */
void os_mkdir_between(string_ty *root, string_ty *rel, int mode);

/**
  * The os_mkdir_between function is used to make intermediate
  * directories, of necessary between a root and a relative destination.
  * The final portion is not created.
  *
  * \param root
  *     The root directory.
  * \param rel
  *     The path, relative to the root, which needs to have intermediate
  *     directories created.
  * \param mode
  *     The permissions mode for any created directories.
  * \note
  *     This function will not return if there is any error creating a
  *     directory, instead it will report the error and exit.
  */
void os_mkdir_between(const nstring &root, const nstring &rel, int mode);

/**
  * The os_rename function is used to change the name of a file.
  *
  * Any other hard links to the file (as created using link(2)) are
  * unaffected.
  *
  * If newpath already exists it will be atomically replaced (subject
  * to a few conditions), so that there is no point at which another
  * process attempting to access newpath will find it missing.
  *
  * If newpath exists but the operation fails for some reason rename
  * guarantees to leave an instance of newpath in place.
  *
  * However, when overwriting there will probably be a window in which
  * both oldpath and newpath refer to the file being renamed.
  *
  * If oldpath refers to a symbolic link the link is renamed; if newpath
  * refers to a symbolic link the link will be overwritten.
  *
  * \param oldpath
  *     The name of the file on entry.
  * \param newpath
  *     The name of the file on return.
  *
  * \note
  *     This function will not return if there are any errors.
  */
void os_rename(string_ty *oldpath, string_ty *newpath);

/**
  * The os_rename function is used to change the name of a file.
  *
  * Any other hard links to the file (as created using link(2)) are
  * unaffected.
  *
  * If newpath already exists it will be atomically replaced (subject
  * to a few conditions), so that there is no point at which another
  * process attempting to access newpath will find it missing.
  *
  * If newpath exists but the operation fails for some reason rename
  * guarantees to leave an instance of newpath in place.
  *
  * However, when overwriting there will probably be a window in which
  * both oldpath and newpath refer to the file being renamed.
  *
  * If oldpath refers to a symbolic link the link is renamed; if newpath
  * refers to a symbolic link the link will be overwritten.
  *
  * \param oldpath
  *     The name of the file on entry.
  * \param newpath
  *     The name of the file on return.
  *
  * \note
  *     This function does not return if there is an error; it prints an
  *     error message and exits.
  */
void os_rename(const nstring &oldpath, const nstring &newpath);

/**
  * The os_unlink function deletes a name from the filesystem. If that
  * name was the last link to a file and no processes have the file open
  * the file is deleted and the space it was using is made available for
  * reuse.
  *
  * If the name was the last link to a file but any processes still have
  * the file open the file will remain in existence until the last file
  * descriptor referring to it is closed.
  *
  * If the name referred to a symbolic link the link is removed.
  *
  * If the name referred to a socket, fifo or device the name for it is
  * removed but processes which have the object open may continue to use
  * it.
  *
  * \param path
  *     The name of the file to be deleted.
  *
  * \note
  *     This function does not return if there is an error; it prints an
  *     error message and exits.
  */
void os_unlink(string_ty *path);

/**
  * The os_unlink function deletes a name from the filesystem. If that
  * name was the last link to a file and no processes have the file open
  * the file is deleted and the space it was using is made available for
  * reuse.
  *
  * If the name was the last link to a file but any processes still have
  * the file open the file will remain in existence until the last file
  * descriptor referring to it is closed.
  *
  * If the name referred to a symbolic link the link is removed.
  *
  * If the name referred to a socket, fifo or device the name for it is
  * removed but processes which have the object open may continue to use
  * it.
  *
  * \param path
  *     The name of the file to be deleted.
  * \note
  *     This function does not return if there is an error; it prints an
  *     error message and exits.
  */
void os_unlink(const nstring &path);

/**
  * The os_unlink_errok function deletes a name from the filesystem. If
  * that name was the last link to a file and no processes have the
  * file open the file is deleted and the space it was using is made
  * available for reuse.
  *
  * If the name was the last link to a file but any processes still have
  * the file open the file will remain in existence until the last file
  * descriptor referring to it is closed.
  *
  * If the name referred to a symbolic link the link is removed.
  *
  * If the name referred to a socket, fifo or device the name for it is
  * removed but processes which have the object open may continue to use
  * it.
  *
  * \param path
  *     The name of the file to be deleted.
  * \note
  *     This function ignores any errors.
  */
void os_unlink_errok(string_ty *path);

/**
  * The os_unlink_errok function deletes a name from the filesystem. If
  * that name was the last link to a file and no processes have the
  * file open the file is deleted and the space it was using is made
  * available for reuse.
  *
  * If the name was the last link to a file but any processes still have
  * the file open the file will remain in existence until the last file
  * descriptor referring to it is closed.
  *
  * If the name referred to a symbolic link the link is removed.
  *
  * If the name referred to a socket, fifo or device the name for it is
  * removed but processes which have the object open may continue to use
  * it.
  *
  * \param path
  *     The name of the file to be deleted.
  * \note
  *     This function ignores any errors.
  */
void os_unlink_errok(const nstring &path);

/**
  * The os_curdir function is used to obtain the absolute path of the
  * current directory.
  *
  * \note
  *     This function does not return if there is an error; it prints an
  *     error message and exits.
  */
string_ty *os_curdir(void);

/**
  * The os_path_join function may be used to joint two strings together
  * with a slash between them.
  *
  * \param lhs
  *     The root or upper portion.
  * \param rhs
  *     The relative or lower portion.
  */
string_ty *os_path_join(string_ty *lhs, string_ty *rhs);

/**
  * The os_path_join function may be used to joint two strings together
  * with a slash between them.
  *
  * \param lhs
  *     The root or upper portion.
  * \param rhs
  *     The relative or lower portion.
  */
nstring os_path_join(const nstring &lhs, const nstring &rhs);

/**
  * The os_path_join function may be used to joint two strings together
  * with a slash between them.
  *
  * \param lhs
  *     The root or upper portion.
  * \param rhs
  *     The relative or lower portion.
  */
nstring os_path_join(const nstring &lhs, const nstring &rhs);

/**
  * The os_path_cat function is used to carefully join two path
  * components together with a slash.  Care is taken to remove redundant
  * "." components and redundant slashes.
  *
  * \param lhs
  *     The directory the next part is relative to.
  * \param rhs
  *     The additional path, relative to the first argument.
  */
string_ty *os_path_cat(string_ty *lhs, string_ty *rhs);

/**
  * The os_path_cat function is used to carefully join two path
  * components together with a slash.  Care is taken to remove redundant
  * "." components and redundant slashes.
  *
  * \param lhs
  *     The directory the next part is relative to.
  * \param rhs
  *     The additional path, relative to the first argument.
  */
nstring os_path_cat(const nstring &lhs, const nstring &rhs);

/**
  * The os_path_cat function is used to carefully joint two path
  * components together with a slash.  Care is taken to remove redundant
  * "." components and redundant slashes.
  *
  * \param lhs
  *     The directory the next part is relative to.
  * \param rhs
  *     The additional path, relative to the first argument.
  */
nstring os_path_cat(const nstring &lhs, const nstring &rhs);

/**
  * The os_path_rel2abs function is used to make relative paths
  * absolute, if they need it.
  *
  * \param root
  *     The root for relative paths.
  * \param path
  *     The path to (conditionally) be made absolute.
  * \returns
  *     The "path" if it was already abvsolute, otherwise the "root" is
  *     used to make the "path" absolute.
  */
string_ty *os_path_rel2abs(string_ty *root, string_ty *path);

/**
  * The os_path_rel2abs function is used to make relative paths
  * absolute, if they need it.
  *
  * \param root
  *     The root for relative paths.
  * \param path
  *     The path to (conditionally) be made absolute.
  * \returns
  *     The "path" if it was already abvsolute, otherwise the "root" is
  *     used to make the "path" absolute.
  */
nstring os_path_rel2abs(const nstring &root, const nstring &path);

/**
  * Os_pathname is used to determine the full path name
  * of a partial path given.
  *
  * \param path
  *     path to canonicalize
  * \param resolve
  *     non-zero if should resolve symlinks, 0 if not
  * \returns
  *     pointer to dynamically allocated string.
  * \note
  *     Use str_free() when you are done with the value returned.
  */
string_ty *os_pathname(string_ty *path, int resolve);

/**
  * Os_pathname is used to determine the full path name
  * of a partial path given.
  *
  * \param path
  *     path to canonicalize
  * \param resolve
  *     true if should resolve symlinks, false if not
  */
nstring os_pathname(const nstring &path, bool resolve);

/**
  * The os_basename function strip directory and suffix from
  * filenames.  (see basename(1))
  *
  * \param name
  *     the filename to process
  * \param ext
  *     the (optional) suffix to be stripped
  *
  */
string_ty *os_basename(string_ty *name, string_ty *ext = NULL);

/**
  * The os_basename function strip directory and suffix from
  * filenames.  (see basename(1))
  *
  * \param name
  *     the filename to process
  * \param ext
  *     the (optional) suffix to be stripped
  *
  */
nstring os_basename(const nstring &name, const nstring &ext = "");

/**
  * The os_dirname function is used to extract the directory part of a
  * path (i.e. the last /component removed).
  *
  * \param path
  *     The path to be dismembered.
  * \note
  *     The pash is resolved via os_pathname before the directory part is
  *     extracted.
  */
string_ty *os_dirname(string_ty *path);

/**
  * The os_dirname function is used to extract the directory part of a
  * path (i.e. the last /component removed).
  *
  * \param path
  *     The path to be dismembered.
  * \note
  *     The pash is resolved via os_pathname before the directory part is
  *     extracted.
  */
nstring os_dirname(const nstring &path);

/**
  * The os_dirname_relative function is used to extract the directory
  * part of a path (i.e. the last /component removed). If there is no
  * slash (/) in the filename, "." is returned.
  *
  * \param path
  *     The path to be dismembered.
  */
string_ty *os_dirname_relative(string_ty *path);

/**
  * The os_dirname_relative function is used to extract the directory
  * part of a path (i.e. the last /component removed). If there is no
  * slash (/) in the filename, "." is returned.
  *
  * \param path
  *     The path to be dismembered.
  */
nstring os_dirname_relative(const nstring &path);

/**
  * The os_entryname function is used to extract the last pathname
  * portion of the given path.
  *
  * \param path
  *     The path to be dismembered.
  * \note
  *     The path is resolved via os_pathname before the last portion of
  *     the path is extracted.
  */
string_ty *os_entryname(string_ty *path);

/**
  * The os_entryname function is used to extract the last pathname
  * portion of the given path.
  *
  * \param path
  *     The path to be dismembered.
  * \note
  *     The path is resolved via os_pathname before the last portion of
  *     the path is extracted.
  */
nstring os_entryname(const nstring &path);

/**
  * The os_entryname_relative function is used to extract the last
  * pathname portion of the given path.
  *
  * \param path
  *     The path to be dismembered.
  */
string_ty *os_entryname_relative(string_ty *path);

/**
  * The os_entryname_relative function is used to extract the last
  * pathname portion of the given path.
  *
  * \param path
  *     The path to be dismembered.
  */
nstring os_entryname_relative(const nstring &path);

/**
  * The os_below_dir function is used to test whether a given path
  * (lower) is below another directory (upper).
  *
  * \param higher
  *     The top directory.
  * \param lower
  *     The full directory including "higher" and exterding for
  *     additional path compenents.
  * \returns
  *     pointer to string in dynamic memory, being the relative portion
  *     below "higher"; NULL if "lower" is not below "higher"; the empty
  *     string if lower equals upper.
  */
string_ty *os_below_dir(string_ty *higher, string_ty *lower);

/**
  * The os_below_dir function is used to test whether a given path
  * (lower) is below another directory (upper).
  *
  * \param upper
  *     The top directory.
  * \param lower
  *     The full directory including "higher" and exterding for
  *     additional path compenents.
  * \returns
  *     narrow string; being the relative portion below "higher"; or ""
  *     if "lower" is not below "higher"; or "." if lower equals higher.
  * \note
  *     The return values are <b>not</b> the same as the old string_ty
  *     version of this function.
  */
nstring os_below_dir(const nstring &upper, const nstring &lower);

/**
  * The os_chdir function is used to changes the current directory to
  * the specified path.
  *
  * \param path
  *     The directory to change to.
  * \note
  *     This function does not return if there is an error; it printf an
  *     error message and exits.
  */
void os_chdir(string_ty *path);

/**
  * The os_chdir function is used to changes the current directory to
  * the specified path.
  *
  * \param path
  *     The directory to change to.
  * \note
  *     This function does not return if there is an error; it printf an
  *     error message and exits.
  */
void os_chdir(const nstring &path);

/**
  * The os_setuid function is used to set the effective user ID of the
  * current process.  If the effective userid of the caller is root, the
  * real and saved user IDs are also set.
  *
  * Under Linux, setuid is implemented like the POSIX version with the
  * _POSIX_SAVED_IDS feature.  This allows a setuid (other than root)
  * program to drop all of its user privileges, do some un-privileged
  * work, and then re-engage the original effective user ID in a secure
  * manner.
  *
  * If the user is root or the program is setuid root, special care must
  * be taken. The setuid function checks the effective uid of the caller
  * and if it is the superuser, all process related user IDs are set
  * to uid.  After this has occurred, it is impossible for the program
  * to regain root privileges.
  *
  * Thus, a setuid-root program wishing to temporarily drop root
  * privileges, assume the identity of a non-root user, and then regain
  * root privileges afterwards cannot use setuid. You can accomplish
  * this with the (non-POSIX, BSD) call seteuid.
  *
  * \param id
  *     The UID to assume.
  *
  * \note
  *     This function does not return in the event of an error; instead
  *     it prints an error message and exits.
  */
void os_setuid(int id);

/**
  * The os_setgid function si used to set the effective group ID of the
  * current process. If the caller is the superuser, the real and saved
  * group IDs are also set.
  *
  * Under Linux, setgid is implemented like the POSIX version with the
  * _POSIX_SAVED_IDS feature.  This allows a setgid program that is not
  * suid root to drop all of its group privileges, do some un-privileged
  * work, and then re-engage the original effective group ID in a secure
  * manner.
  *
  * \param id
  *     The GID to assume.
  *
  * \note
  *     This function does not return in the event of an error; instead
  *     it prints an error message and exits.
  */
void os_setgid(int id);

/**
  * The os_execute function i sused to execute a command.  It is
  * expected that you have already called os_become to set the user the
  * command is to be executed as.
  *
  * This function does not return if the command returns a non-zero exit
  * status.
  *
  * @param cmd
  *     The command to be executed.
  * @param flags
  *     specific conditions to run the command
  * @param dir
  *     The current directory to execute the command from.
  */
void os_execute(string_ty *cmd, int flags, string_ty *dir);

/**
  * The os_execute function i sused to execute a command.  It is
  * expected that you have already called os_become to set the user the
  * command is to be executed as.
  *
  * This function does not return if the command returns a non-zero exit
  * status.
  *
  * @param cmd
  *     The command to be executed.
  * @param flags
  *     specific conditions to run the command
  * @param dir
  *     The current directory to execute the command from.
  */
void os_execute(const nstring &cmd, int flags, const nstring &dir);

/**
  * The os_execute_retcode function is used to execute a command an
  * returns its exit status.
  *
  * @param cmd
  *     The command to execute
  * @param flags
  *     flags for how to run the command
  * @param dir
  *     the directory in which to run the command
  * @returns
  *     the exit status: zero on success, non-zero on failure
  */
int os_execute_retcode(string_ty *cmd, int flags, string_ty *dir);

/**
  * The os_execute_retcode function is used to execute a command an
  * returns its exit status.
  *
  * @param cmd
  *     The command to execute
  * @param flags
  *     flags for how to run the command
  * @param dir
  *     the directory in which to run the command
  * @returns
  *     the exit status: zero on success, non-zero on failure
  */
int os_execute_retcode(const nstring &cmd, int flags, const nstring &dir);

string_ty *os_execute_slurp(string_ty *cmd, int flags, string_ty *dir);

/**
  * The os_xargs function is used to run a command with command line
  * arguments supplied by a list of strings.  It will break it into
  * chunks if necessary.  This is analogous to the unix xargs(1)
  * command.
  *
  * @param the_command
  *     The command to be executed.  Additional arguments are expected
  *     to have been quoted appropriately already.
  * @param the_list
  *     The arguments to the command.  They will be quoted to protect
  *     shell special characters if necessary.
  * @param dir
  *     The directory the command is to be run in.
  */
void os_xargs(string_ty *the_command, struct string_list_ty *the_list,
    string_ty *dir);

/**
  * The os_xargs function is used to run a command with command line
  * arguments supplied by a list of strings.  It will break it into
  * chunks if necessary.  This is analogous to the unix xargs(1)
  * command.
  *
  * @param the_command
  *     The command to be executed.  Additional arguments are expected
  *     to have been quoted appropriately already.
  * @param the_list
  *     The arguments to the command.  They will be quoted to protect
  *     shell special characters if necessary.
  * @param dir
  *     The directory the command is to be run in.
  */
void os_xargs(const nstring &the_command, const nstring_list &the_list,
    const nstring &dir);

long os_file_size(string_ty *);
void os_mtime_range(string_ty *, time_t *, time_t *);
time_t os_mtime_actual(string_ty *);
void os_mtime_set(string_ty *, time_t);
void os_mtime_set_errok(string_ty *, time_t);
void os_chown_check(string_ty *path, int mode, int uid, int gid);

/**
  * The os_chmod function is used to change the permission mode of a
  * file.  This function does not return if there is an error; instead
  * it prints an error messages and exits.
  *
  * \param path
  *     The path of the file to be changed.
  * \param mode
  *     The permissions mode the file is to assume.
  */
void os_chmod(string_ty *path, int mode);

/**
  * The os_chmod function is used to change the permission mode of a
  * file.  This function does not return if there is an error; instead
  * it prints an error messages and exits.
  *
  * \param path
  *     The path of the file to be changed.
  * \param mode
  *     The permissions mode the file is to assume.
  */
void os_chmod(const nstring &path, int mode);

/**
  * The os_chmod_errok function is used to change the permission mode of
  * a file.  This function ignores arror.
  *
  * \param path
  *     The path of the file to be changed.
  * \param mode
  *     The permissions mode the file is to assume.
  */
void os_chmod_errok(string_ty *path, int mode);

/**
  * The os_chmod_errok function is used to change the permission mode of
  * a file.  This function ignores arror.
  *
  * \param path
  *     The path of the file to be changed.
  * \param mode
  *     The permissions mode the file is to assume.
  */
void os_chmod_errok(const nstring &path, int mode);

int os_chmod_query(string_ty *);

/**
  * The os_link function is used to make a hard link between two files.
  *
  * @param from
  *     The existing file.
  * @param to
  *     The new file to be a hard link of the existing file.
  */
void os_link(string_ty *from, string_ty *to);

/**
  * The os_link function is used to make a hard link between two files.
  *
  * @param from
  *     The existing file.
  * @param to
  *     The new file to be a hard link of the existing file.
  */
void os_link(const nstring &from, const nstring &to);

int os_testing_mode(void);
void os_become_init(void);
void os_become_init_mortal(void);
void os_become_reinit_mortal(void);
void os_become(int uid, int gid, int umsk);

/**
  * The os_become_undo function is used to undo the effects of the
  * os_become function.  It returns the effective uid and gid to root,
  * so that future os_become call will work.
  *
  * It is a bug (and a fatal error will be issued) if there is no
  * matching os_become call.
  */
void os_become_undo(void);

/**
  * The os_become_undo function is used to undo the effects of the
  * os_become function.  It returns the effective uid and gid to root,
  * so that future os_become call will work.
  *
  * It is a bug (and a fatal error will be issued) if there is no
  * matching os_become call.  If DEBUG is enabled, it will cause an
  * assert failure if the uid and gid in the undo does not match the
  * os_become.
  */
void os_become_undo(int uid, int gid);

/**
  * The os_become_undo_atexit function is used to cancel any os_become
  * setting, if one is active.  It performs all of the functions
  * of_os_become_undo, except it is not an error if there has been no
  * matching os_become call.  This function may <b>only</b> be called
  * from quite_action derived classes.
  */
void os_become_undo_atexit(void);

void os_become_orig(void);
void os_become_query(int *uid, int *gid, int *umsk);
void os_become_orig_query(int *uid, int *gid, int *umsk);

int os_become_active(void);
#define os_become_must_be_active() \
	os_become_must_be_active_gizzards(__FILE__, __LINE__)
void os_become_must_be_active_gizzards(const char *, int);
#define os_become_must_not_be_active() \
	os_become_must_not_be_active_gizzards(__FILE__, __LINE__)
void os_become_must_not_be_active_gizzards(const char *, int);


/**
  * \brief
  *	test for backgroundness
  *
  * The background function is used to determine if the curent process
  * is running in the background.
  *
  *\return
  *	zero if process is not in the background, nonzero if the process
  *	is in the background.
  */
int os_background(void);

/**
  * The os_readable function may be used to determine whether or not the
  * given file is readable.
  *
  * @param path
  *     The path (aabsolute or relative) of the file to test.
  * @returns
  *     zero on success, or the errno value on failure.
  */
int os_readable(string_ty *path);

/**
  * The os_readable function may be used to determine whether or not the
  * given file is readable.
  *
  * @param path
  *     The path (aabsolute or relative) of the file to test.
  * @returns
  *     zero on success, or the errno value on failure.
  */
int os_readable(const nstring &path);

bool os_executable(string_ty *);
int os_waitpid(int child, int *status);
int os_waitpid_status(int child, const char *cmd);

const char *os_shell(void);

enum edit_ty
{
	edit_not_set,
	edit_foreground,
	edit_background
};

/**
  * The os_edit function is used to pass the named file to an edirot for
  * the user to edit.  It returns whn the user quit the editor.
  *
  * @param filename
  *     The name of the file to be edited.
  * @param mode
  *     How the editing is to be done.
  */
void os_edit(string_ty *filename, edit_ty mode);

/**
  * The os_edit function is used to pass the named file to an edirot for
  * the user to edit.  It returns whn the user quit the editor.
  *
  * @param filename
  *     The name of the file to be edited.
  * @param mode
  *     How the editing is to be done.
  */
void os_edit(const nstring &filename, edit_ty mode);

string_ty *os_edit_string(string_ty *, edit_ty);
string_ty *os_edit_new(edit_ty);
string_ty *os_edit_filename(int);
string_ty *os_tmpdir(void);

/**
  * The os_pathconf_name_max function may be used to determine the
  * maximum length of a path name component.
  *
  * @param path
  *     the file or directory of interest
  */
int os_pathconf_name_max(const nstring &path);

/**
  * The os_pathconf_name_max function may be used to determine the
  * maximum length of a path name component.
  *
  * @param path
  *     the file or directory of interest
  */
int os_pathconf_name_max(string_ty *path);

/**
  * The os_pathconf_path_max function may be used to determine the
  * maximum length of a file name path, over all components, including
  * separators.
  *
  * @param path
  *     the file or directory of interest
  */
int os_pathconf_path_max(const nstring &path);

/**
  * The os_pathconf_path_max function may be used to determine the
  * maximum length of a file name path, over all components, including
  * separators.
  *
  * @param path
  *     the file or directory of interest
  */
int os_pathconf_path_max(string_ty *path);

/**
  * The os_symlink function is used to make symbolic links.
  *
  * @param src
  *     The src paramter is the source file to be linked from.
  * @param dst
  *     The dst paramter is the destination file to be created.
  */
void os_symlink(string_ty *src, string_ty *dst);

/**
  * The os_symlink function is used to make symbolic links.
  *
  * @param src
  *     The src paramter is the source file to be linked from.
  * @param dst
  *     The dst paramter is the destination file to be created.
  */
void os_symlink(const nstring &src, const nstring &dst);

/**
  * The os_symlink_or_copy function is used to make symbolic links if
  * possible otherwise hard links otherwise make a complete copy.
  *
  * @param src
  *     The src paramter is the source file to be linked from.
  * @param dst
  *     The dst paramter is the destination file to be created.
  */
void os_symlink_or_copy(string_ty *src, string_ty *dst);

string_ty *os_readlink(string_ty *);
int os_symlink_query(string_ty *);

/**
  * This unlikely function is used to slow aegis down.  It is primarily
  * used for Aegis' own tests, to ensure that the time stamps are kosher
  * even on ultra-fast machines.  It is also useful in shell scripts,
  * e.g. automatic integration queue handling.
  *
  * The AEGIS_THROTTLE environment variable is set to the number of
  * seconds to throttle, typically 2.
  */
void os_throttle(void);

/**
  * The os_unthrottle method is used to determine whether or not strict
  * timestamp checks should be performed, because no throoline is in
  * effect.
  *
  * @returns
  *     bool; true if AEGIS_THROTTLE is less than zero,; false if unset
  *     or zero or positive.
  */
bool os_unthrottle(void);

void os_owner_query(string_ty *, int *, int *);
string_ty *os_fingerprint(string_ty *);

void os_interrupt_register(void);
void os_interrupt_cope(void);
void os_interrupt_ignore(void);
int os_interrupt_has_occurred(void);

/**
  * The os_isa_directory function is used to test whether the given
  * absolute path is a directory.
  *
  * \param path
  *     The absolute file path to be examined.
  * \returns
  *     bool; true if the file exists and is a directory,
  *	false if the file does not exist,
  *     false if the file exists and is not a directory.
  */
int os_isa_directory(string_ty *path);

/**
  * The os_isa_symlink function is used to test whether the given
  * absolute path is a symbolic link.
  *
  * \param path
  *     The absolute file path to be examined.
  * \returns
  *     bool; true if the file exists and is a symbolic link,
  *	false if the file does not exists,
  *     false if the file exists and is not a symbolic link.
  */
bool os_isa_symlink(string_ty *path);

/**
  * The os_isa_special_file function is used to test whether the given
  * absolute path is a regular file or not.
  *
  * \param path
  *     The absolute file path to be examined.
  * \returns
  *     bool;
  *	false if the file does not exist,
  *     false if the file exists and is a regular file,
  *	true if the file exists and is a directory,
  *	true if the file exists and is a symlink,
  *     true if the file exists and is not a regular file,
  */
int os_isa_special_file(string_ty *path);

/**
  * The os_magic_file function is used to determine a file type from
  * file contents.
  *
  * @param filename
  *     The name of the file to be examined to determine the file type.
  * @returns
  *     A string describing the file, as a MIME content-type.
  *     E.g. most source files will be "text/plain; charset=us-ascii"
  */
nstring os_magic_file(const nstring &filename);

/**
  * The os_magic_file function is used to determine a file type from
  * file contents.  This is for compatibility and will eventually disappear.
  *
  * @param filename
  *     The name of the file to be examined to determine the file type.
  * @returns
  *     A string describing the file, as a MIME content-type.
  *     E.g. most source files will be "text/plain; charset=us-ascii"
  */
nstring os_magic_file(string_ty *filename);

/**
  * The os_canonify_dirname function is used transform a directory
  * name in canonical form (without the trailing slash).   This is for
  * compatibility and will eventually disappear.
  *
  * @param dirname
  *     The name of the directory to be canonified.
  * @returns
  *     A string with the canonified name of the directory.
  */
string_ty* os_canonify_dirname(string_ty *dirname);

/**
  * The os_canonify_dirname function is used transform a directory
  * name in canonical form (without the trailing slash).
  *
  * @param dirname
  *     The name of the directory to be canonified.
  * @returns
  *     A string with the canonified name of the directory.
  */
nstring os_canonify_dirname(const nstring &dirname);

/**
  * The os_check_path_traversable function is used to check that a path,
  * consisting entirely of existing directories, is traversable (has
  * directory 'x' permissions) by the current user.
  *
  * @param path
  *     The directory path to walk and check.
  * @note
  *     This function does not return if any problem is found; instead
  *     it prints a fatal error message and exits.
  */
void os_check_path_traversable(string_ty *path);

/** @} */
#endif // OS_H
