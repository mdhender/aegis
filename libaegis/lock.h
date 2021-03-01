//
// aegis - project change supervisor
// Copyright (C) 1991-1993, 1996, 2001, 2002, 2004-2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef LOCK_H
#define LOCK_H

struct string_ty;

typedef void (*lock_callback_ty)(void *);

void lock_prepare_gstate(lock_callback_ty, void *);
void lock_prepare_pstate(struct string_ty *project_name,
    lock_callback_ty, void *);
void lock_prepare_ustate(int uid, lock_callback_ty, void *);
void lock_prepare_ustate_all(lock_callback_ty, void *);
void lock_prepare_cstate(struct string_ty *project_name, long change_number,
    lock_callback_ty, void *);
void lock_take(void);
void lock_release(void);
long lock_magic(void);
void lock_prepare_baseline_read(struct string_ty *project_name,
    lock_callback_ty, void *);
void lock_prepare_baseline_write(struct string_ty *project_name,
    lock_callback_ty, void *);
void lock_prepare_history(struct string_ty *project_name, lock_callback_ty,
    void *);

enum lock_walk_name
{
    lock_walk_name_master,
    lock_walk_name_gstate,
    lock_walk_name_pstate,
    lock_walk_name_cstate,
    lock_walk_name_baseline,
    lock_walk_name_baseline_priority,
    lock_walk_name_history,
    lock_walk_name_ustate,
    lock_walk_name_unknown
};

enum lock_walk_type
{
    lock_walk_type_shared,
    lock_walk_type_exclusive,
    lock_walk_type_unknown
};

struct lock_walk_found
{
    lock_walk_name  name;
    lock_walk_type  type;
    long            subset;
    long            address;
    int             pid;
    int             pid_is_local;
};

typedef void (*lock_walk_callback)(lock_walk_found *);

void lock_walk(lock_walk_callback);
void lock_release_child(void);

/**
  * The lock_quitetr function releases all locks.  This function shall
  * only be called by the quit_action_lock class.
  */
void lock_quitter(void);

/**
  * The lock_active function is used to determine wherther or not a lock
  * is active (i.e. with lock_take/lock_release pair) or not.  Ideally,
  * it will be used in assert statements.
  *
  * @returns
  *     true when between lock_take/lock_release, false if outside.
  */
bool lock_active(void);


#endif // LOCK_H
// vim: set ts=8 sw=4 et :
