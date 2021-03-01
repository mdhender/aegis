/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2001, 2002 Peter Miller;
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
 * MANIFEST: interface definition for libaegis/sub/user.c
 */

#ifndef LIBAEGIS_SUB_USER_H
#define LIBAEGIS_SUB_USER_H

#include <main.h>

struct wstring_ty; /* existence */
struct sub_context_ty; /* existence */
struct wstring_list_ty; /* existence */

struct wstring_ty *sub_user(struct sub_context_ty *,
	struct wstring_list_ty *arg);

/*
 * Looks for a user function, by name.  Used by ${developer},
 * ${developers}, ${reviewer}, ${reviewers}, ${integrator},
 * ${integrators}, ${administrator}, ${administrators} and ${user}.
 */
struct user_ty; /* forward */
struct string_ty; /* forward */
typedef struct string_ty *(*sub_user_func_ptr)(struct user_ty *);
sub_user_func_ptr sub_user_func(struct string_ty *);

#endif /* LIBAEGIS_SUB_USER_H */
