/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate users
 */

#include <ac/pwd.h>

#include <complete/private.h>
#include <complete/user.h>
#include <project.h>
#include <shell.h>


typedef struct complete_user_ty complete_user_ty;
struct complete_user_ty
{
    complete_ty     inherited;
    project_ty      *pp;
    complete_user_func func;
};


static void destructor _((complete_ty *));

static void
destructor(cp)
    complete_ty     *cp;
{
    complete_user_ty *this;

    this = (complete_user_ty *)cp;
    project_free(this->pp);
}


static void perform _((complete_ty *, shell_ty *));

static void
perform(cp, sh)
    complete_ty     *cp;
    shell_ty        *sh;
{
    complete_user_ty *this;
    string_ty       *prefix;
    string_ty       *name;
    struct passwd   *pw;

    this = (complete_user_ty *)cp;
    prefix = shell_prefix_get(sh);
    setpwent();
    for (;;)
    {
	pw = getpwent();
	if (!pw)
	    break;
	if (pw->pw_uid < AEGIS_MIN_UID)
	    continue;
	name = str_from_c(pw->pw_name);
	if
	(
	    str_leading_prefix(name, prefix)
	&&
	    (!this->func || this->func(this->pp, name))
	)
	{
	    shell_emit(sh, name);
	}
	str_free(name);
    }
    endpwent();
}


static complete_vtbl_ty vtbl =
{
    destructor,
    perform,
    sizeof(complete_user_ty),
    "user",
};


complete_ty *
complete_user(pp, func)
    project_ty      *pp;
    complete_user_func func;
{
    complete_ty     *result;
    complete_user_ty *this;

    result = complete_new(&vtbl);
    this = (complete_user_ty *)result;
    this->pp = pp;
    this->func = func;
    return result;
}
