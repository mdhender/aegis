//
//	aegis - project change supervisor
//	Copyright (C) 2002, 2004-2006, 2008 Peter Miller
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

#include <common/ac/unistd.h>
#include <common/ac/sys/ioctl.h>
#include <common/ac/termios.h>
#include <common/ac/signal.h>

#include <libaegis/os.h>


#ifdef SIGSTOP
#ifndef HAVE_TCGETPGRP

#include <sys/termio.h>

int
tcgetpgrp(int fd)
{
    int             result;

#ifdef TIOCGETPGRP
    if (ioctl(fd, TIOCGETPGRP, &result))
	result = -1;
#else
#ifdef TIOCGPGRP
    if (ioctl(fd, TIOCGPGRP, &result))
	result = -1;
#else
    result = -1;
#endif
#endif
    return result;
}

#endif // !HAVE_TCGETPGRP
#endif // SIGSTOP


int
os_background(void)
{
    RETSIGTYPE      (*x)(int);

    //
    // C shell and Bash
    //      puts its children in a different process group.
    //      The process group the terminal is in is the forground.
    //
    // Only available on systems with job control.
    //
#ifdef SIGSTOP
    int             stdout_process_group;

    stdout_process_group = tcgetpgrp(0);
    if (stdout_process_group < 0)
    {
	//
	// The standard input doesn't have a process group.
	// This means the input is coming from a pipe or a file
	// or something.  We aren't in the background, or if we
	// are it doesn't matter.
	//
	return 0;
    }
    if (getpgrp(CONF_getpgrp_arg) != stdout_process_group)
	return 1;
#endif

    //
    // Bourne shell
    //      sets its children to ignore SIGINT
    //
    x = signal(SIGINT, SIG_IGN);
    if (x == SIG_IGN)
	return 1;
    signal(SIGINT, x);

    //
    // There are reports that Ksh does something else,
    // and this function is frequently wrong.
    // Anybody out there use Ksh and know what to do?
    //

    //
    // probably foreground
    //
    return 0;
}
