/*
 *	aegis - project change supervisor
 *	Copyright (C) 1992, 1993 Peter Miller.
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
 * MANIFEST: interface definition for aegis/user.c
 */

#ifndef USER_H
#define USER_H

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


user_ty *user_numeric _((struct project_ty *pp, int uid));
user_ty *user_symbolic _((struct project_ty *pp, string_ty *user_name));
user_ty *user_executing _((struct project_ty *));
void user_free _((user_ty *));
user_ty *user_copy _((user_ty *));

string_ty *user_name _((user_ty *));
int user_id _((user_ty *));
int user_gid _((user_ty *));
int user_umask _((user_ty *));

void user_ustate_lock_prepare _((user_ty *));

string_ty *user_group _((user_ty *));

string_ty *user_home _((user_ty *));

char *user_full_name _((string_ty *));

void user_ustate_write _((user_ty *));

void user_own_add _((user_ty *up, struct string_ty *project_name,
	long change_number));
int user_own_nth _((user_ty *up, long n, long *change_number));
void user_own_remove _((user_ty *, struct string_ty *project_name,
	long change_number));

long user_default_change _((user_ty *));
string_ty *user_default_project _((void));
string_ty *user_default_development_directory _((user_ty *));
string_ty *user_default_project_directory _((user_ty *));

int user_uid_check _((string_ty *));
int user_gid_check _((string_ty *));
void user_become _((user_ty *));
void user_become_undo _((void));

#endif /* USER_H */
