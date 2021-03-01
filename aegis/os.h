/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993 Peter Miller.
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
 * MANIFEST: interface definition for aegis/os.c
 */

#ifndef OS_H
#define OS_H

#include <time.h>

#include <main.h>
#include <str.h>

#define OS_EXEC_FLAG_NO_INPUT 0
#define OS_EXEC_FLAG_INPUT 1
#define OS_EXEC_FLAG_ERROK 2

int os_exists _((string_ty *));
void os_mkdir _((string_ty *path, int mode));
void os_rmdir _((string_ty *));
void os_rmdir_errok _((string_ty *));
void os_mkdir_between _((string_ty *root, string_ty *rel, int mode));
void os_rename _((string_ty *, string_ty *));
void os_unlink _((string_ty *));
void os_unlink_errok _((string_ty *));
string_ty *os_curdir _((void));
string_ty *os_pathname _((string_ty *, int));
string_ty *os_dirname _((string_ty *));
string_ty *os_entryname _((string_ty *));
string_ty *os_below_dir _((string_ty *, string_ty *));
void os_chdir _((string_ty *));
void os_setuid _((int));
void os_setgid _((int));
void os_execute _((string_ty *cmd, int flags, string_ty *dir));
int os_execute_retcode _((string_ty *cmd, int flags, string_ty *dir));
string_ty *os_execute_slurp _((string_ty *cmd, int flags, string_ty *dir));
time_t os_mtime _((string_ty *));
void os_mtime_set _((string_ty *, time_t));
void os_chown_check _((string_ty *path, int mode, int uid, int gid));
void os_chmod _((string_ty *, int));
void os_chmod_errok _((string_ty *, int));
int os_chmod_query _((string_ty *));
void os_link _((string_ty *from, string_ty *to));
int os_testing_mode _((void));
void os_become_init _((void));
void os_become _((int uid, int gid, int umask));
void os_become_undo _((void));
void os_become_orig _((void));
void os_become_query _((int *uid, int *gid, int *umask));
void os_become_orig_query _((int *uid, int *gid, int *umask));

int os_become_active _((void));
#define os_become_must_be_active() \
	os_become_must_be_active_gizzards(__FILE__, __LINE__)
void os_become_must_be_active_gizzards _((char *, int));
#define os_become_must_not_be_active() \
	os_become_must_not_be_active_gizzards(__FILE__, __LINE__)
void os_become_must_not_be_active_gizzards _((char *, int));

int os_background _((void));
int os_readable _((string_ty *));
int os_waitpid _((int child, int *status));
int os_waitpid_status _((int child, char *cmd));

char *os_shell _((void));

void os_edit _((string_ty *));
string_ty *os_edit_new _((void));
string_ty *os_edit_filename _((void));

#endif /* OS_H */
