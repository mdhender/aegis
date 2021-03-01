//
//	aegis - project change supervisor
//	Copyright (C) 2002-2008 Peter Miller
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

#include <common/ac/ctype.h>
#include <common/ac/string.h>

#include <common/trace.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>

#define MAX_CMD_RPT 36


static nstring
extract_command_name(const nstring &s, bool abbrev)
{
    const char *cp = s.c_str();
    for (;;)
    {
        unsigned char c = *cp;
        if (!c)
            return s;
        if (isspace(c))
        {
            ++cp;
            continue;
        }
        const char *ep = cp + 1;
        for (;;)
        {
            c = *ep;
            if (!c || isspace(c) || strchr("<>|&;^", c))
                break;
            ++ep;
        }
        if (0 == memchr(cp, '=', ep - cp))
        {
            if (!abbrev || ispunct((unsigned char)*cp))
                return nstring(cp, s.c_str() + s.size() - cp);
            return nstring(cp, ep - cp);
        }
        cp = ep;
    }
}


void
os_execute(string_ty *cmd, int flags, string_ty *dir)
{
    trace(("os_execute()\n{\n"));
    int result = os_execute_retcode(cmd, flags, dir);
    if (result)
    {
        bool abbrev = !(flags & OS_EXEC_FLAG_SILENT);
        nstring cname = extract_command_name(nstring(cmd), abbrev);
	if (abbrev && cname.size() > MAX_CMD_RPT)
	    cname = nstring::format("%.*s...", MAX_CMD_RPT - 3, cname.c_str());
	sub_context_ty sc;
	sc.var_set_string("Command", cname);
	sc.var_set_long("Number", result);
	sc.fatal_intl(i18n("command \"$command\" exit status $number"));
	// NOTREACHED
    }
    trace(("}\n"));
}


void
os_execute(const nstring &cmd, int flags, const nstring &dir)
{
    os_execute(cmd.get_ref(), flags, dir.get_ref());
}
