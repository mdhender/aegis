//
//	aegis - project change supervisor
//	Copyright (C) 2007, 2008 Peter Miller
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

#include <common/ac/string.h>

#include <common/trace.h>
#include <libaegis/getgr_cache.h>
#include <libaegis/getpw_cache.h>
#include <libaegis/user.h>


user_ty::pointer
user_ty::create(int uid, int gid)
{
    trace(("user_ty::create(uid = %d, gid = %d)\n{\n", uid, gid));

    user_ty::pointer old = pool_find(uid);
    if (old)
    {
        trace(("return %08lX\n", (long)old.get()));
        trace(("}\n"));
        return old;
    }

    user_ty::pointer up(new user_ty(uid, gid));
    pool_add(up);
    trace(("return %08lX;\n", (long)up.get()));
    trace(("}\n"));
    return up;
}


user_ty::user_ty(int uid, int gid) :
    user_id(uid),
    group_id(gid),
    umask(022),
    ustate_data(0),
    ustate_is_new(false),
    ustate_modified(false),
    uconf_data(0),
    lock_magic(0)
{
    trace(("user_ty::user_ty(uid = %d, gid = %d)\n{\n", uid, gid));

    //
    // try the passwd file,
    // otherwise default to something sensable
    //
    struct passwd *pw = getpwuid_cached(user_id);
    if (pw)
    {
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
    }
    else
    {
	login_name = nstring::format("uid %d", user_id);
	home = "/";
        login_full_name = login_name;
    }

    //
    // try the group file,
    // otherwise default to something sensable
    //
    struct group *gr = getgrgid_cached(group_id);
    if (!gr)
	group_name = nstring::format("gid %d", group_id);
    else
	group_name = gr->gr_name;

    trace(("}\n"));
}
