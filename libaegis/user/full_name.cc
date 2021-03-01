//
// aegis - project change supervisor
// Copyright (C) 2007, 2008, 2012 Peter Miller
// Copyright (C) 2007, 2009 Walter Franzini
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

#include <common/trace.h>
#include <libaegis/getpw_cache.h>
#include <libaegis/user.h>


string_ty *
user_name2(user_ty::pointer up)
{
    trace(("user_name2(up = %p)\n{\n", up.get()));
    static nstring result;
    result = up->full_name();
    trace(("return %s;\n", result.quote_c().c_str()));
    trace(("}\n"));
    return result.get_ref();
}


nstring
user_ty::full_name()
    const
{
    return login_full_name;
}


nstring
user_ty::full_name(const nstring &u_name)
{
    trace(("user_full_name(name = %s)\n{\n", u_name.quote_c().c_str()));
    struct passwd *pw = getpwnam_cached(u_name);
    if (!pw)
    {
        trace(("no such user\n"));
        trace(("}\n"));
        return "";
    }

    nstring result(pw->pw_name);
    if (pw->pw_gecos && pw->pw_gecos[0])
        result = pw->pw_gecos;
#ifdef HAVE_pw_comment
    else if (pw->pw_comment && pw->pw_comment[0])
        result = pw->pw_comment;
#endif

    //
    // Some systems add lots of other stuff to the full name field
    // in the passwd file.  We are only interested in the name.
    //
    const char *comma = strchr(result.c_str(), ',');
    if (comma)
        result = result.substr(0, comma - result.c_str());

    trace(("return %s;\n", result.quote_c().c_str()));
    trace(("}\n"));
    return result;
}


string_ty *
user_full_name(string_ty *u_name)
{
    trace(("user_full_name(name = \"%s\")\n{\n", u_name->str_text));
    nstring result = user_ty::full_name(nstring(u_name));
    trace(("return %s;\n", result.quote_c().c_str()));
    trace(("}\n"));
    return result.get_ref();
}


// vim: set ts=8 sw=4 et :
