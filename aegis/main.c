/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993 Peter Miller.
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <aeb.h>
#include <aeca.h>
#include <aecd.h>
#include <aecp.h>
#include <aecpu.h>
#include <aed.h>
#include <aedb.h>
#include <aedbu.h>
#include <aede.h>
#include <aedeu.h>
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
#include <arglex2.h>
#include <error.h>
#include <help.h>
#include <log.h>
#include <option.h>
#include <os.h>
#include <str.h>
#include <trace.h>
#include <undo.h>
#include <version.h>

static arglex_table_ty argtab[] =
{
	{
		"-ANticipate",
		(arglex_token_ty)arglex_token_anticipate,
	},
	{
		"-AUTOmatic",
		(arglex_token_ty)arglex_token_automatic,
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
		"-Force",
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
		"-Not_Logging",
		(arglex_token_ty)arglex_token_nolog,
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
		"-TERse",
		(arglex_token_ty)arglex_token_terse,
	},
	{
		"-Test",
		(arglex_token_ty)arglex_token_test,
	},
	{
		"-UNFormatted",
		(arglex_token_ty)arglex_token_unformatted,
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
"NAME",
"	%s - project change supervisor",
"",
"SYNOPSIS",
"	%s <function> [ <option>...  ]",
"	%s -Help",
"",
"DESCRIPTION",
"	The %s program is used to supervise the development",
"	and integration of changes into projects.",
"",
"FUNCTIONS",
"	The following functions are available:",
"",
"	-Build",
"		The %s -Build command is used to build a",
"		project.  See aeb(1) for more information.",
"",
"	-Change_Attributes",
"		The %s -Change_Attributes command is used to",
"		modify the attributes of a change.  See aeca(1)",
"		for more information.",
"",
"	-Change_Directory",
"		The %s -Change_Directory command is used to",
"		change directory.  See aecd(1) for more",
"		information.",
"",
"	-CoPy_file",
"		The %s -CoPy_file command is used to copy a",
"		file into a change.  See aecp(1) for more",
"		information.",
"",
"	-CoPy_file_Undo",
"		The %s -Copy_File_Undo command is used to",
"		remove a copy of a file from a change.	See",
"		aecpu(1) for more information.",
"",
"	-Develop_Begin",
"		The %s -Develop_Begin command is used to begin",
"		development of a change.  See aedb(1) for more",
"		information.",
"",
"	-Develop_Begin_Undo",
"		The %s -Develop_Begin_Undo command is used to",
"		cease development of a change.	aedbu(1) for more",
"		information.",
"",
"	-Develop_End",
"		The %s -Develop_End command is used to",
"		complete development of a change.  See aede(1)",
"		for more information.",
"",
"	-Develop_End_Undo",
"		The %s -Develop_End_Undo command is used to",
"		recall a change for further deveopment.	 See",
"		aedeu(1) for more information.",
"",
"	-DIFFerence",
"		The %s -DIFFerence command is used to find",
"		differences between development directory and",
"		baseline.  See aed(1) for more information.",
"",
"	-Help",
"		This option may be used to obtain more",
"		information about how to use the %s program.",
"",
"	-Integrate_Begin",
"		The %s -Integrate_Begin command is used to",
"		being integrating a change.  See aeib(1) for more",
"		information.",
"",
"	-Integrate_Begin_Undo",
"		The %s -Integrate_Begin_Undo command is used",
"		to cease integrating a change.	See aeibu(1) for",
"		more information.",
"",
"	-Integrate_Fail",
"		The %s -Integrate_Fail command is used to fail",
"		a change integration.  See aeif(1) for more",
"		information.",
"",
"	-Integrate_Pass",
"		The %s -Integrate_Pass command is used to pass",
"		a change integration.  See aeip(1) for more",
"		information.",
"",
"	-List",
"		The %s -List command is used to list",
"		interesting things.  See ael(1) for more",
"		information.",
"",
"	-MoVe_file",
"		The aegis -MoVe_file command is	used to	change",
"		the name of a file as part of a	change.	 See",
"		aemv(1)	for more information.",
"",
"	-New_Administrator",
"		The %s -New_Administrator command is used to",
"		add new administrators to a project.  See aena(1)",
"		for more information.",
"",
"	-New_Change",
"		The %s -New_Change command is used to add a",
"		new change to a project.  See aenc(1) for more",
"		information.",
"",
"	-New_Change_Undo",
"		The %s -New_Change_Undo command is used to",
"		remove a new change from a project.  See aencu(1)",
"		for more information.",
"",
"	-New_Developer",
"		The %s -New_Developer command is used to add",
"		new developers to a project.  See aend(1) for",
"		more information.",
"",
"	-New_File",
"		The %s -New_File command is used to add new",
"		files to a change.  See aenf(1) for more",
"		information.",
"",
"	-New_File_Undo",
"		The %s -New_File_Undo command is used to",
"		remove new files from a change.	 See aenfu(1) for",
"		more information.",
"",
"	-New_Integrator",
"		The %s -New_Integrator command is used to add",
"		new integrators to a project.  See aeni(1) for",
"		more information.",
"",
"	-New_Project",
"		The %s -New_Project command is used to create",
"		a new project to be watched over by %s.  See",
"		aenp(1) for more information.",
"",
"	-New_ReLeaSe",
"		The %s -New_ReLeaSe command is used to create",
"		a new project from an existing project.	 See",
"		aenrls(1) for more information.",
"",
"	-New_ReViewer",
"		The %s -New_ReViewer command is used to add",
"		new reviewers to a project.  See aenrv(1) for",
"		more information.",
"",
"	-New_Test",
"		The %s -New_Test command is used to add a new",
"		test to a change See aent(1) for more",
"		information.",
"",
"	-New_Test_Undo",
"		The %s -New_Test_Undo command is used to",
"		remove new tests from a change.	 See aentu(1) for",
"		more information.",
"",
"	-Project_Attributes",
"		The %s -Project_Attributes command is used to",
"		modify the attributes of a project.",
"",
"	-Remove_Administrator",
"		The %s -Remove_Administrator command is used",
"		to remove administrators from a project.  See",
"		aera(1) for more information.",
"",
"	-Remove_Developer",
"		The %s -Remove_Developer command is used to",
"		remove developers from a project.  See aerd(1)",
"		for more information.",
"",
"	-ReMove_file",
"		The %s -ReMove_file command is used to add",
"		files to be deleted to a change.  See aerm(1) for",
"		more information.",
"",
"	-ReMove_file_Undo",
"		The %s -Remove_File_Undo command is used to",
"		remove files to be deleted from a change.  See",
"		aermu(1) for more information.",
"",
"	-Remove_Integrator",
"		The %s -Remove_Integrator command is used to",
"		remove integrators from a project.  See aeri(1)",
"		for more information.",
"",
"	-ReMove_PRoject",
"		The %s -ReMove_PRoject command is used to",
"		remove a project.  See aermpr(1) for more information.",
"",
"	-Remove_ReViewer",
"		The %s -Remove_ReViewer command is used to",
"		remove reviewers from a project.  See aerrv(1)",
"		for more information.",
"",
"	-Review_Fail",
"		The %s -Review_Fail command is used to fail a",
"		change review.	See aerf(1) for more information.",
"",
"	-Review_Pass",
"		The %s -Review_Pass command is used to pass a",
"		change review.	See aerp(1) for more information.",
"",
"	-Review_Pass_Undo",
"		The %s -Review_Pass_Undo command is used to rescind",
"		a change review pass.  See aerpu(1) for more information.",
"",
"	-Test",
"		The %s -Test command is used to run tests.",
"		See aet(1) for more information.",
"",
"	-User_Attributes",
"		The %s -User_Attributes command is used to",
"		modify the attributes of a user.  See aeua(1) for",
"		more information.",
"",
"	-VERSion",
"		The %s -VERsion command is used to get",
"		copyright and version details.	See aev(1) for",
"		more information.",
"",
"	All options may be abbreviated; the abbreviation is",
"	documented as the upper case letters, all lower case",
"	letters and underscores (_) are optional.  You must use",
"	consecutive sequences of optional letters.",
"",
"	All options are case insensitive, you may type them in",
"	upper case or lower case or a combination of both, case",
"	is not important.",
"",
"	For example: the arguments \"-project, \"-PROJ\" and \"-p\"",
"	are all interpreted to mean the -Project option.  The",
"	argument \"-prj\" will not be understood, because",
"	consecutive optional characters were not supplied.",
"",
"	Options and other command line arguments may be mixed",
"	arbitrarily on the command line, after the function",
"	selectors.",
"",
"	The GNU long option names are understood.  Since all",
"	option names for aegis are long, this means ignoring the",
"	extra leading '-'.  The \"--option=value\" convention is",
"	also understood.",
"",
"OPTIONS",
"	The following options are available to all functions.  These",
"	options may appear anywhere on the command line following",
"	the function selector.",
"",
"	-LIBrary <abspath>",
"		This option may be used to specify a directory to be",
"		searched for global state files and user state",
"		files.  (See aegstate(5) and aeustate(5) for more",
"		information.) Several library options may be present",
"		on the command line, and are search in the order",
"		given.  Appended to this explicit search path are",
"		the directories specified by the AEGIS enviroment",
"		variable (colon separated), and finally,",
"		/usr/local/lib/aegis is always searched.  All paths",
"		specified, either on the command line or in the",
"		AEGIS environment variable, must be absolute.",
"",
"	-Page_Length <number>",
"		This option may be used to set the page length of",
"		listings.  The default, in order of preference, is",
"		obtained from the system, from the LINES environment",
"		variable, or set to 24 lines.",
"",
"	-Page_Width <number>",
"		This option may be used to set the page width of",
"		listings and error messages.  The default, in order",
"		of preference, is obtained from the system, from the",
"		COLS environment variable, or set to 79 characters.",
"",
"	-TERse",
"		This option may be used to cause listings to",
"		produce the bare minimum of information.  It is",
"		usually useful for shell scripts.",
"",
"	-UNFormatted",
"		This option may be used with most listings to",
"		specify that the column formatting is not to be",
"		performed.  This is useful for shell scripts.",
"",
"	-Verbose",
"		This option may be used to cause aegis to produce",
"		more output.  By default aegis only produces",
"		output on errors.  When used with the -List",
"		option this option causes column headings to be",
"		added.",
"",
"	All options may be abbreviated; the abbreviation is",
"	documented as the upper case letters, all lower case",
"	letters and underscores (_) are optional.  You must use",
"	consecutive sequences of optional letters.",
"",
"	All options are case insensitive, you may type them in",
"	upper case or lower case or a combination of both, case",
"	is not important.",
"",
"	For example: the arguments \"-project, \"-PROJ\" and \"-p\"",
"	are all interpreted to mean the -Project option.  The",
"	argument \"-prj\" will not be understood, because",
"	consecutive optional characters were not supplied.",
"",
"	Options and other command line arguments may be mixed",
"	arbitrarily on the command line, after the function",
"	selectors.",
"",
"	The GNU long option names are understood.  Since all",
"	option names for aegis are long, this means ignoring the",
"	extra leading '-'.  The \"--option=value\" convention is",
"	also understood.",
"",
"EXIT STATUS",
"	The %s command will exit with a status of 1 on any",
"	error.	The %s command will only exit with a status of",
"	0 if there are no errors.",
"",
"COPYRIGHT",
"	%C",
"",
"AUTHOR",
"	%A",
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
	os_become_init();
	str_initialize();
	arglex_init(argc, argv, argtab);
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
