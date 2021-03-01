//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
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

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>

#include <libaegis/arglex2.h>
#include <common/env.h>
#include <common/error.h>
#include <libaegis/help.h>
#include <common/language.h>
#include <aecvsserver/net.h>
#include <libaegis/os.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/rsrc_limits.h>
#include <aecvsserver/server/kerberos.h>
#include <aecvsserver/server/password.h>
#include <aecvsserver/server/simple.h>
#include <libaegis/version.h>


enum
{
    arglex_token_server = ARGLEX2_MAX,
    arglex_token_kserver,
    arglex_token_pserver
};

static arglex_table_ty argtab[] =
{
    { "_SERVER", arglex_token_server },
    { "_Kerberos_SERVER", arglex_token_kserver },
    { "_Password_SERVER", arglex_token_pserver },
    ARGLEX_END_MARKER
};


static void
usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf(stderr, "usage: %s [ <option>... ][ <filename>... ]\n", progname);
    fprintf(stderr, "       %s -Help\n", progname);
    quit(1);
}


static void
main_help(void)
{
    help((char *)0, usage);
}


int
main(int argc, char **argv)
{
    server_ty       *serve;

    resource_limits_init();
    os_become_init_mortal();
    arglex2_init3(argc, argv, argtab);
    env_initialize();
    language_init();

    switch (arglex())
    {
    case arglex_token_help:
	main_help();
	quit(0);

    case arglex_token_version:
	version();
	quit(0);
    }

    //
    // cvsclient.texi, section 3, How to Connect to and Authenticate
    // Oneself to the CVS server: "Connection and authentication occurs
    // before the CVS protocol itself is started.  There are several
    // ways to connect."
    //
    serve = 0;
    net_ty *np = new net_ty();
#if 1 // def DEBUG
    np->log_by_env("AELOG");
#endif
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(usage);
	    continue;

	case arglex_token_server:
	    //
	    // cvsclient.texi: "If the client has a way to execute
	    // commands on the server, and provide input to the commands
	    // and output from them, then it can connect that way.
	    // This could be the usual rsh (port 514) protocol, Kerberos
	    // rsh, SSH, or any similar mechanism.  The client may allow
	    // the user to specify the name of the server program; the
	    // default is cvs.	    It is invoked with one argument,
	    // server.	Once it invokes the server, the client proceeds
	    // to start the cvs protocol."
	    //
	    if (serve)
	    {
		too_many:
		error_raw("too many server types specified");
		usage();
		// NOTREACHED
	    }
	    serve = server_simple_new(np);
	    break;

	case arglex_token_kserver:
	    //
	    // cvsclient.texi: "The kerberized server listens on a port
	    // (in the current implementation, by having inetd call "cvs
	    // kserver") which defaults to 1999.  The client connects,
	    // sends the usual kerberos authentication information,
	    // and then starts the cvs protocol.  Note: port 1999 is
	    // officially registered for another use, and in any event
	    // one cannot register more than one port for CVS, so GSS-API
	    // (see below) is recommended instead of kserver as a way
	    // to support kerberos."
	    //
	    if (serve)
		goto too_many;
	    serve = server_kerberos_new(np);
	    break;

	case arglex_token_pserver:
	    //
	    // cvsclient.texi: "The name pserver is somewhat confusing.
	    // It refers to both a generic framework which allows the CVS
	    // protocol to support several authentication mechanisms, and
	    // a name for a specific mechanism which transfers a username
	    // and a cleartext password.  Servers need not support all
	    // mechanisms, and in fact servers will typically want to
	    // support only those mechanisms which meet the relevant
	    // security needs.
	    //
	    // "The pserver server listens on a port (in the current
	    // implementation, by having inetd call "cvs pserver") which
	    // defaults to 2401 (this port is officially registered)."
	    //
	    if (serve)
		goto too_many;
	    serve = server_password_new(np);
	    break;
	}
	arglex();
    }
    if (!serve)
    {
	error_raw("no server mode specified");
	usage();
    }

    //
    // Run the server.
    //
    server_run(serve);
    server_delete(serve);
    delete np;

    //
    // Report success.
    //
    quit(0);
    return 0;
}
