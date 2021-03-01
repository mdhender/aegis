//
//	aegis - project change supervisor
//	Copyright (C) 2005 Peter Miller;
//	All rights reserved.
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
// MANIFEST: implementation of the main class
//

#include <ac/stdio.h>
#include <ac/string.h>

#include <arglex2.h>
#include <env.h>
#include <error.h> // HACK
#include <help.h>
#include <language.h>
#include <nstring.h>
#include <os.h>
#include <progname.h>
#include <quit.h>
#include <r250.h>
#include <rfc822.h>
#include <rfc822/functor/list_meta.h>
#include <rfc822/functor/print_vers.h>
#include <simpverstool.h>
#include <sub.h>
#include <version.h>

enum
{
    arglex_token_checkin = ARGLEX2_MAX,
    arglex_token_checkout,
    arglex_token_history,
    arglex_token_query
};

static arglex_table_ty argtab[] =
{
    { "-CHeck_In", arglex_token_checkin },
    { "-COMmit", arglex_token_checkin },
    { "-CHeck_Out", arglex_token_checkout },
    { "-HIstory", arglex_token_history },
    { "-Query", arglex_token_query },
    ARGLEX_END_MARKER
};


static void
usage()
{
    const char *prog = progname_get();
    fprintf(stderr, "Usage: %s -checkin -hist <history> -f <file>\n", prog);
    fprintf(stderr, "       %s -checkout -hist <history> -o <file>\n", prog);
    fprintf(stderr, "       %s -query -hist <history>\n", prog);
    fprintf(stderr, "       %s -list -hist <history>\n", prog);
    quit(1);
}


static void
aesvt_help()
{
    help(0, usage);
}


static const char *
get_string(int tname)
{
    const char *result = "";
    switch (arglex_token)
    {
    case arglex_token_string:
    case arglex_token_number:
	result = arglex_value.alv_string;
	arglex();
	break;

    default:
	option_needs_string(tname, usage);
	// NOTREACHED
    }
    return result;
}


static const char *
get_file_name(int tname)
{
    const char *result = "";
    switch (arglex_token)
    {
    case arglex_token_string:
    case arglex_token_number:
	result = arglex_value.alv_string;
	arglex();
	break;

    case arglex_token_stdio:
	result = "-";
	arglex();
	break;

    default:
	option_needs_file(tname, usage);
	// NOTREACHED
    }
    return result;
}


int
main(int argc, char **argv)
{
    r250_init();
    os_become_init_mortal();
    arglex2_init3(argc, argv, argtab);
    env_initialize();
    language_init();

    switch (arglex())
    {
    case arglex_token_help:
	aesvt_help();
	quit(0);

    case arglex_token_version:
	version();
	quit(0);
    }


    enum action_t
    {
	action_unset,
	action_listing,
	action_checkin,
	action_checkout,
	action_head_revision
    };

    nstring edit;
    nstring file_name;
    nstring history_name;
    action_t action = action_unset;
    bool verbose = false;
    rfc822 meta_data;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	case arglex_token_edit:
	case arglex_token_version:
	    arglex();
	    edit = get_string(arglex_token_edit);
	    continue;

	case arglex_token_file:
	case arglex_token_output:
	    arglex();
	    file_name = get_file_name(arglex_token_file);
	    continue;

	case arglex_token_history:
	    arglex();
	    history_name = get_file_name(arglex_token_history);
	    continue;

	case arglex_token_checkin:
	    action = action_checkin;
	    break;

	case arglex_token_list:
	    action = action_listing;
	    break;

	case arglex_token_checkout:
	    action = action_checkout;
	    break;

	case arglex_token_query:
	    action = action_head_revision;
	    break;

	case arglex_token_verbose:
	    verbose = true;
	    break;

	case arglex_token_string:
	    // The rest of the arguments should be name=value pairs for checkin.
	    switch (action)
	    {
	    default:
		usage();
		// NOTREACHED

	    case action_unset:
		action = action_checkin;
		// fall  through...

	    case action_checkin:
		const char *arg = arglex_value.alv_string;
		const char *ep = strchr(arg, '=');
		if (!ep)
		    bad_argument(usage);
		nstring name(arg, ep - arg);
		nstring value(ep + 1);
		meta_data.set(name, value);
		break;
	    }
	    break;

	default:
	    bad_argument(usage);
	    // NOTREACHED
	}
	arglex();
    }
    if (history_name.empty())
    {
	sub_context_ty sc;
	sc.fatal_intl(i18n("no history file name specified"));
	// NOTREACHED
    }

    //
    // Create an instance of the tool.
    //
    simple_version_tool archive(history_name);

    //
    // Perform the appropriate action, based on the command line options
    // given.
    //
    if (action == action_unset)
    {
	// Try to guess.
	if (!edit.empty())
	    action = action_checkout;
    }
    os_become_orig();
    switch (action)
    {
    case action_head_revision:
	{
	    if (!edit.empty() || !file_name.empty())
		usage();
	    rfc822_functor_print_version doodle;
	    archive.list(doodle);
	}
	break;

    case action_checkin:
	if (!edit.empty())
	    usage();
	if (file_name.empty())
	{
	    sub_context_ty sc;
	    sc.error_intl(i18n("no input file"));
	    usage();
	}
	archive.checkin(file_name, meta_data);
	break;

    case action_listing:
	{
	    if (!edit.empty() || !file_name.empty())
		usage();
	    rfc822_functor_list_meta doodle;
	    archive.list(doodle);
	}
	break;

    case action_checkout:
	if (file_name.empty())
	{
	    sub_context_ty sc;
	    sc.error_intl(i18n("no output file"));
	    usage();
	}
	archive.checkout(file_name, edit);
	break;

    case action_unset:
#ifndef DEBUG
    default:
#endif
	{
	    sub_context_ty sc;
	    sc.error_intl(i18n("no action specified"));
	    usage();
	}
	// NOTREACHED
    }
    os_become_undo();
    quit(0);
    return 0;
}
