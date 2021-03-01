/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001, 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate mains
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/signal.h>

#include <arglex3.h>
#include <env.h>
#include <help.h>
#include <language.h>
#include <list.h>
#include <os.h>
#include <progname.h>
#include <receive.h>
#include <send.h>
#include <str.h>


static void usage _((void));

static void
usage()
{
	char		*progname;

	progname = progname_get();
	fprintf(stderr, "Usage: %s --send [ <option>... ]\n", progname);
	fprintf(stderr, "       %s --receive [ <option>... ]\n", progname);
	fprintf(stderr, "       %s --list [ <option>... ]\n", progname);
	fprintf(stderr, "       %s --help\n", progname);
	exit(1);
}


static void main_help _((void));

static void
main_help()
{
	help(0, usage);
}


int main _((int, char **));

int
main(argc, argv)
	int		argc;
	char		**argv;
{
	static arglex_dispatch_ty dispatch[] =
        {
		{ arglex_token_send,	send,		},
		{ arglex_token_list,	list,		},
		{ arglex_token_receive,	receive,	},
		{ arglex_token_help,	main_help,	},
	};

	/*
	 * Some versions of cron(8) set SIGCHLD to SIG_IGN.  This is
	 * kinda dumb, because it breaks assumptions made in libc (like
	 * pclose, for instance).  It also blows away most of Cook's
	 * process handling.  We explicitly set the SIGCHLD signal
	 * handling to SIG_DFL to make sure this signal does what we
	 * expect no matter how we are invoked.
	 */
#ifdef SIGCHLD
	signal(SIGCHLD, SIG_DFL);
#else
	signal(SIGCLD, SIG_DFL);
#endif

	arglex3_init(argc, argv);
	str_initialize();
	env_initialize();
	language_init();
	os_become_init_mortal();
	arglex_dispatch(dispatch, SIZEOF(dispatch), usage);
	exit(0);
	return 0;
}
