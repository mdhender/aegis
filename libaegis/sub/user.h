//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001, 2002, 2005-2008 Peter Miller
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

#ifndef LIBAEGIS_SUB_USER_H
#define LIBAEGIS_SUB_USER_H

#include <common/main.h>
#include <libaegis/user.h>

struct wstring_ty; // existence
struct sub_context_ty; // existence
struct wstring_list_ty; // existence
struct string_ty; // forward

wstring sub_user(sub_context_ty *, const wstring_list &arg);

//
// Looks for a user function, by name.  Used by ${developer},
// ${developers}, ${reviewer}, ${reviewers}, ${integrator},
// ${integrators}, ${administrator}, ${administrators} and ${user}.
//
typedef nstring (*sub_user_func_ptr)(user_ty::pointer);

sub_user_func_ptr sub_user_func(const nstring &name);

#endif // LIBAEGIS_SUB_USER_H
