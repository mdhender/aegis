//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2008 Peter Miller
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
#include <libaegis/file.h>
#include <libaegis/help.h>
#include <common/language.h>
#include <common/nstring/list.h>
#include <libaegis/os.h>
#include <common/progname.h>
#include <libaegis/project.h>
#include <common/quit.h>
#include <common/rsrc_limits.h>
#include <common/str_list.h>
#include <libaegis/sub.h>
#include <common/trace.h>
#include <libaegis/user.h>
#include <libaegis/version.h>


//
// NAME
//	aesub_usage
//
// SYNOPSIS
//	void aesub_usage(void);
//
// DESCRIPTION
//	The aesub_usage function is used to
//	briefly describe how to used the 'aegis -RePorT' command.
//

static void
aesub_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf(stderr, "usage: %s [ <option>... ] <string>...\n", progname);
    fprintf(stderr, "       %s -List [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -Help\n", progname);
    quit(1);
}


//
// NAME
//	aesub_help
//
// SYNOPSIS
//	void aesub_help(void);
//
// DESCRIPTION
//	The aesub_help function is used to
//	describe in detail how to use the `aesub' command.
//

static void
aesub_help(void)
{
    help(0, aesub_usage);
}


//
// NAME
//	aesub_main
//
// SYNOPSIS
//	void aesub_main(void);
//
// DESCRIPTION
//	The aesub_main function is used to substiture the strings on the
//	command line, using the usual Aegis substitutions, and then echo
//	them to stdout.
//

static void
aesub_main(void)
{
    trace(("aesub_main()\n{\n"));
    string_ty *project_name = 0;
    long change_number = 0;
    int baseline = 0;
    nstring_list arg;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(aesub_usage);
	    continue;

	case arglex_token_change:
	    arglex();
	    arglex_parse_change(&project_name, &change_number, aesub_usage);
	    continue;

	case arglex_token_project:
	    arglex();
	    arglex_parse_project(&project_name, aesub_usage);
	    continue;

	case arglex_token_string:
	case arglex_token_number:
	    arg.push_back(nstring(arglex_value.alv_string));
	    break;

	case arglex_token_baseline:
	    if (baseline)
		duplicate_option(aesub_usage);
	    baseline = 1;
	    break;

	case arglex_token_file:
	    switch (arglex())
	    {
	    default:
		option_needs_file(arglex_token_file, aesub_usage);
		// NOTREACHED

	    case arglex_token_string:
		{
		    os_become_orig();
		    nstring s(read_whole_file(arglex_value.alv_string));
		    os_become_undo();
		    arg.push_back(s);
		}
		break;

	    case arglex_token_stdio:
		{
		    os_become_orig();
		    nstring s(read_whole_file((char *)0));
		    os_become_undo();
		    arg.push_back(s);
		}
		break;
	    }
	    break;
	}
	arglex();
    }
    if (change_number && baseline)
    {
	mutually_exclusive_options
	(
	    arglex_token_change,
	    arglex_token_baseline,
	    aesub_usage
	);
    }

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
    // locate change data
    //
    if (baseline)
    {
	for (size_t j = 0; j < arg.size(); ++j)
	{
	    if (j)
		putchar(' ');
	    sub_context_ty sc("command line", j + 1);
	    nstring text(sc.substitute_p(pp, arg[j].get_ref()));
	    fwrite(text.c_str(), 1, text.size(), stdout);
	}
    }
    else
    {
	if (!change_number)
	    change_number = up->default_change(pp);
	change::pointer cp = change_alloc(pp, change_number);
	change_bind_existing(cp);

	for (size_t j = 0; j < arg.size(); ++j)
	{
	    if (j)
		putchar(' ');
	    sub_context_ty sc("command line", j + 1);
	    nstring text(sc.substitute(cp, arg[j].get_ref()));
	    fwrite(text.c_str(), 1, text.size(), stdout);
	}
	change_free(cp);
    }
    putchar('\n');

    //
    // clean up and go home
    //
    project_free(pp);
    trace(("}\n"));
}


//
// NAME
//	aesub
//
// SYNOPSIS
//	void aesub(void);
//
// DESCRIPTION
//	The aesub function is used to
//	dispatch the 'aegis -RePorT' command to the relevant functionality.
//	Where it goes depends on the command line.
//

int
main(int argc, char **argv)
{
    resource_limits_init();
    os_become_init_mortal();
    arglex2_init(argc, argv);
    env_initialize();
    language_init();
    switch (arglex())
    {
    default:
	aesub_main();
	break;

    case arglex_token_help:
	aesub_help();
	break;

    case arglex_token_version:
	version_copyright();
	break;
    }
    quit(0);
    return 0;
}
