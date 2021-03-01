/*
 *	aegis - project change supervisor
 *	Copyright (C) 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000 Peter Miller;
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
 * MANIFEST: interface definition for aegis/user.c
 */

#ifndef AEGIS_USER_H
#define AEGIS_USER_H

#include <ustate.h>
#include <uconf.h>

typedef struct user_ty user_ty;
struct user_ty
{
	long		reference_count;
	struct project_ty *pp;
	string_ty	*name;
	string_ty	*full_name;
	string_ty	*home;
	string_ty	*group;
	int		uid;
	int		gid;
	int		umask;
	string_ty	*ustate_path;
	ustate		ustate_data;
	int		ustate_is_new;
	int		ustate_modified;
	string_ty	*uconf_path;
	uconf		uconf_data;
	long		lock_magic;
	/*
	 * if you add anything to this structure,
	 * make sure you fix user_free in user.c
	 */
};


user_ty *user_numeric _((struct project_ty *, int));
user_ty *user_numeric2 _((int, int));
user_ty *user_symbolic _((struct project_ty *, string_ty *));
user_ty *user_executing _((struct project_ty *));
void user_free _((user_ty *));
user_ty *user_copy _((user_ty *));

string_ty *user_name _((user_ty *));
string_ty *user_name2 _((user_ty *));
int user_id _((user_ty *));
int user_gid _((user_ty *));
int user_umask _((user_ty *));

void user_ustate_lock_prepare _((user_ty *));

string_ty *user_group _((user_ty *));

string_ty *user_home _((user_ty *));
string_ty *user_email_address _((user_ty *));

char *user_full_name _((string_ty *));

void user_ustate_write _((user_ty *));

void user_own_add _((user_ty *, struct string_ty *, long));
int user_own_nth _((user_ty *, long, long *));
void user_own_remove _((user_ty *, struct string_ty *, long));

long user_default_change _((user_ty *));
string_ty *user_default_project _((void));
string_ty *user_default_development_directory _((user_ty *));
string_ty *user_default_project_directory _((user_ty *));

int user_uid_check _((string_ty *));
int user_gid_check _((string_ty *));
void user_become _((user_ty *));
void user_become_undo _((void));

int user_delete_file_query _((user_ty *, string_ty *, int));
void user_delete_file_argument _((void (*)(void)));

int user_diff_preference _((user_ty *));
int user_pager_preference _((user_ty *));

int user_persevere_preference _((user_ty *, int));
void user_persevere_argument _((void(*)(void)));

uconf_log_file_preference_ty user_log_file_preference _((user_ty *,
	uconf_log_file_preference_ty));

void user_lock_wait_argument _((void(*)(void)));
int user_lock_wait _((user_ty *));

void user_whiteout_argument _((void(*)(void)));
int user_whiteout _((user_ty *));

void user_symlink_pref_argument _((void(*)(void)));
int user_symlink_pref _((user_ty *, int));

void user_relative_filename_preference_argument _((void(*)(void)));
uconf_relative_filename_preference_ty user_relative_filename_preference _((
	user_ty *, uconf_relative_filename_preference_ty));

#endif /* AEGIS_USER_H */
