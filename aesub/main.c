/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2001-2003 Peter Miller;
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

#include <arglex2.h>
#include <arglex/change.h>
#include <arglex/project.h>
#include <change.h>
#include <env.h>
#include <error.h>
#include <help.h>
#include <language.h>
#include <os.h>
#include <progname.h>
#include <project.h>
#include <r250.h>
#include <str_list.h>
#include <sub.h>
#include <trace.h>
#include <user.h>
#include <version.h>


/*
 * NAME
 *	aesub_usage
 *
 * SYNOPSIS
 *	void aesub_usage(void);
 *
 * DESCRIPTION
 *	The aesub_usage function is used to
 *	briefly describe how to used the 'aegis -RePorT' command.
 */

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


/*
 * NAME
 *	aesub_help
 *
 * SYNOPSIS
 *	void aesub_help(void);
 *
 * DESCRIPTION
 *	The aesub_help function is used to
 *	describe in detail how to use the `aesub' command.
 */

static void
aesub_help(void)
{
    help(0, aesub_usage);
}


/*
 * NAME
 *	aesub_main
 *
 * SYNOPSIS
 *	void aesub_main(void);
 *
 * DESCRIPTION
 *	The aesub_main function is used to substiture the strings on the
 *	command line, using the usual Aegis substitutions, and then echo
 *	them to stdout.
 */

static void
aesub_main(void)
{
    string_ty	    *project_name;
    long	    change_number;
    string_list_ty  arg;
    string_ty	    *s;
    project_ty	    *pp;
    user_ty	    *up;
    sub_context_ty  *scp;
    size_t	    j;
    int		    baseline;

    trace(("aesub_main()\n{\n"));
    project_name = 0;
    change_number = 0;
    baseline = 0;
    string_list_constructor(&arg);
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
	    s = str_from_c(arglex_value.alv_string);
	    string_list_append(&arg, s);
	    str_free(s);
	    break;

	case arglex_token_baseline:
	    if (baseline)
		duplicate_option(aesub_usage);
	    baseline = 1;
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

    /*
     * locate project data
     */
    if (!project_name)
	project_name = user_default_project();
    pp = project_alloc(project_name);
    str_free(project_name);
    project_bind_existing(pp);

    /*
     * locate user data
     */
    up = user_executing(pp);

    /*
     * locate change data
     */
    if (baseline)
    {
	for (j = 0; j < arg.nstrings; ++j)
	{
	    if (j)
		putchar(' ');
	    scp = sub_context_New("command line", j + 1);
	    s = substitute_p(scp, pp, arg.string[j]);
	    sub_context_delete(scp);
	    fputs(s->str_text, stdout);
	    str_free(s);
	}
    }
    else
    {
	change_ty	*cp;

	if (!change_number)
	    change_number = user_default_change(up);
	cp = change_alloc(pp, change_number);
	change_bind_existing(cp);

	for (j = 0; j < arg.nstrings; ++j)
	{
	    if (j)
		putchar(' ');
	    scp = sub_context_New("command line", j + 1);
	    s = substitute(scp, cp, arg.string[j]);
	    sub_context_delete(scp);
	    fputs(s->str_text, stdout);
	    str_free(s);
	}
	change_free(cp);
    }
    putchar('\n');

    /*
     * clean up and go home
     */
    project_free(pp);
    string_list_destructor(&arg);
    trace(("}\n"));
}


/*
 * NAME
 *	aesub
 *
 * SYNOPSIS
 *	void aesub(void);
 *
 * DESCRIPTION
 *	The aesub function is used to
 *	dispatch the 'aegis -RePorT' command to the relevant functionality.
 *	Where it goes depends on the command line.
 */

int
main(int argc, char **argv)
{
    r250_init();
    os_become_init_mortal();
    arglex2_init(argc, argv);
    str_initialize();
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
    exit(0);
    return 0;
}
