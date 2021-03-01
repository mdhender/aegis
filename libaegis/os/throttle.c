/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-2002 Peter Miller;
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
 * MANIFEST: wrappers around operating system functions
 */

#include <ac/stdlib.h>
#include <ac/unistd.h>

#include <os.h>
#include <sub.h>


/*
 * NAME
 *	os_throttle
 *
 * SYNOPSIS
 *	void os_throttle(void);
 *
 * DESCRIPTION
 *	This unlikely function is used to slow aegis down.  it is
 *	primarily used for aegis' own tests, to ensure that the time
 *	stamps are kosher even on ultra-fast machines.  It is also
 *	useful in shell scripts, e.g. automatic integration queue
 *	handling.
 */

void
os_throttle()
{
    static int      nsecs;

    if (nsecs == 0)
    {
	char            *cp;

	cp = getenv("AEGIS_THROTTLE");
	if (!cp)
	    nsecs = -1;
	else
	{
	    nsecs = atoi(cp);
	    if (nsecs <= 0)
		nsecs = 1;
	    else if (nsecs > 5)
		nsecs = 5;
	}
    }
    if (nsecs > 0)
    {
	sync();
#ifdef DEBUG
	{
	    sub_context_ty *scp;

	    scp = sub_context_new();
	    sub_var_set_long(scp, "Number", (long)nsecs);
	    error_intl(scp, i18n("throttling $number seconds"));
	    sub_context_delete(scp);
	}
#endif
	sleep(nsecs);
    }
}
