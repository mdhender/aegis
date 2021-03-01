//
//	aegis - project change supervisor
//	Copyright (C) 2002-2005 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate interrupts
//

#include <common/ac/signal.h>
#include <common/ac/string.h>
#include <common/ac/unistd.h>

#include <libaegis/os.h>
#include <libaegis/os/interrupt.h>
#include <libaegis/sub.h>


static int      interrupted;


RETSIGTYPE
os_interrupt(int n)
{
    sub_context_ty  *scp;

    signal(n, SIG_IGN);
    if (interrupted)
	return;
    interrupted = 1;
    scp = sub_context_new();
    sub_var_set_charstar(scp, "Signal", strsignal(n));
    fatal_intl(scp, i18n("interrupted by $signal"));
    // NOTREACHED
    sub_context_delete(scp);
}


void
os_interrupt_register(void)
{
    if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
	signal(SIGHUP, os_interrupt);
    if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
	signal(SIGQUIT, os_interrupt);
    if (signal(SIGINT, SIG_IGN) != SIG_IGN)
	signal(SIGINT, os_interrupt);
    if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
	signal(SIGTERM, os_interrupt);
}


void
os_interrupt_ignore(void)
{
    signal(SIGHUP, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
}


void
os_interrupt_cope(void)
{
}


int
os_interrupt_has_occurred(void)
{
    return (interrupted != 0);
}
