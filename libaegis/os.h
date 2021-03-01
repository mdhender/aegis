/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1997, 1999, 2001-2003 Peter Miller;
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
 * MANIFEST: interface definition for aegis/os.c
 */

#ifndef OS_H
#define OS_H

#include <ac/time.h>

#include <main.h>
#include <str.h>

struct string_list_ty; /* forward */

#define OS_EXEC_FLAG_NO_INPUT 0
#define OS_EXEC_FLAG_INPUT 1
#define OS_EXEC_FLAG_ERROK 2
#define OS_EXEC_FLAG_SILENT 4

int os_exists(string_ty *);
void os_mkdir(string_ty *path, int mode);
void os_rmdir(string_ty *);
void os_rmdir_errok(string_ty *);
void os_rmdir_bg(string_ty *);
void os_rmdir_tree(string_ty *);
void os_mkdir_between(string_ty *root, string_ty *rel, int mode);
void os_rename(string_ty *, string_ty *);
void os_unlink(string_ty *);
void os_unlink_errok(string_ty *);
string_ty *os_curdir(void);
string_ty *os_path_cat(string_ty *, string_ty *);
string_ty *os_path_rel2abs(string_ty *, string_ty *);
string_ty *os_pathname(string_ty *, int);
string_ty *os_dirname(string_ty *);
string_ty *os_dirname_relative(string_ty *);
string_ty *os_entryname(string_ty *);
string_ty *os_entryname_relative(string_ty *);
string_ty *os_below_dir(string_ty *, string_ty *);
void os_chdir(string_ty *);
void os_setuid(int);
void os_setgid(int);
void os_execute(string_ty *cmd, int flags, string_ty *dir);
int os_execute_retcode(string_ty *cmd, int flags, string_ty *dir);
string_ty *os_execute_slurp(string_ty *cmd, int flags, string_ty *dir);
void os_xargs(string_ty *the_command, struct string_list_ty *the_list,
    string_ty *dir);
long os_file_size(string_ty *);
void os_mtime_range(string_ty *, time_t *, time_t *);
time_t os_mtime_actual(string_ty *);
void os_mtime_set(string_ty *, time_t);
void os_mtime_set_errok(string_ty *, time_t);
void os_chown_check(string_ty *path, int mode, int uid, int gid);
void os_chmod(string_ty *, int);
void os_chmod_errok(string_ty *, int);
int os_chmod_query(string_ty *);
void os_link(string_ty *from, string_ty *to);
int os_testing_mode(void);
void os_become_init(void);
void os_become_init_mortal(void);
void os_become(int uid, int gid, int umsk);
void os_become_undo(void);
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

int os_readable(string_ty *);
int os_executable(string_ty *);
int os_waitpid(int child, int *status);
int os_waitpid_status(int child, const char *cmd);

const char *os_shell(void);

enum edit_ty
{
	edit_not_set,
	edit_foreground,
	edit_background
};
typedef enum edit_ty edit_ty;

void os_edit(string_ty *, edit_ty);
string_ty *os_edit_string(string_ty *, edit_ty);
string_ty *os_edit_new(edit_ty);
string_ty *os_edit_filename(int);
string_ty *os_tmpdir(void);

int os_pathconf_name_max(string_ty *);
int os_pathconf_path_max(string_ty *);
void os_symlink(string_ty *, string_ty *);
string_ty *os_readlink(string_ty *);
int os_symlink_query(string_ty *);
void os_throttle(void);
void os_owner_query(string_ty *, int *, int *);
string_ty *os_fingerprint(string_ty *);

void os_interrupt_register(void);
void os_interrupt_cope(void);
void os_interrupt_ignore(void);
int os_interrupt_has_occurred(void);

int os_isa_directory(string_ty *);
int os_isa_special_file(string_ty *); /* !S_IFREG */

#endif /* OS_H */
