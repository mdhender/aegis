//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2005 Peter Miller;
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
// MANIFEST: functions to manipulate mains
//

#include <ac/stdlib.h>
#include <ac/signal.h>

#include <arglex3.h>
#include <env.h>
#include <help.h>
#include <language.h>
#include <list.h>
#include <missing.h>
#include <os.h>
#include <pending.h>
#include <quit.h>
#include <receive.h>
#include <replay.h>
#include <rsrc_limits.h>
#include <send.h>
#include <usage.h>
#include <version.h>


static void
aedist_help()
{
    help(0, usage);
}


int
main(int argc, char **argv)
{
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

    static arglex_dispatch_ty dispatch[] =
    {
        { arglex_token_send, send_main },
        { arglex_token_receive, receive_main },
        { arglex_token_missing, missing_main },
        { arglex_token_pending, pending_main },
        { arglex_token_replay, replay_main },
        { arglex_token_version, version },
        { arglex_token_help, aedist_help },
        { arglex_token_list, list_main },
    };
    arglex_dispatch(dispatch, SIZEOF(dispatch), usage);

    quit(0);
    return 0;
}
