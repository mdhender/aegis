/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate backgrounds
 */

#include <ac/unistd.h>
#include <ac/termios.h>
#include <ac/signal.h>

#include <os.h>


#ifdef SIGSTOP
#ifndef HAVE_TCGETPGRP

#include <sys/termio.h>

int
tcgetpgrp(fd)
    int             fd;
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

#endif /* !HAVE_TCGETPGRP */
#endif /* SIGSTOP */


/*
 *  NAME
 *	  background - test for backgroundness
 *
 *  SYNOPSIS
 *	  int background(void);
 *
 *  DESCRIPTION
 *	  The background function is used to determin e if the curent process is
 *	  in the background.
 *
 *  RETURNS
 *	  int: zero if process is not in the background, nonzero if the process
 *	  is in the background.
 *
 * CAVEAT:
 *	This function has a huge chance of being wrong for your system.
 *	If you need to modify this function, please let the author know.
 */

int
os_background()
{
    RETSIGTYPE      (*x)_((int));

    /*
     * C shell and Bash
     *      puts its children in a different process group.
     *      The process group the terminal is in is the forground.
     *
     * Only available on systems with job control.
     */
#ifdef SIGSTOP
    int             stdout_process_group = tcgetpgrp(0);
    if (stdout_process_group < 0)
    {
	/*
	 * The standard input doesn't have a process group.
	 * This means the input is coming from a pipe or a file
	 * or something.  We aren't in the background, or if we
	 * are it doesn't matter.
	 */
	return 0;
    }
    if (getpgrp(CONF_getpgrp_arg) != stdout_process_group)
	return 1;
#endif

    /*
     * Bourne shell
     *      sets its children to ignore SIGINT
     */
    x = signal(SIGINT, SIG_IGN);
    if (x == SIG_IGN)
	return 1;
    signal(SIGINT, x);

    /*
     * There are reports that Ksh does something else,
     * and this function is frequently wrong.
     * Anybody out there use Ksh and know what to do?
     */

    /*
     * probably forground
     */
    return 0;
}
