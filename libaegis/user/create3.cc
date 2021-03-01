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

#include <common/error.h>
#include <common/trace.h>
#include <libaegis/getpw_cache.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>


user_ty::pointer
user_symbolic(string_ty *name)
{
    return user_ty::create(nstring(name));
}


user_ty::pointer
user_ty::create(const nstring &u_name)
{
    trace(("user_ty::create(name = %s)\n{\n", u_name.quote_c().c_str()));

    struct passwd *pw = getpwnam_cached(u_name);
    if (!pw)
    {
        nstring best = getpwnam_fuzzy(u_name);
	if (!best.empty())
	{
            if (u_name == best)
            {
                error_raw
                (
                    "Your name service seems to be broken: it can't "
                    "find user %s by name (via getpwnam), but this "
                    "user is present when scanning through all users "
                    "(via getpwent).  This is not an Aegis bug, please "
                    "do not report it as such.  You network sysadmin "
                    "should be informed.",
                    u_name.quote_c().c_str()
                );
            }

	    sub_context_ty sc;
	    sc.var_set_string("Name", u_name);
	    sc.var_set_string("Guess", best);
	    sc.fatal_intl(i18n("user $name unknown, closest is $guess"));
	    // NOTREACHED
	}
	else
	{
	    sub_context_ty sc;
	    sc.var_set_string("Name", u_name);
	    sc.fatal_intl(i18n("user $name unknown"));
	    // NOTREACHED
	}
    }

    user_ty::pointer result = user_ty::create(pw->pw_uid);
    trace(("return %08lX;\n", (long)result.get()));
    trace(("}\n"));
    return result;
}
