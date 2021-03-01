/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999 Peter Miller;
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
	fprintf(stderr, "       %s --help\n", progname);
	exit(1);
}


int main _((int, char **));

int
main(argc, argv)
	int		argc;
	char		**argv;
{
	arglex3_init(argc, argv); 
	str_initialize();
	env_initialize();
	language_init();
	os_become_init_mortal();
	arglex();
	for (;;)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(usage);
			continue;

		case arglex_token_help:
			help(0, usage);
			break;

		case arglex_token_send:
			arglex();
			send_main(usage);
			break;

		case arglex_token_list:
			arglex();
			list_main(usage);
			break;

		case arglex_token_receive:
			arglex();
			receive_main(usage);
			break;
		}
		break;
	}
	exit(0);
	return 0;
}
