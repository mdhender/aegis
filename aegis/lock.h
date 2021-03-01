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
 * MANIFEST: interface definition for aegis/lock.c
 */

#ifndef LOCK_H
#define LOCK_H

#include <main.h>

struct string_ty;

void lock_prepare_gstate _((void));
void lock_prepare_pstate _((struct string_ty *project_name));
void lock_prepare_ustate _((int uid));
void lock_prepare_ustate_all _((void));
void lock_prepare_cstate _((struct string_ty *project_name, long change_number));
void lock_take _((void));
void lock_release _((void));
long lock_magic _((void));
void lock_prepare_build_read _((struct string_ty *project_name));
void lock_prepare_build_write _((struct string_ty *project_name));

#endif /* LOCK_H */
