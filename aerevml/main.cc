//
//	aegis - project change supervisor
//	Copyright (C) 2005-2007 Peter Miller
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
// MANIFEST: operating system entry point
//

#include <common/ac/signal.h>

#include <aerevml/arglex3.h>
#include <aerevml/send.h>
#include <aerevml/usage.h>
#include <common/env.h>
#include <common/language.h>
#include <common/quit.h>
#include <common/rsrc_limits.h>
#include <libaegis/help.h>
#include <libaegis/os.h>
#include <libaegis/version.h>


static void
revml_help(void)
{
    help(0, usage);
}


int
main(int argc, char **argv)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{ arglex_token_send, revml_send, 0 },
	// { arglex_token_list, revml_list, 0 },
	{ arglex_token_help, revml_help, 0 },
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
    arglex_dispatch(dispatch, SIZEOF(dispatch), usage);
    quit(0);
    return 0;
}
