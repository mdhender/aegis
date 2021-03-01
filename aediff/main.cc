//
// aegis - project change supervisor
// Copyright (C) 2004-2008, 2012 Peter Miller
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

#include <common/ac/stdio.h>
#include <common/ac/signal.h>

#include <common/env.h>
#include <common/language.h>
#include <common/quit.h>
#include <common/rsrc_limits.h>
#include <common/sizeof.h>
#include <common/str.h>
#include <libaegis/help.h>
#include <libaegis/os.h>
#include <libaegis/version.h>

#include <aediff/arglex3.h>
#include <aediff/diff.h>


static void
main_help(void)
{
    help(0, diff_usage);
}


int
main(int argc, char **argv)
{
    static arglex_dispatch_ty dispatch[] =
    {
        { arglex_token_help, main_help, 0 },
        { arglex_token_version, version, 0 },
    };

    //
    // Some versions of cron(8) set SIGCHLD to SIG_IGN.  This is
    // kinda dumb, because it breaks assumptions made in libc (like
    // pclose, for instance).  It also blows away most of Cook's
    // process handling.  We explicitly set the SIGCHLD signal
    // handling to SIG_DFL to make sure this signal does what we
    // expect no matter how we are invoked.
    //
#ifdef SIGCHLD
    signal(SIGCHLD, SIG_DFL);
#else
    signal(SIGCLD, SIG_DFL);
#endif

    resource_limits_init();
    arglex3_init(argc, argv);
    env_initialize();
    language_init();
    os_become_init_mortal();
    arglex_dispatch(dispatch, SIZEOF(dispatch), diff);
    quit(0);
    return 0;
}


// vim: set ts=8 sw=4 et :
