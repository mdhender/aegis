//
//	aegis - project change supervisor
//	Copyright (C) 2002-2008 Peter Miller
//	Copyright (C) 2006 Walter Franzini
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

#include <common/ac/execinfo.h>
#include <common/ac/signal.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>
#include <common/ac/unistd.h>

#include <common/error.h>
#include <common/safe_strsig.h>
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
    sub_var_set_charstar(scp, "Signal", safe_strsignal(n));
    fatal_intl(scp, i18n("interrupted by $signal"));
    // NOTREACHED
    sub_context_delete(scp);
}

#ifdef HAVE_BACKTRACE

RETSIGTYPE
os_backtrace(int n)
{
    signal(n, SIG_IGN);
    if (interrupted)
        return;
    interrupted = 1;

#define BT_MAX_DEPTH 20

    void **bt_info;
    bt_info = (void**)calloc(BT_MAX_DEPTH, sizeof(void*));
    int bt_depth = backtrace(bt_info, BT_MAX_DEPTH);
    char **symbol = backtrace_symbols(bt_info, bt_depth);
    for (int j = 0; j < bt_depth; ++j)
    {
        error_raw("%d: %s", j, symbol[j]);
    }
    signal(n, SIG_DFL);
    kill(0, n);
}

#endif

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

#ifdef HAVE_BACKTRACE
    if (signal(SIGSEGV, SIG_IGN) != SIG_IGN)
        signal(SIGSEGV, os_backtrace);
    if (signal(SIGBUS, SIG_IGN) != SIG_IGN)
        signal(SIGBUS, os_backtrace);
    if (signal(SIGABRT, SIG_IGN) != SIG_IGN)
        signal(SIGABRT, os_backtrace);
#endif
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
