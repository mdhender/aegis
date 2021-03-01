//
// aegis - project change supervisor
// Copyright (C) 2001-2008 Peter Miller
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

#include <common/ac/string.h>

#include <common/error.h>
#include <common/fstrcmp.h>
#include <common/itab.h>
#include <common/mem.h>
#include <common/nstring.h>
#include <common/symtab.h>
#include <libaegis/getpw_cache.h>

static symtab_ty *login_table;
static itab_ty *uid_table;


static passwd *
passwd_null(void)
{
    passwd *result = new passwd;

    //
    // This isn't portable, it assumes that NULL pointers are
    // all-bits-zero, but this is only to cover all the fields we
    // don't care about.  We set the ones we -do- care about to
    // NULL explicitly.
    //
    memset(result, 0, sizeof(*result));

#ifndef SOURCE_FORGE_HACK
    result->pw_name = 0;
    result->pw_passwd = 0;
    result->pw_uid = 0;
    result->pw_gid = 0;
    result->pw_gecos = 0;
#ifdef HAVE_pw_comment
    result->pw_comment = 0;
#endif
    result->pw_dir = 0;
    result->pw_shell = 0;
#else
    //
    // This is used to fake a user account, because on SourceForge.net
    // the Apache servers don't have access to /etc/passwd.  The fake
    // information will be further taylored below.
    //
    result->pw_name = "nobody";
    result->pw_passwd = "x";
    result->pw_uid = AEGIS_MIN_UID;
    result->pw_gid = AEGIS_MIN_GID;
    result->pw_gecos = "nobody";
#ifdef HAVE_pw_comment
    result->pw_comment = 0;
#endif
    result->pw_dir = "/home/nobody";
    result->pw_shell = "/bin/sh";
#endif

    //
    // All done.
    //
    return result;
}


static passwd *
passwd_copy(passwd *pw)
{
    passwd *result = passwd_null();

    //
    // Copy values onto the heap, because the next call to getpwnam
    // will trash the ones in *pw.
    //
    result->pw_name = mem_copy_string(pw->pw_name);
    result->pw_uid = pw->pw_uid;
    result->pw_gid = pw->pw_gid;
    result->pw_gecos = mem_copy_string(pw->pw_gecos);
#ifdef HAVE_pw_comment
    result->pw_comment =
        pw->pw_comment ? mem_copy_string(pw->pw_comment) : 0;
#endif
    result->pw_dir = mem_copy_string(pw->pw_dir);
    result->pw_shell = mem_copy_string(pw->pw_shell);

    //
    // All done.
    //
    return result;
}


passwd *
getpwnam_cached(string_ty *name)
{
    return getpwnam_cached(nstring(name));
}


passwd *
getpwnam_cached(const nstring &name)
{
    //
    // Create the tables if they don't exist already.
    //
    if (!login_table)
    {
        login_table = symtab_alloc(5);
        uid_table = itab_alloc();
    }

    //
    // Look for the data in the name table.
    //
    passwd *data = (passwd *)login_table->query(name);

    //
    // If the data isn't there, ask the system for it.
    //
    if (!data)
    {
        passwd *pw = getpwnam(name.c_str());
        if (pw)
        {
            data = passwd_copy(pw);
            itab_assign(uid_table, data->pw_uid, data);
        }
        else
        {
            data = passwd_null();
#ifdef SOURCE_FORGE_HACK
            data->pw_name = mem_copy_string(name.c_str());
            data->pw_gecos = data->pw_name;
#endif
        }
        login_table->assign(name, data);
    }

    //
    // Negative search results are also cached.
    // They have NULL pointers for all the fields.
    //
    if (!data->pw_name)
        return 0;

    //
    // Success.
    //
    return data;
}


passwd *
getpwuid_cached(int uid)
{
    //
    // Create the tables if they don't exist already.
    //
    if (!login_table)
    {
        login_table = symtab_alloc(5);
        uid_table = itab_alloc();
    }

    //
    // Look for the data in the name table.
    //
    passwd *data = (passwd *)itab_query(uid_table, uid);

    //
    // If the data isn't there, ask the system for it.
    //
    if (!data)
    {
        passwd *pw = getpwuid(uid);
        if (pw)
        {
            data = passwd_copy(pw);

            nstring name = pw->pw_name;
            login_table->assign(name, data);
        }
        else
        {
            data = passwd_null();
            data->pw_uid = uid;
        }
        itab_assign(uid_table, uid, data);
    }

    //
    // Negative search results are also cached.
    // They have NULL pointers for all the fields.
    //
    if (!data->pw_name)
        return 0;

    //
    // Success.
    //
    return data;
}


nstring
getpwnam_fuzzy(const nstring &name)
{
    nstring best = 0;
    double best_weight = 0.6;
    setpwent();
    for (;;)
    {
        passwd *pw = getpwent();
        if (!pw)
            break;
        double weight = fstrcmp(name.c_str(), pw->pw_name);
        if (weight > best_weight)
        {
            best = nstring(pw->pw_name);
            best_weight = weight;
        }
    }
    if (best == name)
    {
        error_raw
        (
            "warning: your name service seems to be broken: it can't "
                "find user %s by name (via getpwnam), but this user "
                "is present when scanning through all users (via "
                "getpwent)",
            name.quote_c().c_str()
        );
    }
    return best;
}
