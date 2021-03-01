/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999 Peter Miller;
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
 * MANIFEST: the operating system start-up point
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/unistd.h>

#include <aeb.h>
#include <aeca.h>
#include <aecd.h>
#include <aechown.h>
#include <aeclean.h>
#include <aeclone.h>
#include <aecp.h>
#include <aecpu.h>
#include <aed.h>
#include <aedb.h>
#include <aedbu.h>
#include <aede.h>
#include <aedeu.h>
#include <aedn.h>
#include <aeib.h>
#include <aeibu.h>
#include <aeif.h>
#include <aeip.h>
#include <ael.h>
#include <aemv.h>
#include <aena.h>
#include <aenbr.h>
#include <aenbru.h>
#include <aenc.h>
#include <aencu.h>
#include <aend.h>
#include <aenf.h>
#include <aenfu.h>
#include <aeni.h>
#include <aenpr.h>
#include <aenrls.h>
#include <aenrv.h>
#include <aent.h>
#include <aentu.h>
#include <aepa.h>
#include <aer.h>
#include <aera.h>
#include <aerd.h>
#include <aerf.h>
#include <aeri.h>
#include <aerm.h>
#include <aermpr.h>
#include <aermu.h>
#include <aerp.h>
#include <aerpu.h>
#include <aerrv.h>
#include <aet.h>
#include <aev.h>
#include <arglex2.h>
#include <env.h>
#include <error.h>
#include <help.h>
#include <language.h>
#include <log.h>
#include <os.h>
#include <progname.h>
#include <r250.h>
#include <str.h>
#include <trace.h>
#include <undo.h>


static void usage _((void));

static void
usage()
{
	char	*progname;

	progname = progname_get();
	fprintf(stderr, "usage: %s <function> [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Help\n", progname);
	quit(1);
}


static void main_help _((void));

static void
main_help()
{
	trace(("main_help()\n{\n"/*}*/));
	help((char *)0, usage);
	trace((/*{*/"}\n"));
}


int main _((int, char **));

int
main(argc, argv)
	int		argc;
	char		**argv;
{
	r250_init();
	os_become_init();
	arglex2_init(argc, argv);
	str_initialize();
	env_initialize();
	language_init();
	quit_register(log_quitter);
	quit_register(undo_quitter);
	os_interrupt_register();
	arglex();
	for (;;)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(usage);
			continue;

		case arglex_token_build:
			build();
			break;

		case arglex_token_change_attributes:
			change_attributes();
			break;

		case arglex_token_change_directory:
			change_directory();
			break;

		case arglex_token_change_owner:
			change_owner();
			break;

		case arglex_token_clean:
			clean();
			break;

		case arglex_token_clone:
			clone();
			break;

		case arglex_token_copy_file:
			copy_file();
			break;

		case arglex_token_copy_file_undo:
			copy_file_undo();
			break;

		case arglex_token_develop_begin:
			develop_begin();
			break;

		case arglex_token_develop_begin_undo:
			develop_begin_undo();
			break;

		case arglex_token_develop_end:
			develop_end();
			break;

		case arglex_token_develop_end_undo:
			develop_end_undo();
			break;

		case arglex_token_difference:
			difference();
			break;

		case arglex_token_delta:
			delta_name_assignment();
			break;

		case arglex_token_help:
			main_help();
			break;

		case arglex_token_integrate_begin:
			integrate_begin();
			break;

		case arglex_token_integrate_begin_undo:
			integrate_begin_undo();
			break;

		case arglex_token_integrate_fail:
			integrate_fail();
			break;

		case arglex_token_integrate_pass:
			integrate_pass();
			break;

		case arglex_token_list:
			list();
			break;

		case arglex_token_move_file:
			move_file();
			break;

		case arglex_token_new_administrator:
			new_administrator();
			break;

		case arglex_token_new_branch:
			new_branch();
			break;

		case arglex_token_new_branch_undo:
			new_branch_undo();
			break;

		case arglex_token_new_change:
			new_change();
			break;

		case arglex_token_new_change_undo:
			new_change_undo();
			break;

		case arglex_token_new_developer:
			new_developer();
			break;

		case arglex_token_new_file:
			new_file();
			break;

		case arglex_token_new_file_undo:
			new_file_undo();
			break;

		case arglex_token_new_integrator:
			new_integrator();
			break;

		case arglex_token_new_project:
			new_project();
			break;

		case arglex_token_new_release:
			new_release();
			break;

		case arglex_token_new_reviewer:
			new_reviewer();
			break;

		case arglex_token_new_test:
			new_test();
			break;

		case arglex_token_new_test_undo:
			new_test_undo();
			break;

		case arglex_token_project_attributes:
			project_attributes();
			break;

		case arglex_token_remove_administrator:
			remove_administrator();
			break;

		case arglex_token_remove_developer:
			remove_developer();
			break;

		case arglex_token_remove_file:
			remove_file();
			break;

		case arglex_token_remove_file_undo:
			remove_file_undo();
			break;

		case arglex_token_remove_project:
			remove_project();
			break;

		case arglex_token_remove_integrator:
			remove_integrator();
			break;

		case arglex_token_remove_reviewer:
			remove_reviewer();
			break;

		case arglex_token_report:
			report();
			break;

		case arglex_token_review_fail:
			review_fail();
			break;

		case arglex_token_review_pass:
			review_pass();
			break;

		case arglex_token_review_pass_undo:
			review_pass_undo();
			break;

		case arglex_token_test:
			test();
			break;

		case arglex_token_version:
			version();
			break;
		}
		break;
	}
	quit(0);
	return 0;
}
