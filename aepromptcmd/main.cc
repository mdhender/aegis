//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
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
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/change.h>
#include <common/env.h>
#include <common/error.h> // for assert
#include <common/language.h>
#include <libaegis/help.h>
#include <libaegis/os.h>
#include <libaegis/option.h>
#include <common/progname.h>
#include <libaegis/project.h>
#include <common/quit.h>
#include <common/rsrc_limits.h>
#include <common/trace.h>
#include <libaegis/user.h>
#include <libaegis/version.h>

enum
{
    black,
    red,
    green,
    yellow,
    blue,
    magenta,
    cyan,
    white,
    foreground = 30,
    background = 40
};


//
// NAME
//	prompt_usage
//
// SYNOPSIS
//	void prompt_usage(void);
//
// DESCRIPTION
//	The prompt_usage function is used to
//	briefly describe how to used the 'aegis -RePorT' command.
//

static void
prompt_usage(void)
{
    const char *progname = progname_get();
    fprintf(stderr, "usage: %s [ <option>... ] <string>...\n", progname);
    fprintf(stderr, "       %s -List [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -Help\n", progname);
    quit(1);
}


//
// NAME
//	prompt_help
//
// SYNOPSIS
//	void prompt_help(void);
//
// DESCRIPTION
//	The prompt_help function is used to
//	describe in detail how to use the `prompt' command.
//

static void
prompt_help(void)
{
    help(0, prompt_usage);
}


static void
set_foreground(int n)
{
    assert(n >= 0 && n < 8);
    printf("\33[%dm", n + foreground);
}


static void
clear_attributes(void)
{
    printf("\33[0m");
}



//
// NAME
//	prompt_main
//
// SYNOPSIS
//	void prompt_main(void);
//
// DESCRIPTION
//	The prompt_main function is used to substiture the strings on the
//	command line, using the usual Aegis substitutions, and then echo
//	them to stdout.
//
static void
prompt_main(void)
{
    trace(("prompt_main()\n{\n"));
    string_ty *project_name = 0;
    long change_number = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(prompt_usage);
	    continue;

	case arglex_token_change:
	    arglex();
	    // fall through...

	case arglex_token_number:
	    arglex_parse_change(&project_name, &change_number, prompt_usage);
	    continue;

	case arglex_token_project:
	    arglex();
	    // fall through...

	case arglex_token_string:
	    arglex_parse_project(&project_name, prompt_usage);
	    continue;
	}
	arglex();
    }

    if (!option_verbose_get())
	freopen("/dev/null", "w", stderr);

    //
    // locate project data
    //
    if (!project_name)
    {
        nstring n = user_ty::create()->default_project();
	project_name = str_copy(n.get_ref());
    }
    project_ty *pp = project_alloc(project_name);
    str_free(project_name);
    pp->bind_existing();

    //
    // locate user data
    //
    user_ty::pointer up = user_ty::create();

    //
    // locate chnage data
    //
    if (!change_number)
	change_number = up->default_change(pp);
    change::pointer cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    //
    // Work out what state the change is in.
    //
    cstate_ty *cstate_data = cp->cstate_get();
    bool build_needed = false;
    bool test_needed = false;
    bool test_baseline_needed = false;
    bool regression_test_needed = false;
    switch (cstate_data->state)
    {
    case cstate_state_awaiting_development:
    case cstate_state_awaiting_review:
    case cstate_state_being_reviewed:
    case cstate_state_awaiting_integration:
    case cstate_state_completed:
	// Nothing to do.
	break;

    case cstate_state_being_developed:
    case cstate_state_being_integrated:
	// Is a build required?
	build_needed =
	    change_build_required(cp) && (cstate_data->build_time == 0);
	test_needed =
	    (cstate_data->test_time == 0 && !cstate_data->test_exempt);
	test_baseline_needed =
	    (
		cstate_data->test_baseline_time == 0
	    &&
		!cstate_data->test_baseline_exempt
	    );
	regression_test_needed =
	    (
		cstate_data->regression_test_time == 0
	    &&
		!cstate_data->regression_test_exempt
	    );

	if (build_needed || test_needed)
	    set_foreground(red);
	else if (test_baseline_needed || regression_test_needed)
	    set_foreground(magenta);
	else
	    set_foreground(green);
	break;
    }

    //
    // clean up and go home
    //
    project_free(pp);
    change_free(cp);
    trace(("}\n"));
}


//
// NAME
//	prompt
//
// SYNOPSIS
//	void prompt(void);
//
// DESCRIPTION
//	The prompt function is used to
//	dispatch the 'aegis -RePorT' command to the relevant functionality.
//	Where it goes depends on the command line.
//

int
main(int argc, char **argv)
{
    resource_limits_init();
    clear_attributes();
    os_become_init_mortal();
    arglex2_init(argc, argv);
    env_initialize();
    language_init();
    switch (arglex())
    {
    default:
	prompt_main();
	break;

    case arglex_token_help:
	prompt_help();
	break;

    case arglex_token_version:
	version_copyright();
	break;
    }
    quit(0);
    return 0;
}
