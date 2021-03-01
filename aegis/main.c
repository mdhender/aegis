/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995 Peter Miller;
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
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: the operating system start-up point
 */

#include <stdio.h>
#include <ac/stdlib.h>
#include <ac/unistd.h>

#include <aeb.h>
#include <aeca.h>
#include <aecd.h>
#include <aechown.h>
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
#include <log.h>
#include <option.h>
#include <os.h>
#include <r250.h>
#include <str.h>
#include <trace.h>
#include <undo.h>

static arglex_table_ty argtab[] =
{
	{
		"-ANticipate",
		(arglex_token_ty)arglex_token_anticipate,
	},
	{
		"-ASk",
		(arglex_token_ty)arglex_token_interactive,
	},
	{
		"-AUTOmatic",
		(arglex_token_ty)arglex_token_automatic,
	},
	{
		"-Automatic_Merge",
		(arglex_token_ty)arglex_token_merge_automatic,
	},
	{
		"-BaseLine",
		(arglex_token_ty)arglex_token_baseline,
	},
	{
		"-Build",
		(arglex_token_ty)arglex_token_build,
	},
	{
		"-Change",
		(arglex_token_ty)arglex_token_change,
	},
	{
		"-Change_Attributes",
		(arglex_token_ty)arglex_token_change_attributes,
	},
	{
		"-Change_Directory",
		(arglex_token_ty)arglex_token_change_directory,
	},
	{
		"-Change_Owner",
		(arglex_token_ty)arglex_token_change_owner,
	},
	{
		"-CoPy_file",
		(arglex_token_ty)arglex_token_copy_file,
	},
	{
		"-CoPy_file_Undo",
		(arglex_token_ty)arglex_token_copy_file_undo,
	},
	{
		"-DIFference",
		(arglex_token_ty)arglex_token_difference,
	},
	{
		"-DIRectory",
		(arglex_token_ty)arglex_token_directory,
	},
	{
		"-DELta",
		(arglex_token_ty)arglex_token_delta,
	},
	{
		"-Delta_Name",
		(arglex_token_ty)arglex_token_delta,
	},
	{
		"-Delta_Number",
		(arglex_token_ty)arglex_token_delta,
	},
	{
		"-Develop_Begin",
		(arglex_token_ty)arglex_token_develop_begin,
	},
	{
		"-Develop_Begin_Undo",
		(arglex_token_ty)arglex_token_develop_begin_undo,
	},
	{
		"-Develop_End",
		(arglex_token_ty)arglex_token_develop_end,
	},
	{
		"-Develop_End_Undo",
		(arglex_token_ty)arglex_token_develop_end_undo,
	},
	{
		"-Development_Directory",
		(arglex_token_ty)arglex_token_development_directory,
	},
	{
		"-Edit",
		(arglex_token_ty)arglex_token_edit,
	},
	{
		"-File",
		(arglex_token_ty)arglex_token_file,
	},
	{
		"-FOrce",
		(arglex_token_ty)arglex_token_force,
	},
	{
		"-INDependent",
		(arglex_token_ty)arglex_token_independent,
	},
	{
		"-Integrate_Begin",
		(arglex_token_ty)arglex_token_integrate_begin,
	},
	{
		"-Integrate_Begin_Undo",
		(arglex_token_ty)arglex_token_integrate_begin_undo,
	},
	{
		"-Integrate_FAIL",
		(arglex_token_ty)arglex_token_integrate_fail,
	},
	{
		"-Integrate_PASS",
		(arglex_token_ty)arglex_token_integrate_pass,
	},
	{
		"-Interactive",
		(arglex_token_ty)arglex_token_interactive,
	},
	{
		"-Keep",
		(arglex_token_ty)arglex_token_keep,
	},
	{
		"-LIBrary",
		(arglex_token_ty)arglex_token_library,
	},
	{
		"-LOg",
		(arglex_token_ty)arglex_token_log,
	},
	{
		"-List",
		(arglex_token_ty)arglex_token_list,
	},
	{
		"-MAJor",
		(arglex_token_ty)arglex_token_major,
	},
	{
		"-MANual",
		(arglex_token_ty)arglex_token_manual,
	},
	{
		"-Merge_Automatic",
		(arglex_token_ty)arglex_token_merge_automatic,
	},
	{
		"-Merge_Not",
		(arglex_token_ty)arglex_token_merge_not,
	},
	{
		"-Merge_Only",
		(arglex_token_ty)arglex_token_merge_only,
	},
	{
		"-MINImum",
		(arglex_token_ty)arglex_token_minimum,
	},
	{
		"-MINOr",
		(arglex_token_ty)arglex_token_minor,
	},
	{
		"-MoVe_file",
		(arglex_token_ty)arglex_token_move_file,
	},
	{
		"-New_Administrator",
		(arglex_token_ty)arglex_token_new_administrator,
	},
	{
		"-New_Change",
		(arglex_token_ty)arglex_token_new_change,
	},
	{
		"-New_Change_Undo",
		(arglex_token_ty)arglex_token_new_change_undo,
	},
	{
		"-New_Developer",
		(arglex_token_ty)arglex_token_new_developer,
	},
	{
		"-New_File",
		(arglex_token_ty)arglex_token_new_file,
	},
	{
		"-New_File_Undo",
		(arglex_token_ty)arglex_token_new_file_undo,
	},
	{
		"-New_Integrator",
		(arglex_token_ty)arglex_token_new_integrator,
	},
	{
		"-New_Project",
		(arglex_token_ty)arglex_token_new_project,
	},
	{
		"-New_ReLeaSe",
		(arglex_token_ty)arglex_token_new_release,
	},
	{
		"-New_ReViewer",
		(arglex_token_ty)arglex_token_new_reviewer,
	},
	{
		"-New_Test",
		(arglex_token_ty)arglex_token_new_test,
	},
	{
		"-New_Test_Undo",
		(arglex_token_ty)arglex_token_new_test_undo,
	},
	{
		"-Not_Keep",
		(arglex_token_ty)arglex_token_no_keep,
	},
	{
		"-Not_Logging",
		(arglex_token_ty)arglex_token_nolog,
	},
	{
		"-Not_Merge",
		(arglex_token_ty)arglex_token_merge_not,
	},
	{
		"-Only_Merge",
		(arglex_token_ty)arglex_token_merge_only,
	},
	{
		"-Output",
		(arglex_token_ty)arglex_token_output,
	},
	{
		"-OverWriting",
		(arglex_token_ty)arglex_token_overwriting,
	},
	{
		"-Page_Length",
		(arglex_token_ty)arglex_token_page_length,
	},
	{
		"-Page_Width",
		(arglex_token_ty)arglex_token_page_width,
	},
	{
		"-Project",
		(arglex_token_ty)arglex_token_project,
	},
	{
		"-Project_Attributes",
		(arglex_token_ty)arglex_token_project_attributes,
	},
	{
		"-Query",
		(arglex_token_ty)arglex_token_report,
	},
	{
		"-REGression",
		(arglex_token_ty)arglex_token_regression,
	},
	{
		"-ReMove_file",
		(arglex_token_ty)arglex_token_remove_file,
	},
	{
		"-ReMove_file_Undo",
		(arglex_token_ty)arglex_token_remove_file_undo,
	},
	{
		"-Remove_Administrator",
		(arglex_token_ty)arglex_token_remove_administrator,
	},
	{
		"-Remove_Developer",
		(arglex_token_ty)arglex_token_remove_developer,
	},
	{
		"-Remove_Integrator",
		(arglex_token_ty)arglex_token_remove_integrator,
	},
	{
		"-ReMove_PRoject",
		(arglex_token_ty)arglex_token_remove_project,
	},
	{
		"-Remove_ReViewer",
		(arglex_token_ty)arglex_token_remove_reviewer,
	},
	{
		"-RePorT",
		(arglex_token_ty)arglex_token_report,
	},
	{
		"-Review_FAIL",
		(arglex_token_ty)arglex_token_review_fail,
	},
	{
		"-Review_PASS",
		(arglex_token_ty)arglex_token_review_pass,
	},
	{
		"-Review_Pass_Undo",
		(arglex_token_ty)arglex_token_review_pass_undo,
	},
	{
		"-Tab_Width",
		(arglex_token_ty)arglex_token_tab_width,
	},
	{
		"-TERse",
		(arglex_token_ty)arglex_token_terse,
	},
	{
		"-Test",
		(arglex_token_ty)arglex_token_test,
	},
	{
		"-UNChanged",
		(arglex_token_ty)arglex_token_unchanged,
	},
	{
		"-UNFormatted",
		(arglex_token_ty)arglex_token_unformatted,
	},
	{
		"-User",
		(arglex_token_ty)arglex_token_user,
	},
	{
		"-Verbose",
		(arglex_token_ty)arglex_token_verbose,
	},

	/* end marker */
	{ 0, (arglex_token_ty)0, },
};


static void usage _((void));

static void
usage()
{
	char	*progname;

	progname = option_progname_get();
	fprintf(stderr, "usage: %s <function> [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Help\n", progname);
	quit(1);
}


static void main_help _((void));

static void
main_help()
{
	static char *text[] =
	{
#include <../man1/aegis.h>
	};

	trace(("main_help()\n{\n"/*}*/));
	help(text, SIZEOF(text), usage);
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
	arglex_init(argc, argv, argtab);
	str_initialize();
	env_initialize();
	quit_register(log_quitter);
	quit_register(undo_quitter);
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
