//
//	aegis - project change supervisor
//	Copyright (C) 2002, 2004 Peter Miller;
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

#include <error.h> // for assert
#include <ac/signal.h>
#include <ac/unistd.h>

#include <interrupt.h>


static int depth;

#ifndef HAVE_SIGHOLD
#ifdef HAVE_SIGPROCMASK
static sigset_t oldsigs;
#endif
#endif


void
interrupt_disable()
{
    //
    // There are races here, but we shouldn't be nesting, anyway.
    //
    if (++depth > 1)
	return;

#ifdef HAVE_SIGHOLD
    sighold(SIGHUP);
    sighold(SIGINT);
    sighold(SIGQUIT);
    sighold(SIGTERM);
#else
#ifdef HAVE_SIGPROCMASK
    {
	sigset_t        set;

	sigemptyset(&set);
	sigaddset(&set, SIGHUP);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGQUIT);
	sigaddset(&set, SIGTERM);
	sigprocmask(SIG_BLOCK, &set, &oldsigs);
    }
#endif // HAVE_SIGPROCMASK
#endif // !HAVE_SIGHOLD
}


void
interrupt_enable()
{
    //
    // There are races here, but we shouldn't be nesting, anyway.
    //
    assert(depth > 0);
    if (depth <= 0)
	return;
    if (--depth > 0)
	return;

#ifdef HAVE_SIGHOLD
    sigrelse(SIGHUP);
    sigrelse(SIGINT);
    sigrelse(SIGQUIT);
    sigrelse(SIGTERM);
#else
#ifdef HAVE_SIGPROCMASK
    sigprocmask(SIG_SETMASK, &oldsigs, (sigset_t *)0);
#endif // HAVE_SIGPROCMASK
#endif // !HAVE_SIGHOLD
}
