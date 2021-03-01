//
//	aegis - project change supervisor
//	Copyright (C) 2001-2008 Peter Miller
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
#include <common/error.h>
#include <common/language.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/rsrc_limits.h>
#include <libaegis/arglex2.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/change.h>
#include <libaegis/help.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>
#include <libaegis/version.h>
#include <libaegis/zero.h>

#include <aels/list.h>
#include <aels/stack.h>


enum
{
    arglex_token_attr_hide = ARGLEX2_MAX,
    arglex_token_attr_show,
    arglex_token_dot_files_hide,
    arglex_token_dot_files_show,
    arglex_token_group_hide,
    arglex_token_group_show,
    arglex_token_long,
    arglex_token_mode_hide,
    arglex_token_mode_show,
    arglex_token_recursive,
    arglex_token_size_hide,
    arglex_token_size_show,
    arglex_token_user_hide,
    arglex_token_user_show,
    arglex_token_when_hide,
    arglex_token_when_show,
    ARGLEX3_MAX
};

static arglex_table_ty argtab[] =
{
    { "-Hide_Attributes", arglex_token_attr_hide, },
    { "-Show_Attributes", arglex_token_attr_show, },
    { "-Hide_Dot_Files", arglex_token_dot_files_hide, },
    { "-ALL", arglex_token_dot_files_show, },
    { "-Show_Dot_Files", arglex_token_dot_files_show, },
    { "-Hide_Group", arglex_token_group_hide, },
    { "-Show_Group", arglex_token_group_show, },
    { "-LOng", arglex_token_long, },
    { "-Hide_Mode", arglex_token_mode_hide, },
    { "-Show_Mode", arglex_token_mode_show, },
    { "-RECursive", arglex_token_recursive, },
    { "-Hide_Size", arglex_token_size_hide, },
    { "-Show_Size", arglex_token_size_show, },
    { "-Hide_User", arglex_token_user_hide, },
    { "-Show_User", arglex_token_user_show, },
    { "-Hide_When", arglex_token_when_hide, },
    { "-Show_When", arglex_token_when_show, },
    ARGLEX_END_MARKER
};


static void
list_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf(stderr, "usage: %s [ <option>... ][ <filename>... ]\n", progname);
    fprintf(stderr, "       %s -Help\n", progname);
    quit(1);
}


static void
list_help(void)
{
    help((char *)0, list_usage);
}


int
main(int argc, char **argv)
{
    long	    change_number;
    string_ty	    *project_name;
    string_ty	    *s;
    project_ty	    *pp;
    change::pointer cp;
    int		    baseline;
    user_ty::pointer up;
    int		    based;
    size_t	    j;

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

    project_name = 0;
    change_number = 0;
    baseline = 0;
    string_list_ty name;
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

        case arglex_token_string:
	    s = str_from_c(arglex_value.alv_string);
	    name.push_back(s);
	    str_free(s);
	    break;

        case arglex_token_base_relative:
        case arglex_token_current_relative:
	    user_ty::relative_filename_preference_argument(list_usage);
	    break;

        case arglex_token_baseline:
	    if (baseline)
		    duplicate_option(list_usage);
	    baseline = 1;
	    break;

        case arglex_token_recursive:
	    if (recursive_flag)
		    duplicate_option(list_usage);
	    recursive_flag = 1;
	    break;

        case arglex_token_long:
	    if (long_flag)
		    duplicate_option(list_usage);
	    long_flag = 1;
	    break;

        case arglex_token_dot_files_show:
	    show_dot_files = 1;
	    break;

        case arglex_token_dot_files_hide:
	    show_dot_files = 0;
	    break;

        case arglex_token_mode_show:
	    mode_flag = 1;
	    break;

        case arglex_token_mode_hide:
	    mode_flag = 0;
	    break;

        case arglex_token_attr_show:
	    attr_flag = 1;
	    break;

        case arglex_token_attr_hide:
	    attr_flag = 0;
	    break;

        case arglex_token_user_show:
	    user_flag = 1;
	    break;

        case arglex_token_user_hide:
	    user_flag = 0;
	    break;

        case arglex_token_group_show:
	    group_flag = 1;
	    break;

        case arglex_token_group_hide:
	    group_flag = 0;
	    break;

        case arglex_token_size_show:
	    size_flag = 1;
	    break;

        case arglex_token_size_hide:
	    size_flag = 0;
	    break;

        case arglex_token_when_show:
	    when_flag = 1;
	    break;

        case arglex_token_when_hide:
	    when_flag = 0;
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
    pp = project_alloc(project_name);
    str_free(project_name);
    pp->bind_existing();

    if (baseline)
    {
	if (change_number)
	{
    	    mutually_exclusive_options
    	    (
       		arglex_token_branch,
       		arglex_token_change,
       		list_usage
    	    );
	}

	stack_from_project(pp);

	up.reset();
	cp = 0;
    }
    else
    {
	cstate_ty       *cstate_data;

	//
	// locate user data
	//
	up = user_ty::create();

	//
	// locate change data
	//
	if (!change_number)
	    change_number = up->default_change(pp);
	cp = change_alloc(pp, change_number);
	change_bind_existing(cp);
	cstate_data = cp->cstate_get();

	if (cstate_data->state == cstate_state_completed)
	{
	    //
	    // Get the search path from the project.
	    //
	    stack_from_project(pp);

	    up.reset();
	    cp = 0;
	}
	else
	{
	    //
	    // It is an error if the change is not in the
	    // being_developed state (if it does not have a
	    // directory).
	    //
	    if (cstate_data->state < cstate_state_being_developed)
		change_fatal(cp, 0, i18n("bad aels state"));

	    //
	    // Get the search path from the change.
	    //
	    stack_from_change(cp);
	}
    }

    //
    // resolve the path of each path
    // 1. the absolute path of the file name is obtained
    // 2. if the file is inside the development directory, ok
    // 3. if the file is inside the baseline, ok
    // 4. if neither, error
    //
    if (!up)
	up = user_ty::create();
    based =
	(
	    up != 0
	&&
	    (
		up->relative_filename_preference
		(
		    uconf_relative_filename_preference_current
		)
	    ==
		uconf_relative_filename_preference_base
	    )
	);
    if (!name.nstrings)
    {
	os_become_orig();
	name.push_back(os_curdir());
	os_become_undo();
    }
    for (j = 0; j < name.nstrings; ++j)
    {
	string_ty	*s0;
	string_ty	*s1;
	string_ty	*s2;

	s0 = name.string[j];
	if (s0->str_text[0] == '/' || !based)
	    s1 = str_copy(s0);
	else
	    s1 = os_path_join(stack_nth(0), s0);
	str_free(s0);
	s2 = stack_relative(s1);
	if (!s2)
	{
	    sub_context_ty	*scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", s1);
	    if (cp)
	       	change_fatal(cp, scp, i18n("$filename unrelated"));
	    project_fatal(pp, scp, i18n("$filename unrelated"));
	    // NOTREACHED
	}
	assert(s2);
	str_free(s1);
	name.string[j] = s2;
    }

    //
    // emit the listing
    //
    list(&name, pp, cp);

    //
    // report success
    //
    quit(0);
    return 0;
}
