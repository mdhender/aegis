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

#include <common/ac/signal.h>
#include <common/ac/string.h>
#include <common/ac/unistd.h>

#include <common/error.h>
#include <common/safe_strsig.h>
#include <common/trace.h>
#include <libaegis/os.h>
#include <libaegis/os/interrupt.h>
#include <libaegis/sub.h>


int
os_waitpid_status(int child, const char *cmd)
{
    sub_context_ty  *scp;
    int             result;
    RETSIGTYPE      (*hold)(int);
    int             a;
    int             b;
    int             c;
    int             status;

    trace(("os_waitpid_status(child = %d, cmd = \"%s\")\n{\n", child,
	cmd));
    hold = signal(SIGINT, SIG_IGN);
    if (hold != SIG_IGN)
	signal(SIGINT, os_interrupt);
    result = 0;
    if (os_waitpid(child, &status))
	nfatal("wait");
    a = (status >> 8) & 0xFF;
    b = (status >> 7) & 1;
    c = status & 0x7F;
    switch (c)
    {
    case 0x7F:
	//
	// process was stopped,
	// since we didn't do it, treat it as an error
	//
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Command", cmd);
	fatal_intl(scp, i18n("command \"$command\" stopped"));
	// NOTREACHED

    case 0:
	// normal termination
	result = a;
	break;

    default:
	//
	// process dies from unhandled condition
	//
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Command", cmd);
	sub_var_set_charstar(scp, "Signal", safe_strsignal(c));
	if (!b)
	    fatal_intl(scp, i18n("command \"$command\" terminated by $signal"));
	else
	{
	    fatal_intl
	    (
		scp,
		i18n("command \"$command\" terminated by $signal (core dumped)")
	    );
	}
	// NOTREACHED
    }
    if (hold != SIG_IGN)
	signal(SIGINT, hold);
    os_interrupt_cope();
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}
