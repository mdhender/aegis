//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2008 Peter Miller
//	Copyright (C) 2007 Walter Franzini
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
//	along with this program, If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/stdlib.h>
#include <common/ac/signal.h>

#include <common/env.h>
#include <common/language.h>
#include <common/quit.h>
#include <common/rsrc_limits.h>
#include <libaegis/help.h>
#include <libaegis/os.h>
#include <libaegis/version.h>

#include <aedist/archive.h>
#include <aedist/arglex3.h>
#include <aedist/inventory.h>
#include <aedist/list.h>
#include <aedist/missing.h>
#include <aedist/pending.h>
#include <aedist/receive.h>
#include <aedist/replay.h>
#include <aedist/send.h>
#include <aedist/usage.h>


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
        { arglex_token_help, aedist_help, 1 },
	{ arglex_token_archive, archive_main, 0 },
        { arglex_token_inventory, inventory_main, 0 },
        { arglex_token_list, list_main, 1 },
        { arglex_token_missing, missing_main, 0 },
        { arglex_token_pending, pending_main, 0 },
        { arglex_token_receive, receive_main, 0 },
        { arglex_token_replay, replay_main, 0 },
        { arglex_token_send, send_main, 0 },
        { arglex_token_version, version, 0 },
    };
    arglex_dispatch(dispatch, SIZEOF(dispatch), usage);

    quit(0);
    return 0;
}
