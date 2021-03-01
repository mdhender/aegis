//
//      aegis - project change supervisor
//      Copyright (C) 2002-2006, 2008, 2011, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/pwd.h>

#include <aecomplete/complete/private.h>
#include <aecomplete/complete/user.h>
#include <libaegis/project.h>
#include <aecomplete/shell.h>


struct complete_user_ty
{
    complete_ty     inherited;
    project      *pp;
    complete_user_func func;
};


static void
destructor(complete_ty *cp)
{
    complete_user_ty *this_thing;

    this_thing = (complete_user_ty *)cp;
    project_free(this_thing->pp);
}


static void
perform(complete_ty *cp, shell_ty *sh)
{
    complete_user_ty *this_thing;
    string_ty       *prefix;
    string_ty       *name;
    struct passwd   *pw;

    this_thing = (complete_user_ty *)cp;
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
            (!this_thing->func || this_thing->func(this_thing->pp, name))
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
complete_user(project *pp, complete_user_func func)
{
    complete_ty     *result;
    complete_user_ty *this_thing;

    result = complete_new(&vtbl);
    this_thing = (complete_user_ty *)result;
    this_thing->pp = pp;
    this_thing->func = func;
    return result;
}


// vim: set ts=8 sw=4 et :
