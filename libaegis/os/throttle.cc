//
// aegis - project change supervisor
// Copyright (C) 1991-2002, 2004-2008, 2012 Peter Miller
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

#include <common/ac/stdlib.h>
#include <common/ac/unistd.h>

#include <common/debug.h>
#include <common/now.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>


static int nsecs;
static bool nsecs_set;


static void
slurp(void)
{
    if (nsecs_set)
        return;
    nsecs_set = true;

    const char *cp = getenv("AEGIS_THROTTLE");
    if (!cp)
    {
        //
        // If not set, default to zero
        //
        nsecs = 0;
    }
    else
    {
        char *ep = 0;
        nsecs = strtol(cp, &ep, 10);

        //
        // If it isn't a valid number, default to zero.
        //
        if (cp == ep || *ep)
            nsecs = 0;
    }
}


void
os_throttle(void)
{
    slurp();
    if (nsecs > 0)
    {
        sync();
#ifdef DEBUG
        sub_context_ty sc;
        sc.var_set_long("Number", (long)nsecs);
        sc.error_intl(i18n("throttling $number seconds"));
#endif
        sleep(nsecs > 5 ? 5 : nsecs);
        now_clear();
    }
}


bool
os_unthrottle(void)
{
    slurp();
    return (nsecs < 0);
}


// vim: set ts=8 sw=4 et :
