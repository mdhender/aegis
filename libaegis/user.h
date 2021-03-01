//
//	aegis - project change supervisor
//	Copyright (C) 1992-2000, 2002-2006 Peter Miller;
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
// MANIFEST: interface definition for aegis/user.c
//

#ifndef AEGIS_USER_H
#define AEGIS_USER_H

#include <libaegis/output.h>
#include <libaegis/uconf.h>
#include <libaegis/ustate.h>

struct user_ty
{
    long            reference_count;
    struct project_ty *pp;
    string_ty       *name;
    string_ty       *full_name;
    string_ty       *home;
    string_ty       *group;
    int             uid;
    int             gid;
    int             umask;
    string_ty       *ustate_path;
    ustate_ty       *ustate_data;
    int             ustate_is_new;
    int             ustate_modified;
    string_ty       *uconf_path;
    uconf_ty        *uconf_data;
    long            lock_magic;
    //
    // if you add anything to this structure,
    // make sure you fix user_free in user.c
    //
};


user_ty *user_numeric(struct project_ty *, int);
user_ty *user_numeric2(int, int);
user_ty *user_symbolic(struct project_ty *, string_ty *);
user_ty *user_executing(struct project_ty *);
void user_free(user_ty *);
user_ty *user_copy(user_ty *);

string_ty *user_name(user_ty *);
string_ty *user_name2(user_ty *);
int user_id(user_ty *);
int user_gid(user_ty *);
int user_umask(user_ty *);

void user_ustate_lock_prepare(user_ty *);

string_ty *user_group(user_ty *);

string_ty *user_home(user_ty *);
string_ty *user_email_address(user_ty *);
string_ty *user_editor_command(user_ty *);
string_ty *user_visual_command(user_ty *);
string_ty *user_pager_command(user_ty *);

const char *user_full_name(string_ty *);

void user_ustate_write(user_ty *);

void user_own_add(user_ty *, struct string_ty *, long);
int user_own_nth(user_ty *, struct string_ty *, long, long *);
void user_own_remove(user_ty *, struct string_ty *, long);

long user_default_change(user_ty *);
string_ty *user_default_project_by_user(user_ty *);
string_ty *user_default_project(void);
string_ty *user_default_development_directory(user_ty *);
string_ty *user_default_project_directory(user_ty *);

int user_uid_check(string_ty *);
int user_gid_check(string_ty *);
void user_become(user_ty *);
void user_become_undo(void);

/**
  * The user_delete_file_query function is used to determine whether a
  * file should be deleted or not.
  *
  * @param up
  *     The user in question
  * @param filename
  *     The name of the file to be deleted
  * @param isdir
  *     whether the file is a directory (true) or a regular file (false).
  * @param default_preference
  *     If the user gave no preference on the command line, use this instead.
  *     1 -> true, 0 -> false, -1 -> look in .aegisrc
  * @returns
  *     true if the file should be deleted, or false if it should not.
  */
bool user_delete_file_query(user_ty *up, string_ty *filename, bool isdir,
    int default_preference);

void user_delete_file_argument(void (*)(void));

int user_diff_preference(user_ty *);
int user_pager_preference(user_ty *);

int user_persevere_preference(user_ty *, int);
void user_persevere_argument(void(*)(void));

uconf_log_file_preference_ty user_log_file_preference(user_ty *,
    uconf_log_file_preference_ty);

void user_lock_wait_argument(void(*)(void));
int user_lock_wait(user_ty *);

void user_whiteout_argument(void(*)(void));
int user_whiteout(user_ty *up, int dflt = -1);

void user_symlink_pref_argument(void(*)(void));
int user_symlink_pref(user_ty *, int);

void user_relative_filename_preference_argument(void(*)(void));
uconf_relative_filename_preference_ty user_relative_filename_preference(
    user_ty *, uconf_relative_filename_preference_ty);

struct output_ty;
void user_uconf_write_xml(user_ty *, struct output_ty *);

/**
  * The user_uconf_get function is used to get the uconf data
  * corresponding to the specified user.  This should be used sparingly,
  * if at all.  It is preferable to use one of the above interfaces if
  * at all possible.
  */
uconf_ty *user_uconf_get(user_ty *);

#endif // AEGIS_USER_H
