/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1997, 1999, 2001, 2002 Peter Miller;
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

#define OS_EXEC_FLAG_NO_INPUT 0
#define OS_EXEC_FLAG_INPUT 1
#define OS_EXEC_FLAG_ERROK 2
#define OS_EXEC_FLAG_SILENT 4

int os_exists _((string_ty *));
void os_mkdir _((string_ty *path, int mode));
void os_rmdir _((string_ty *));
void os_rmdir_errok _((string_ty *));
void os_rmdir_bg _((string_ty *));
void os_rmdir_tree _((string_ty *));
void os_mkdir_between _((string_ty *root, string_ty *rel, int mode));
void os_rename _((string_ty *, string_ty *));
void os_unlink _((string_ty *));
void os_unlink_errok _((string_ty *));
string_ty *os_curdir _((void));
string_ty *os_path_cat _((string_ty *, string_ty *));
string_ty *os_path_rel2abs _((string_ty *, string_ty *));
string_ty *os_pathname _((string_ty *, int));
string_ty *os_dirname _((string_ty *));
string_ty *os_dirname_relative _((string_ty *));
string_ty *os_entryname _((string_ty *));
string_ty *os_entryname_relative _((string_ty *));
string_ty *os_below_dir _((string_ty *, string_ty *));
void os_chdir _((string_ty *));
void os_setuid _((int));
void os_setgid _((int));
void os_execute _((string_ty *cmd, int flags, string_ty *dir));
int os_execute_retcode _((string_ty *cmd, int flags, string_ty *dir));
string_ty *os_execute_slurp _((string_ty *cmd, int flags, string_ty *dir));
long os_file_size _((string_ty *));
void os_mtime_range _((string_ty *, time_t *, time_t *));
time_t os_mtime_actual _((string_ty *));
void os_mtime_set _((string_ty *, time_t));
void os_mtime_set_errok _((string_ty *, time_t));
void os_chown_check _((string_ty *path, int mode, int uid, int gid));
void os_chmod _((string_ty *, int));
void os_chmod_errok _((string_ty *, int));
int os_chmod_query _((string_ty *));
void os_link _((string_ty *from, string_ty *to));
int os_testing_mode _((void));
void os_become_init _((void));
void os_become_init_mortal _((void));
void os_become _((int uid, int gid, int umsk));
void os_become_undo _((void));
void os_become_orig _((void));
void os_become_query _((int *uid, int *gid, int *umsk));
void os_become_orig_query _((int *uid, int *gid, int *umsk));

int os_become_active _((void));
#define os_become_must_be_active() \
	os_become_must_be_active_gizzards(__FILE__, __LINE__)
void os_become_must_be_active_gizzards _((char *, int));
#define os_become_must_not_be_active() \
	os_become_must_not_be_active_gizzards(__FILE__, __LINE__)
void os_become_must_not_be_active_gizzards _((char *, int));

int os_background _((void));
int os_readable _((string_ty *));
int os_executable _((string_ty *));
int os_waitpid _((int child, int *status));
int os_waitpid_status _((int child, char *cmd));

char *os_shell _((void));

enum edit_ty
{
	edit_not_set,
	edit_foreground,
	edit_background
};
typedef enum edit_ty edit_ty;

void os_edit _((string_ty *, edit_ty));
string_ty *os_edit_string _((string_ty *, edit_ty));
string_ty *os_edit_new _((edit_ty));
string_ty *os_edit_filename _((int));
string_ty *os_tmpdir _((void));

int os_pathconf_name_max _((string_ty *));
int os_pathconf_path_max _((string_ty *));
void os_symlink _((string_ty *, string_ty *));
string_ty *os_readlink _((string_ty *));
int os_symlink_query _((string_ty *));
void os_throttle _((void));
void os_owner_query _((string_ty *, int *, int *));
string_ty *os_fingerprint _((string_ty *));

void os_interrupt_register _((void));
void os_interrupt_cope _((void));
void os_interrupt_ignore _((void));
int os_interrupt_has_occurred _((void));

int os_isa_directory _((string_ty *));
int os_isa_special_file _((string_ty *)); /* !S_IFREG */

#endif /* OS_H */
