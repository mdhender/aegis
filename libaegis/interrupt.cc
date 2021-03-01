//
// aegis - project change supervisor
// Copyright (C) 2002, 2004-2006, 2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>
#include <common/ac/signal.h>
#include <common/ac/unistd.h>

#include <libaegis/interrupt.h>


static int depth;

#if !defined (HAVE_SIGHOLD) || !defined (HAVE_SIGRELSE)
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

#if defined (HAVE_SIGHOLD) && defined (HAVE_SIGRELSE)
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

#if defined (HAVE_SIGHOLD) && defined (HAVE_SIGRELSE)
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


// vim: set ts=8 sw=4 et :
