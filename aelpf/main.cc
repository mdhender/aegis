//
//	aegis - project change supervisor
//	Copyright (C) 2005-2008 Peter Miller
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

#include <common/env.h>
#include <common/language.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/rsrc_limits.h>
#include <libaegis/arglex2.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/change/file.h>
#include <libaegis/change.h>
#include <libaegis/fstate.h>
#include <libaegis/help.h>
#include <libaegis/os.h>
#include <libaegis/project/file.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>
#include <libaegis/version.h>


enum
{
    arglex_token_usage = ARGLEX2_MAX,
    arglex_token_usage_not,
    arglex_token_action,
    arglex_token_action_not,
    ARGLEX3_MAX
};

static arglex_table_ty argtab[] =
{
    { "-ACtion", arglex_token_action, },
    { "-Not_ACtion", arglex_token_action_not, },
    { "-USAge", arglex_token_usage, },
    { "-Not_USAge", arglex_token_usage_not, },
    ARGLEX_END_MARKER
};


static void
list_usage(void)
{
    const char *progname = progname_get();
    fprintf(stderr, "usage: %s [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -Help\n", progname);
    quit(1);
}


static void
list_help(void)
{
    help((char *)0, list_usage);
}


static int
find_action()
{
    int curtok = arglex_token;
    if (arglex() != arglex_token_string)
	option_needs_name(curtok, list_usage);
    string_ty *s = str_from_c(arglex_value.alv_string);
    file_action_ty fa;
    if (file_action_type.enum_parse(s, &fa))
    {
	str_free(s);
	return (1 << fa);
    }
    string_ty *s2 = file_action_type.fuzzy(s);
    if (s2)
    {
	sub_context_ty sc;
	sc.var_set_string("Name", s);
	sc.var_set_string("Guess", s2);
	sc.fatal_intl(i18n("no \"$name\", guessing \"$guess\""));
	// NOTREACHED
    }
    else
    {
	sub_context_ty sc;
	sc.var_set_string("Name", s);
	sc.fatal_intl(i18n("the name \"$name\" is undefined"));
	// NOTREACHED
    }
    return 0;
}


static int
find_usage()
{
    int curtok = arglex_token;
    if (arglex() != arglex_token_string)
	option_needs_name(curtok, list_usage);
    string_ty *s = str_from_c(arglex_value.alv_string);
    file_usage_ty fu;
    if (file_usage_type.enum_parse(s, &fu))
    {
	str_free(s);
	return (1 << fu);
    }
    string_ty *s2 = file_usage_type.fuzzy(s);
    if (s2)
    {
	sub_context_ty sc;
	sc.var_set_string("Name", s);
	sc.var_set_string("Guess", s2);
	sc.fatal_intl(i18n("no \"$name\", guessing \"$guess\""));
	// NOTREACHED
    }
    else
    {
	sub_context_ty sc;
	sc.var_set_string("Name", s);
	sc.fatal_intl(i18n("the name \"$name\" is undefined"));
	// NOTREACHED
    }
    return 0;
}


int
main(int argc, char **argv)
{
    resource_limits_init();
    os_become_init_mortal();
    arglex2_init3(argc, argv, argtab);
    env_initialize();
    language_init();

    switch (arglex())
    {
    case arglex_token_help:
	list_help();
	quit(0);

    case arglex_token_version:
	version();
	quit(0);
    }

    string_ty *project_name = 0;
    long change_number = 0;
    int action = 0;
    int action_not = 0;
    int usage = 0;
    int usage_not = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(list_usage);
	    continue;

	case arglex_token_change:
	    arglex();
	    // fall through...

	case arglex_token_number:
	    arglex_parse_change(&project_name, &change_number, list_usage);
	    continue;

        case arglex_token_project:
	    arglex();
	    arglex_parse_project(&project_name, list_usage);
	    continue;

	case arglex_token_action:
	    action |= find_action();
	    break;

	case arglex_token_action_not:
	    action_not |= find_action();
	    break;

	case arglex_token_usage:
	    usage |= find_usage();
	    break;

	case arglex_token_usage_not:
	    usage_not |= find_usage();
	    break;
	}
	arglex();
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
    project_name = 0;
    pp->bind_existing();

    change::pointer cp = 0;
    if (change_number > 0)
    {
	//
	// locate change data
	//
	cp = change_alloc(pp, change_number);
	change_bind_existing(cp);
    }

    //
    // Construct the file action mask.
    //
    // If the user specified no particular file actions to include or
    // exclude, then by default we will exclude removed files.
    //
    if (action == 0 && action_not == 0)
	action_not = 1 << file_action_remove;
    if (action == 0)
	action = ~0;
    action &= ~action_not;

    //
    // Construct the file usage mask.
    //
    if (usage == 0)
	usage = ~0;
    usage &= ~usage_not;

    //
    // emit the listing
    //
    for (size_t j = 0; ; ++j)
    {
	fstate_src_ty *src = pp->file_nth(j, view_path_simple);
	if (!src)
	    break;
	if (cp)
	{
	    // Change file superceede project files,
	    // so we hide them.
	    fstate_src_ty *csrc = change_file_find(cp, src, view_path_first);
	    if (csrc)
		continue;
	}
	if ((action & (1 << src->action)) && (usage & (1 << src->usage)))
	    printf("%s\n", src->file_name->str_text);
    }

    //
    // report success
    //
    quit(0);
    return 0;
}
