//
//      aegis - project change supervisor
//      Copyright (C) 2007, 2008, 2012 Peter Miller
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

#include <common/ac/string.h>

#include <common/trace.h>
#include <libaegis/getgr_cache.h>
#include <libaegis/getpw_cache.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>


user_ty::pointer
user_ty::create(int uid)
{
    //
    // see if we already know her
    //
    trace(("user_ty::create(uid = %d)\n{\n", uid));
    user_ty::pointer old = pool_find(uid);
    if (old)
    {
        trace(("return %p;\n", old.get()));
        trace(("}\n"));
        return old;
    }

    //
    // first time we have met her
    //
    user_ty::pointer up(new user_ty(uid));
    pool_add(up);
    trace(("return %p;\n", up.get()));
    trace(("}\n"));
    return up;
}


user_ty::user_ty(int a_uid) :
    user_id(a_uid),
    group_id(0),
    umask(022),
    ustate_data(0),
    ustate_is_new(false),
    ustate_modified(false),
    uconf_data(0),
    lock_magic(0)
{
    trace(("user_ty::user_ty(a_uid = %d)\n{\n", a_uid));

    //
    // Always use the UID as the primary reference:
    // the system treats the first entry as cannonical, so do we.
    //
    struct passwd *pw = getpwuid_cached(user_id);
    if (!pw)
    {
        sub_context_ty sc;
        sc.var_set_long("Number", user_id);
        sc.fatal_intl(i18n("uid $number unknown"));
        // NOTREACHED
    }
    group_id = pw->pw_gid;
    login_name = pw->pw_name;
    home = pw->pw_dir;

    char *fnm = pw->pw_name;
    if (pw->pw_gecos && pw->pw_gecos[0])
        fnm = pw->pw_gecos;
#ifdef HAVE_pw_comment
    else if (pw->pw_comment && pw->pw_comment[0])
        fnm = pw->pw_comment;
#endif

    //
    // Some systems add lots of other stuff to the full name field
    // in the passwd file.  We are only interested in the name.
    //
    const char *comma = strchr(fnm, ',');
    if (comma)
        login_full_name = nstring(fnm, comma - fnm);
    else
        login_full_name = fnm;

    struct group *gr = getgrgid_cached(group_id);
    if (!gr)
    {
        sub_context_ty sc;
        sc.var_set_long("Number", group_id);
        sc.fatal_intl(i18n("gid $number unknown"));
        // NOTREACHED
    }
    group_name = gr->gr_name;

    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
